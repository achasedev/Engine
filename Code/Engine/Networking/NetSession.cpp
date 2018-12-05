/************************************************************************/
/* File: NetSession.cpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Implementation of the NetSession class
/************************************************************************/
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/LogSystem.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Networking/NetObject.hpp"
#include "Engine/Networking/UDPSocket.hpp"
#include "Engine/Networking/NetPacket.hpp"
#include "Engine/Networking/NetSession.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Networking/NetConnection.hpp"
#include "Engine/Networking/NetObjectSystem.hpp"

// Message callbacks
bool OnPing(NetMessage* msg, const NetSender_t& sender);
bool OnPong(NetMessage* msg, const NetSender_t& sender);

bool OnNetObjectCreate(NetMessage* msg, const NetSender_t& sender);
bool OnNetObjectDestroy(NetMessage* msg, const NetSender_t& sender);
bool OnNetObjectUpdate(NetMessage* msg, const NetSender_t& sender);


//-----------------------------------------------------------------------------------------------
// Constructor
//
NetSession::NetSession()
{
	m_netObjectSystem = new NetObjectSystem(this);

	RegisterCoreMessages();
	m_netClock.Reset();
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
NetSession::~NetSession()
{
	// Clean up connections; send hang up messages
	ShutdownSession();

	if (m_netObjectSystem != nullptr)
	{
		delete m_netObjectSystem;
		m_netObjectSystem = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Attempts to host a session within the given port range
//
void NetSession::Host(const std::string& myName, uint16_t port, uint16_t portRange /*= DEFAULT_PORT_RANGE*/)
{
	// Early out if we're not in a state to host
	if (m_state != SESSION_DISCONNECTED)
	{
		ConsoleWarningf("NetSession attempted to host when not in a hostable state");
		LogTaggedPrintf("NET", "NetSession::Host() failed, attempted to host with name \"%s\" when not in a hostable state", myName.c_str());
		return;
	}

	// Try to bind the socket
	bool bindSucceeded = BindSocket(port, portRange);

	if (!bindSucceeded)
	{
		LogTaggedPrintf("NET", "NetSession::Host() failed, couldn't bind the socket");
		return;
	}

	// Bind succeeded, so make a connection for us
	NetConnectionInfo_t info;
	info.address = m_boundSocket->GetNetAddress();
	info.name = myName;
	info.sessionIndex = 0;

	NetConnection* connection = CreateConnection(info);
	m_myConnection = connection;
	m_hostConnection = connection;

	connection->SetConnectionState(CONNECTION_READY);
	TransitionToState(SESSION_READY);
}


//-----------------------------------------------------------------------------------------------
// Attempts to join a session given in hostInfo
//
void NetSession::Join(const std::string& myName, NetConnectionInfo_t& hostInfo)
{
	if (m_state != SESSION_DISCONNECTED)
	{
		ConsoleErrorf("Join called when session wasn't fully disconnected");
		return;
	}

	uint16_t portRange = MAX_CONNECTIONS;
	
	bool bound = BindSocket(hostInfo.address.port, portRange);

	if (!bound)
	{
		ConsoleErrorf("Couldn't join - socket couldn't bind");
		LogTaggedPrintf("NET", "Error: NetSession::Join() couldn't bind the socket to port %u", hostInfo.address.port);
		return;
	}

	// Create a connection for the host
	hostInfo.sessionIndex = 0;
	m_hostConnection = CreateConnection(hostInfo);
	
	// Create a connection for ourselves, with no index yet
	NetConnectionInfo_t myInfo;
	myInfo.address = m_boundSocket->GetNetAddress();
	myInfo.name = myName;
	myInfo.sessionIndex = INVALID_CONNECTION_INDEX; // Mark connnection as INVALID until we get an index from the host

	m_myConnection = CreateConnection(myInfo);

	// Send a join request
	NetMessage* msg = new NetMessage(GetMessageDefinition("join_request"));
	m_hostConnection->Send(msg);

	// We have a connection but is still in the DISCONNECTED connection state, so we're flagged as
	// connecting still
	TransitionToState(SESSION_CONNECTING);
}


//-----------------------------------------------------------------------------------------------
// Destroys all connections and sets us to a disconnected state
//
void NetSession::ShutdownSession()
{
	// Join the receiving thread
	m_isReceiving = false;
	if (m_receivingThread.joinable())
	{
		m_receivingThread.join();
	}

	// Send hang up messages
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
	{
		if (m_boundConnections[i] != nullptr && m_boundConnections[i] != m_myConnection && m_boundConnections[i]->IsReady())
		{
			NetMessage* msg = new NetMessage(GetMessageDefinition("hang_up"));
			m_boundConnections[i]->Send(msg);
		}
	}

	// Force a send out to get the hang up received
	ProcessOutgoing();

	// Then delete them all
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
	{
		if (m_boundConnections[i] != nullptr)
		{
			DestroyConnection(m_boundConnections[i]);
		}
	}

	// Our connection and the host connection aren't in the bound list when connecting
	// To prevent memory leak, check them here
	if (m_myConnection != nullptr && !m_myConnection->IsConnected())
	{
		delete m_myConnection;
	}

	m_myConnection = nullptr;

	if (m_hostConnection != nullptr && !m_hostConnection->IsConnected())
	{
		delete m_hostConnection;
	}

	m_hostConnection = nullptr;
	
	if (m_boundSocket != nullptr)
	{
		if (!m_boundSocket->IsClosed())
		{
			m_boundSocket->Close();
		}

		delete m_boundSocket;
		m_boundSocket = nullptr;
	}

	ConsolePrintf("Session shut down");
	LogTaggedPrintf("NET", "Session shut down");

	TransitionToState(SESSION_DISCONNECTED);
}


//-----------------------------------------------------------------------------------------------
// Returns whether this session is currently a host of a connection graph
//
bool NetSession::IsHosting() const
{
	// Session isn't even fully connected to anyone
	if (m_state != SESSION_READY) 
	{ 
		return false; 
	}

	// My connection should not be null here
	return (m_myConnection == m_hostConnection);
}


//-----------------------------------------------------------------------------------------------
// Sets the error of the session, only if an error doesn't already exist
//
void NetSession::SetError(eSessionError error, const std::string& errorMessage)
{
	if (m_error == SESSION_OK)
	{
		m_error = error;
		m_errorMesssage = errorMessage;
	}
}


//-----------------------------------------------------------------------------------------------
// Clears the error on the session
//
void NetSession::ClearError()
{
	m_error = SESSION_OK;
	m_errorMesssage.clear();
}


//-----------------------------------------------------------------------------------------------
// Returns the error currently on the session, and clears the error
//
eSessionError NetSession::GetLastError(std::string& out_errorMessage)
{
	eSessionError error = m_error;
	out_errorMessage = m_errorMesssage;

	ClearError();
	return error;
}


//-----------------------------------------------------------------------------------------------
// Processes all incoming packets and adjusts state of the session
//
void NetSession::Update()
{
	// Processes all received packets in the queue
	ProcessIncoming();

	// Check for timed out connections; shuts down the system if the host or my connection closes
	CheckForDisconnects();
	
	// Update switch on state
	switch (m_state)
	{
	case SESSION_DISCONNECTED:
		// Do nothing
		break;
	case SESSION_BOUND:
		// Idk why this exists
		break;
	case SESSION_CONNECTING:
		// Check if we've connected, and if so transition out
		if (m_hostConnection->IsConnected())
		{
			if (m_hostConnection->IsReady())
			{
				TransitionToState(SESSION_READY);
			}
			else
			{
				TransitionToState(SESSION_JOINING);
			}
		}
		else
		{
			if (m_stateTimer.GetElapsedTime() >= JOIN_TIMEOUT)
			{
				m_error = SESSION_ERROR_JOIN_DENIED;
				m_errorMesssage = "Timed out";

				ShutdownSession();
			}
			else if (m_joinTimer.HasIntervalElapsed())
			{
				// We're not connected to the host yet at all, so keep sending requests
				NetMessage* msg = new NetMessage(GetMessageDefinition("join_request"));
				m_hostConnection->Send(msg);

				m_joinTimer.SetInterval(0.1f);
			}
		}
		break;
	case SESSION_JOINING:
		// Check if we're ready, and if so mark the session as ready
		if (m_myConnection->IsReady())
		{
			TransitionToState(SESSION_READY);
		}
		break;
	case SESSION_READY:
		if (!IsHosting())
		{
			UpdateClientTime();
		}

		// Let the NetObjectSystem Update
		if (m_netObjectSystem != nullptr)
		{
			m_netObjectSystem->Update();
		}
		break;
	default:
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the session info to the screen for debugging
//
void NetSession::RenderDebugInfo() const
{
	AABB2 bounds = Renderer::GetUIBounds();
	Renderer* renderer = Renderer::GetInstance();

	float fontHeight = bounds.maxs.y * 0.03f;

	BitmapFont* font = AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png");
	renderer->DrawTextInBox2D("SESSION INFO", bounds, Vector2::ZERO, fontHeight, TEXT_DRAW_OVERRUN, font, Rgba::BLUE);

	bounds.Translate(Vector2(0.f, -fontHeight));
	fontHeight = bounds.maxs.y * 0.02f;

	std::string simText = Stringf("Simulated Lag: %.0fms-%.0fms | Simulated Loss: %.2f%%", m_latencyRange.min, m_latencyRange.max, m_lossChance * 100.f);
	renderer->DrawTextInBox2D(simText.c_str(), bounds, Vector2::ZERO, fontHeight, TEXT_DRAW_OVERRUN, font, Rgba::GRAY);
	bounds.Translate(Vector2(0.f, -fontHeight));

	std::string stateText = "State: ";
	switch (m_state)
	{
	case SESSION_DISCONNECTED:
		stateText += "Disconnected";
		break;
	case SESSION_BOUND:
		stateText += Stringf("Bound to address %s", m_boundSocket->GetNetAddress().ToString().c_str());
		break;
	case SESSION_CONNECTING:
		stateText += "Connecting...";
		break;
	case SESSION_JOINING:
		stateText += "Joining...";
		break;
	case SESSION_READY:
		stateText += "Ready";
		break;
	default:
		break;
	}
	renderer->DrawTextInBox2D(stateText, bounds, Vector2::ZERO, fontHeight, TEXT_DRAW_OVERRUN, font, Rgba::YELLOW);
	bounds.Translate(Vector2(0.f, -2.f * fontHeight));

	std::string netTimeText = Stringf("Net time: %.2f", GetCurrentNetTime());

	renderer->DrawTextInBox2D(netTimeText, bounds, Vector2::ZERO, fontHeight, TEXT_DRAW_OVERRUN, font, Rgba::YELLOW);
	bounds.Translate(Vector2(0.f, -2.f * fontHeight));

	renderer->DrawTextInBox2D("Connections:", bounds, Vector2::ZERO, fontHeight, TEXT_DRAW_OVERRUN, font);
	bounds.Translate(Vector2(0.f, -fontHeight));

	std::string headingText = Stringf("-- %-*s%-*s%-*s%-*s%-*s%-*s%-*s%-*s%-*s%-*s",
		6, "INDEX", 10, "NAME", 21, "ADDRESS", 8, "RTT(ms)", 7, "LOSS", 7, "LRCV", 7, "LSNT", 8, "SNTACK", 8, "RCVACK", 10, "STATE");

	renderer->DrawTextInBox2D(headingText.c_str(), bounds, Vector2::ZERO, fontHeight, TEXT_DRAW_OVERRUN, font);
	bounds.Translate(Vector2(0.f, -fontHeight));

	for (int index = 0; index < MAX_CONNECTIONS; ++index)
	{
		if (m_boundConnections[index] != nullptr)
		{
			std::string connectionInfo = m_boundConnections[index]->GetDebugInfo();

			renderer->DrawTextInBox2D(connectionInfo.c_str(), bounds, Vector2::ZERO, fontHeight, TEXT_DRAW_OVERRUN, font);
			bounds.Translate(Vector2(0.f, -fontHeight));
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Adds a call back for message handling
//
void NetSession::RegisterMessageDefinition(uint8_t messageID, const std::string& name, NetMessage_cb callback, eNetMessageOption options /*= NET_MSG_OPTION_NONE*/, uint8_t sequenceChannelIndex /*= 0*/)
{
	// Check for duplicates
	if (m_messageDefinitions[messageID] != nullptr)
	{
		LogTaggedPrintf("NET", "Warning - NetSession::RegisterMessageDefinition() registered duplicate definition id for \"%s\" and \"%s\"", m_messageDefinitions[messageID]->name.c_str(), name.c_str());
		delete m_messageDefinitions[messageID];
	}

	m_messageDefinitions[messageID] = new NetMessageDefinition_t(messageID, name, callback, options, sequenceChannelIndex);
}


//-----------------------------------------------------------------------------------------------
// Transitions the NetSession to the given state
//
void NetSession::TransitionToState(eSessionState state)
{
	m_state = state;

	m_joinTimer.Reset();
	m_stateTimer.Reset();
}


//-----------------------------------------------------------------------------------------------
// Adds a socket binding for sending and listening to connections
//
bool NetSession::BindSocket(unsigned short port, uint16_t portRange)
{
	if (m_boundSocket != nullptr)
	{
		if (!m_boundSocket->IsClosed())
		{
			m_boundSocket->Close();
		}

		delete m_boundSocket;
		m_boundSocket = nullptr;
	}

	UDPSocket* newSocket = new UDPSocket();

	NetAddress_t localAddress;
	bool foundLocal = NetAddress_t::GetLocalAddress(&localAddress, port, true);

	if (!foundLocal)
	{
		LogTaggedPrintf("NET", "NetSession::Bind() failed to bind to the local address.");
		return false;
	}

	bool bound = newSocket->Bind(localAddress, portRange);

	if (bound)
	{
		m_boundSocket = newSocket;
		m_isReceiving = true;
		m_receivingThread = std::thread(&NetSession::ReceiveIncoming, this);
		LogTaggedPrintf("NET", "NetSession bound to address %s", newSocket->GetNetAddress().ToString().c_str());
	}
	else
	{
		delete newSocket;
		
		if (m_isReceiving)
		{
			m_isReceiving = false;
			m_receivingThread.join();
		}

		LogTaggedPrintf("NET", "Error: NetSession::Bind() couldn't bind to address %s", newSocket->GetNetAddress().ToString().c_str());
	}

	return bound;
}


//-----------------------------------------------------------------------------------------------
// Sends the packet out of the bound socket
//
bool NetSession::SendPacket(const NetPacket* packet)
{
	NetConnection* connection = m_boundConnections[packet->GetReceiverConnectionIndex()];
	NetAddress_t address = connection->GetAddress();

	size_t amountSent = m_boundSocket->SendTo(address, packet->GetBuffer(), packet->GetWrittenByteCount());

	return amountSent > 0;
}


//-----------------------------------------------------------------------------------------------
// Returns the definition given by name
//
bool NetSession::SendMessageDirect(NetMessage* message, const NetSender_t& sender)
{
	NetPacket packet;
	packet.AdvanceWriteHead(PACKET_HEADER_SIZE);

	packet.WriteMessage(message);

	PacketHeader_t header;
	header.totalMessageCount = 1;
	header.senderConnectionIndex = INVALID_CONNECTION_INDEX;

	packet.WriteHeader(header);

	size_t amountSent = m_boundSocket->SendTo(sender.address, packet.GetBuffer(), packet.GetWrittenByteCount());

	return (amountSent > 0);
}


//-----------------------------------------------------------------------------------------------
// Broadcasts the given message to all connections except my connection
//
void NetSession::BroadcastMessage(NetMessage* message)
{
	bool firstSent = false;
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
	{
		if (m_boundConnections[i] != nullptr && m_boundConnections[i] != m_myConnection && m_boundConnections[i]->IsReady())
		{
			if (!firstSent)
			{
				m_boundConnections[i]->Send(message);
				firstSent = true;
			}
			else
			{
				NetMessage* copy = new NetMessage(*message);
				m_boundConnections[i]->Send(copy);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the definition given by name
//
const NetMessageDefinition_t* NetSession::GetMessageDefinition(const std::string& name) const
{
	for (int index = 0; index < MAX_MESSAGE_DEFINITIONS; ++index)
	{
		if (m_messageDefinitions[index] != nullptr && m_messageDefinitions[index]->name == name)
		{
			return m_messageDefinitions[index];
		}
	}

	ERROR_AND_DIE("Message definition doesn't exist");
}


//-----------------------------------------------------------------------------------------------
// Returns the definition at the given index
//
const NetMessageDefinition_t* NetSession::GetMessageDefinition(const uint8_t index)
{
	if (index >= MAX_MESSAGE_DEFINITIONS)
	{
		ERROR_AND_DIE("Message definition doesn't exist");
	}

	return m_messageDefinitions[index];
}


//-----------------------------------------------------------------------------------------------
// Returns the index of the message definition given by name
//
bool NetSession::GetMessageDefinitionIndex(const std::string& name, uint8_t& out_index)
{
	for (uint8_t index = 0; index < MAX_MESSAGE_DEFINITIONS; ++index)
	{
		if (m_messageDefinitions[index]->name == name)
		{
			out_index = index;
			return true;
		}
	}

	LogTaggedPrintf("NET", "Error - NetSession::GetMessageDefinition() couldn't find definition for name %s", name.c_str());
	return false;
}


//-----------------------------------------------------------------------------------------------
// Returns the NetConnection at the given index, nullptr if out of range
//
NetConnection* NetSession::GetConnection(uint8_t index) const
{
	if (index >= INVALID_CONNECTION_INDEX)
	{
		return nullptr;
	}

	return m_boundConnections[index];
}


//-----------------------------------------------------------------------------------------------
// Returns the index for the connection of this instance
//
uint8_t NetSession::GetLocalConnectionIndex() const
{
	if (m_myConnection == nullptr)
	{
		return INVALID_CONNECTION_INDEX;
	}

	return m_myConnection->GetSessionIndex();
}


//-----------------------------------------------------------------------------------------------
// Returns the connection that represents this session
//
NetConnection* NetSession::GetMyConnection() const
{
	return m_myConnection;
}


//-----------------------------------------------------------------------------------------------
// Returns the connection that represents the host in this session
//
NetConnection* NetSession::GetHostConnection() const
{
	return m_hostConnection;
}


//-----------------------------------------------------------------------------------------------
// Returns whether the bound connection list is full of active connections
//
bool NetSession::IsConnectionListFull() const
{
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
	{
		if (m_boundConnections[i] == nullptr)
		{
			return false;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
// Returns whether a connection already exists for the given sender
//
bool NetSession::DoesConnectionForAddressExist(const NetSender_t& sender) const
{
	if (sender.connectionIndex != INVALID_CONNECTION_INDEX)
	{
		return true;
	}

	// Sender doesn't have an index, but might be connectionless - check for the address
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
	{
		if (m_boundConnections[i] == nullptr)
		{
			continue;
		}

		if (m_boundConnections[i]->GetAddress() == sender.address)
		{
			return true;
		}
	}

	// Invalid connection index and couldn't find address with port, so it's new
	return false;
}


//-----------------------------------------------------------------------------------------------
// Returns the number of bound connections in the NetSession
//
unsigned int NetSession::GetConnectionCount() const
{
	unsigned int count = 0;
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
	{
		if (m_boundConnections[i] != nullptr)
		{
			count++;
		}
	}

	return count;
}


//-----------------------------------------------------------------------------------------------
// Receives on all current connections, calling callbacks on all receiving messages depending on their tag
//
void NetSession::ProcessIncoming()
{
	bool done = false;
	while (!done)
	{
		PendingReceive pending;
		bool packetReceived = GetNextReceive(pending);
		
		if (packetReceived)
		{
			if (VerifyPacket(pending.packet))
			{
				ProcessReceivedPacket(pending.packet, pending.senderAddress);
			}
			else
			{
				LogTaggedPrintf("NET", "Received a bad packet from address %s, message was %i bytes", pending.senderAddress.ToString().c_str(), pending.packet->GetWrittenByteCount());
			}

			delete pending.packet;
		}

		done = !packetReceived;
	}
}


//-----------------------------------------------------------------------------------------------
// Send out all pending messages
//
void NetSession::ProcessOutgoing()
{
	// Flush each connection
	for (int index = 0; index < MAX_CONNECTIONS; ++index)
	{
		NetConnection* currConnection = m_boundConnections[index];

		if (currConnection != nullptr)
		{
			// Check heartbeat
			if (currConnection != m_myConnection && currConnection->HasHeartbeatElapsed())
			{
				const NetMessageDefinition_t* definition = GetMessageDefinition("heartbeat");

				if (definition != nullptr)
				{
					NetMessage* message = new NetMessage(definition);
					message->Write(m_netClock.GetElapsedTime());

					currConnection->Send(message);
				}
			}

			// Check send rate
			if ((currConnection->HasOutboundMessages() || currConnection->NeedsToForceSend()))
			{
				currConnection->FlushMessages();
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the simulated loss for the NetSession
//
void NetSession::SetSimLoss(float lossAmount)
{
	m_lossChance = lossAmount;
}


//-----------------------------------------------------------------------------------------------
// Adds the given amounts to the min and max latency values, for simulating latency
//
void NetSession::SetSimLatency(float minLatency, float maxLatency = 0.f)
{
	minLatency = MaxFloat(minLatency, 0.f);
	maxLatency = MaxFloat(maxLatency, minLatency);
	m_latencyRange = FloatRange(minLatency, maxLatency);
}


//-----------------------------------------------------------------------------------------------
// Returns true if the session is shutting down
//
bool NetSession::IsReceiving() const
{
	return m_isReceiving;
}


//-----------------------------------------------------------------------------------------------
// Sets the network tick rate for the session
//
void NetSession::SetNetTickRate(float hertz)
{
	m_timeBetweenSends = (1.f / hertz);
}


//-----------------------------------------------------------------------------------------------
// Returns the time to wait between sending messages
//
float NetSession::GetTimeBetweenSends() const
{
	return m_timeBetweenSends;
}


//-----------------------------------------------------------------------------------------------
// Sets the heartbeat of all connections
//
void NetSession::SetConnectionHeartbeatInterval(float hertz)
{
	m_heartBeatInverval = (1.f / hertz);
}


//-----------------------------------------------------------------------------------------------
// Returns the heartbeat interval for the session
//
float NetSession::GetHeartbeatInterval() const
{
	return m_heartBeatInverval;
}


//-----------------------------------------------------------------------------------------------
// Returns the host time that this client last received
//
float NetSession::GetLastHostTime() const
{
	return m_lastHostTime;
}


//-----------------------------------------------------------------------------------------------
// Returns the current time of this session
//
float NetSession::GetCurrentNetTime() const
{
	if (IsHosting())
	{
		return m_netClock.GetElapsedTime();
	}

	return m_currentClientTime;
}


//-----------------------------------------------------------------------------------------------
// Returns the time that this client is trying to match
//
float NetSession::GetDesiredClientTime() const
{
	return m_desiredClientTime;
}


//-----------------------------------------------------------------------------------------------
// Returns the NetObjectSystem used by this session
//
NetObjectSystem* NetSession::GetNetObjectSystem() const
{
	return m_netObjectSystem;
}


//-----------------------------------------------------------------------------------------------
// Creates a connection with the given info and attempts to bind it if valid
//
NetConnection* NetSession::CreateConnection(const NetConnectionInfo_t& connectionInfo)
{
	NetConnection* connection = new NetConnection(this, connectionInfo);

	if (connectionInfo.sessionIndex != INVALID_CONNECTION_INDEX)
	{
		BindConnection(connectionInfo.sessionIndex, connection);
	}

	return connection;
}


//-----------------------------------------------------------------------------------------------
// Cleans up and destroys the given connection
//
void NetSession::DestroyConnection(NetConnection* connection)
{
	// Remove the connection view from the NetObjectSystem
	m_netObjectSystem->ClearConnectionViewForIndex(connection->GetSessionIndex());

	// Remove the connection from the list if bound connections if bound
	if (connection->IsConnected())
	{
		uint8_t index = connection->GetSessionIndex();
		m_boundConnections[index] = nullptr;

		// Call the game-side callback
		m_onLeaveCallback(connection);
	}

	// Clean up convenience pointers
	if (m_myConnection == connection)
	{
		m_myConnection = nullptr;
	}

	if (m_hostConnection == connection)
	{
		m_hostConnection = nullptr;
	}

	// Free up the memory
	delete connection;
}


//-----------------------------------------------------------------------------------------------
// Binds the given connection by putting it in the indexed list, removing it from the pending list
//
void NetSession::BindConnection(uint8_t index, NetConnection* connection)
{
	if (m_boundConnections[index] != nullptr)
	{
		LogTaggedPrintf("NET", "Error: NetSession::BindConnection() tried to bind connection address %s to index %i already in use", connection->GetAddress().ToString().c_str(), index);
		return;
	}

	m_boundConnections[index] = connection;

	// Update the connection's index stored internally
	connection->SetSessionIndex(index);

	// Flag it as bound
	connection->SetConnectionState(CONNECTION_BOUND);

	// Add a NetObjectConnectionView for the connection
	m_netObjectSystem->AddConnectionViewForIndex(index);

	// Call the game-side callback
	m_onJoinCallback(connection);
}


//-----------------------------------------------------------------------------------------------
// Returns an index in th
//
uint8_t NetSession::GetFreeConnectionIndex() const
{
	for (uint8_t i = 0; i < MAX_CONNECTIONS; ++i)
	{
		if (m_boundConnections[i] == nullptr)
		{
			return i;
		}
	}

	return INVALID_CONNECTION_INDEX;
}


//-----------------------------------------------------------------------------------------------
// Checks for any connections that haven't received any information in CONNECTION_LAST_SENT_TIMEOUT
// and disconnects/destroys any connections that haven't
//
void NetSession::CheckForDisconnects()
{
	for (uint8_t connectionIndex = 0; connectionIndex < MAX_CONNECTIONS; ++connectionIndex)
	{
		if (m_boundConnections[connectionIndex] != nullptr && m_boundConnections[connectionIndex] != m_myConnection) 
		{ 
			float lastReceivedTime = m_boundConnections[connectionIndex]->GetTimeSinceLastReceive();

			if (m_boundConnections[connectionIndex]->IsDisconnected() || lastReceivedTime >= CONNECTION_LAST_RECEIVED_TIMEOUT)
			{
				ConsolePrintf("%s timed out", m_boundConnections[connectionIndex]->GetName().c_str());
				LogTaggedPrintf("NET", "%s timed out", m_boundConnections[connectionIndex]->GetName().c_str());

				DestroyConnection(m_boundConnections[connectionIndex]);
			}
		}
	}

	if (m_state != SESSION_DISCONNECTED && m_hostConnection == nullptr)
	{
		ConsolePrintf("Lost connection to host");
		LogTaggedPrintf("NET", "Lost connection to host at address");

		ShutdownSession();
	}
}


//-----------------------------------------------------------------------------------------------
// Creates the NetMessageDefinitions for the core supported messages
//
void NetSession::RegisterCoreMessages()
{
	RegisterMessageDefinition(NET_MSG_PING, "ping", OnPing, NET_MSG_OPTION_CONNECTIONLESS);
	RegisterMessageDefinition(NET_MSG_PONG, "pong", OnPong, NET_MSG_OPTION_CONNECTIONLESS);
	RegisterMessageDefinition(NET_MSG_HEARTBEAT, "heartbeat", OnHeartBeat);

	// Host/Join
	RegisterMessageDefinition(NET_MSG_JOIN_REQUEST, "join_request", OnJoinRequest, NET_MSG_OPTION_CONNECTIONLESS);
	RegisterMessageDefinition(NET_MSG_JOIN_DENY, "join_deny", OnJoinDeny, NET_MSG_OPTION_CONNECTIONLESS);
	RegisterMessageDefinition(NET_MSG_JOIN_ACCEPT, "join_accept", OnJoinAccept, NET_MSG_OPTION_IN_ORDER);
	RegisterMessageDefinition(NET_MSG_NEW_CONNECTION, "new_connection", OnNewConnection, NET_MSG_OPTION_IN_ORDER);
	RegisterMessageDefinition(NET_MSG_HOST_FINISHED_SETUP, "host_setup_complete", OnHostFinishedSettingClientUp, NET_MSG_OPTION_IN_ORDER);
	RegisterMessageDefinition(NET_MSG_CLIENT_JOIN_FINISHED, "client_join_finished", OnClientFinishedTheirSetup, NET_MSG_OPTION_IN_ORDER);
	RegisterMessageDefinition(NET_MSG_HANG_UP, "hang_up", OnHangUp);

	// NetObjectSystem
	RegisterMessageDefinition(NET_MSG_OBJ_CREATE, "netobj_create", OnNetObjectCreate, NET_MSG_OPTION_IN_ORDER);
	RegisterMessageDefinition(NET_MSG_OBJ_DESTROY, "netobj_destroy", OnNetObjectDestroy, NET_MSG_OPTION_IN_ORDER);
	RegisterMessageDefinition(NET_MSG_OBJ_UPDATE, "netobj_update", OnNetObjectUpdate);
}


//-----------------------------------------------------------------------------------------------
// Receives on the bound sockets, and adds the messages to the queue
// Runs on a separate thread
//
void NetSession::ReceiveIncoming()
{
	while (m_isReceiving)
	{
		NetAddress_t senderAddress;
		uint8_t buffer[PACKET_MTU];

		size_t amountReceived = m_boundSocket->ReceiveFrom(&senderAddress, buffer, PACKET_MTU);

		if (amountReceived > 0)
		{
			// Check if we should keep the packet, or simulate loss
			if (!CheckRandomChance(m_lossChance))
			{
				NetPacket* packet = new NetPacket(buffer, PACKET_MTU);

				packet->AdvanceWriteHead(amountReceived);

				PendingReceive pending;
				pending.packet = packet;
				pending.senderAddress = senderAddress;

				float latency = m_latencyRange.GetRandomInRange() * 0.001f;
				pending.timeStamp = Clock::GetMasterClock()->GetTotalSeconds() + latency;

				PushNewReceive(pending);
			}
		}
	}

	LogTaggedPrintf("NET", "NetSession Receive thread joined");
}


//-----------------------------------------------------------------------------------------------
// Pushes a new receive in the correct location in the location array
//
void NetSession::PushNewReceive(PendingReceive& pending)
{
	m_receiveLock.lock();
	bool pushed = false;
	for (unsigned int i = 0; i < (unsigned int) m_receiveQueue.size(); ++i)
	{
		if (m_receiveQueue[i].timeStamp > pending.timeStamp)
		{
			m_receiveQueue.insert(m_receiveQueue.begin() + i, pending);
			pushed = true;
			break;
		}
	}

	if (!pushed)
	{
		m_receiveQueue.push_back(pending);
	}

	m_receiveLock.unlock();
}


//-----------------------------------------------------------------------------------------------
// Gets the next packet to receive, returning false if there are no packets to return
//
bool NetSession::GetNextReceive(PendingReceive& out_pending)
{
	m_receiveLock.lock();

	if (m_receiveQueue.size() == 0)
	{
		m_receiveLock.unlock();
		return false;
	}
	
	// Else check if the first packet is ready to be processed
	float currTime = Clock::GetMasterClock()->GetTotalSeconds();

	bool packetReady = false;
	if (m_receiveQueue[0].timeStamp <= currTime)
	{
		out_pending = m_receiveQueue[0];
		m_receiveQueue.erase(m_receiveQueue.begin());
		packetReady = true;
	}

	m_receiveLock.unlock();
	return packetReady;
}


//-----------------------------------------------------------------------------------------------
// Verifies that the packet is of correct format
//
bool NetSession::VerifyPacket(NetPacket* packet)
{
	// Enough room for the packet header
	PacketHeader_t header;
	if (!packet->ReadHeader(header))
	{
		LogTaggedPrintf("NET", "NetSession::VerifyPacket() failed, packet was less than size of header.");
		return false;
	}

	// Check the sender connection index, and ensure it is valid
	// (Index isn't the invalid index but is either out of range or has no connection associated with it)
	int connIndex = header.senderConnectionIndex;
	if (connIndex != INVALID_CONNECTION_INDEX && (connIndex >= MAX_CONNECTIONS || m_boundConnections[connIndex] == nullptr))
	{
		return false;
	}

	// Peek at the header and verify the messages
	uint8_t messageCount = header.totalMessageCount;

	for (int i = 0; i < messageCount; ++i)
	{
		uint16_t messageSize;
		packet->Read(messageSize);

		bool couldAdvance = packet->AdvanceReadHead(messageSize);

		if (!couldAdvance)
		{
			LogTaggedPrintf("NET", "NetSession::VerifyPacket() failed, packet message count and size went over the packet size.");
			return false;
		}
	}

	// Check for under the limit
	if (packet->GetRemainingReadableByteCount() > 0)
	{
		LogTaggedPrintf("NET", "NetSession::VerifyPacket() failed, packet message count and sizes were under the packet size.");
		return false;
	}
	
	// Reset the read, packet was good
	packet->ResetRead();
	return true;
}


//-----------------------------------------------------------------------------------------------
// Processes the received packet by calling all message callbacks for messages within the packet
//
void NetSession::ProcessReceivedPacket(NetPacket* packet, const NetAddress_t& senderAddress)
{
	// Get the messages out
	PacketHeader_t header;
	packet->ReadHeader(header);
	packet->SetSenderConnectionIndex(header.senderConnectionIndex);

	NetConnection* connection = GetConnection(header.senderConnectionIndex);

	// Update the connection's acknowledgment data if there is one
	if (connection != nullptr)
	{
		connection->OnPacketReceived(header);
	}

	// Process the messages
	uint8_t messageCount = header.totalMessageCount;

	for (int i = 0; i < messageCount; ++i)
	{
		NetMessage message;
		packet->ReadMessage(&message, this); // Need to pass the session to look up the definition

		ConsolePrintf("Received message: %s", message.GetDefinition()->name.c_str());

		// Check if we should process it
		bool shouldProcess = ShouldMessageBeProcessed(&message, connection);

		if (shouldProcess)
		{
			// Process
			ProcessReceivedMessage(&message, senderAddress, header.senderConnectionIndex);

			// In order - check all messages in sequence after it
			if (message.IsInOrder())
			{
				uint8_t sequenceChannelID = message.GetSequenceChannelID();
				NetSequenceChannel* channel = connection->GetSequenceChannel(sequenceChannelID);

				// Process all messages that we have after that in order
				NetMessage* nextMessage = channel->GetNextMessageToProcess();

				while (nextMessage != nullptr)
				{
					ProcessReceivedMessage(nextMessage, senderAddress, header.senderConnectionIndex);
				
					delete nextMessage;
					nextMessage = channel->GetNextMessageToProcess();
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks if the message should be processed right now, returning true if so
//
bool NetSession::ShouldMessageBeProcessed(NetMessage* message, NetConnection* connection)
{
	// Requires connection check
	bool connectionExists = (connection != nullptr);
	if (message->RequiresConnection() && !connectionExists)
	{
		LogTaggedPrintf("NET", "Received message \"%s\" from a connectionless client that requires a connection", message->GetName().c_str());
		return false;
	}

	// Double process check
	if (message->IsReliable() && connection->HasReliableIDAlreadyBeenReceived(message->GetReliableID()))
	{
		return false;
	}

	// Sequence Channel ID check
	NetSequenceChannel* channel = connection->GetSequenceChannel(message->GetSequenceChannelID());
	if (message->IsInOrder() && channel == nullptr)
	{
		LogTaggedPrintf("NET", "ProcessIncoming received in-order message with a bad sequence channel ID, ID was %i", message->GetSequenceChannelID());
		return false;
	}

	// In order check - queue it for later
	if (message->IsInOrder() && !connection->IsNextMessageInSequence(message))
	{
		NetMessage* queuedMessage = new NetMessage();
		*queuedMessage = std::move(*message);

		connection->QueueInOrderMessage(queuedMessage);
		return false;
	}

	// Return true - signaling we should process it
	return true;
}


//-----------------------------------------------------------------------------------------------
// Processes the message
//
void NetSession::ProcessReceivedMessage(NetMessage* message, const NetAddress_t& address, uint8_t connectionIndex)
{
	NetSender_t sender;
	sender.address = address;
	sender.netSession = this;
	sender.connectionIndex = connectionIndex;

	const NetMessageDefinition_t* definition = message->GetDefinition();
	definition->callback(message, sender);

	NetConnection* connection = GetConnection(sender.connectionIndex);
	if (message->IsReliable() && connection != nullptr)
	{
		connection->AddProcessedReliableID(message->GetReliableID());
	}

	// Increment the next id to expect for in order traffic
	if (message->IsInOrder() && connection != nullptr)
	{
		NetSequenceChannel* channel = connection->GetSequenceChannel(message->GetSequenceChannelID());

		if (channel != nullptr)
		{
			channel->IncrementNextExpectedID();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the given client time to dilate towards the target time
//
void NetSession::UpdateClientTime()
{
	float deltaTime = m_netClock.GetDeltaSeconds();
	m_desiredClientTime += deltaTime;

	float timeWithDT = m_currentClientTime + deltaTime;
	if (timeWithDT > m_desiredClientTime)
	{
		float minDilation = (1.0f - NET_MAX_TIME_DILATION) * deltaTime;
		m_currentClientTime += minDilation;
		m_currentClientTime = ClampFloat(m_currentClientTime, m_desiredClientTime, timeWithDT);
	}
	else if (timeWithDT < m_desiredClientTime)
	{
		float maxDilation = (1.0f + NET_MAX_TIME_DILATION) * deltaTime;
		m_currentClientTime += maxDilation;
		m_currentClientTime = ClampFloat(m_currentClientTime, timeWithDT, m_desiredClientTime);
	}
}


//-----------------------------------------------------------------------------------------------
// Message callbacks
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
// Called when a ping is received, responds with a pong
//
bool OnPing(NetMessage* msg, const NetSender_t& sender)
{
	std::string str;
	msg->ReadString(str);

	ConsolePrintf("Received ping from %s: %s", sender.address.ToString().c_str(), str.c_str());

	// Respond with a pong
	const NetMessageDefinition_t* definition = sender.netSession->GetMessageDefinition("pong");
	if (definition == nullptr)
	{
		return false;
	}

	NetMessage message(definition);
	NetConnection* connection = sender.netSession->GetConnection(sender.connectionIndex);


	if (connection != nullptr)
	{
		connection->Send(&message);
	}
	else
	{
		sender.netSession->SendMessageDirect(&message, sender);
	}

	// all messages serve double duty
	// do some work, and also validate
	// if a message ends up being malformed, we return false
	// to notify the session we may want to kick this connection; 
	return true;
}


//-----------------------------------------------------------------------------------------------
// Called when a pong message is received, after a ping is sent
//
bool OnPong(NetMessage* msg, const NetSender_t& sender)
{
	UNUSED(msg);
	ConsolePrintf("Received pong from %s", sender.address.ToString().c_str());

	return true;
}


//-----------------------------------------------------------------------------------------------
// Heartbeat message callback
//
bool OnHeartBeat(NetMessage* msg, const NetSender_t& sender)
{
	// Read off the host's time
	float hostTime;
	msg->Read(hostTime);

	if (hostTime > sender.netSession->GetLastHostTime())
	{
		sender.netSession->m_lastHostTime = hostTime + 0.5f * sender.netSession->GetHostConnection()->GetRTT();
		sender.netSession->m_desiredClientTime = sender.netSession->m_lastHostTime;
	}

	return true;
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Join request message callback
//
bool OnJoinRequest(NetMessage* msg, const NetSender_t& sender)
{
	UNUSED(msg);

	NetSession* session = sender.netSession;

	// Check to ignore
	if (session->DoesConnectionForAddressExist(sender))
	{
		return false;
	}

	// Check to deny
	bool isHosting = session->IsHosting();
	bool isFull = session->IsConnectionListFull();

	if (isHosting && !isFull)
	{
		// Create a connection for this
		NetConnectionInfo_t info;
		info.address = sender.address;
		info.name = ""; // Fill in later
		info.sessionIndex = session->GetFreeConnectionIndex();

		NetConnection* connection = session->CreateConnection(info);

		NetMessage* acceptMsg = new NetMessage(session->GetMessageDefinition("join_accept"));
		connection->Send(acceptMsg);

		// Need to send the client all the info of the other connections
		NetMessage* finishedMessage = new NetMessage(session->GetMessageDefinition("host_setup_complete"));

		// Write their new index
		finishedMessage->Write(info.sessionIndex);

		// Write the host name
		finishedMessage->WriteString(session->GetMyConnection()->GetName());

		// Write all other connections to the packet (skip host)
		uint8_t connectionCount = (uint8_t) session->GetConnectionCount() - 2; // Remove the client and the host
		finishedMessage->Write(connectionCount);

		// Write each info
		for (uint8_t i = 1; i < MAX_CONNECTIONS; ++i)
		{
			if (i == info.sessionIndex) { continue; } // Don't send them their own info

			NetConnection* currConnection = session->GetConnection(i);
		
			if (currConnection == nullptr) { continue; }

			finishedMessage->WriteString(currConnection->GetName());
			finishedMessage->Write(currConnection->GetSessionIndex());
			finishedMessage->WriteString(currConnection->GetAddress().ToString().c_str());
		}

		// At the end, write the host's current time
		finishedMessage->Write(session->m_netClock.GetElapsedTime());

		connection->Send(finishedMessage);
	}
	else
	{
		// Need to deny the request
		std::string error = "Error: ";
		if (!isHosting)
		{
			error += "Recipient isn't hosting";
		}
		else if (isFull)
		{
			error += "Recipient is full on connections";
		}

		NetMessage* denyMsg = new NetMessage(session->GetMessageDefinition("join_deny"));
		denyMsg->WriteString(error);

		session->SendMessageDirect(denyMsg, sender);
	}

	return true;
}


//- C FUNCTION ----------------------------------------------------------------------------------
// When a join deny message is received, just print
//
bool OnJoinDeny(NetMessage* msg, const NetSender_t& sender)
{
	std::string errorMessage;
	msg->ReadString(errorMessage);

	ConsoleErrorf("Failed to join host at address %s - %s", sender.address.ToString().c_str(), errorMessage.c_str());
	LogTaggedPrintf("NET", "Failed to join host at address %s - %s", sender.address.ToString().c_str(), errorMessage.c_str());

	ConsoleErrorf("Disconnecting session");
	sender.netSession->ShutdownSession();

	return true;
}


//- C FUNCTION ----------------------------------------------------------------------------------
// When the host accepts our join request
//
bool OnJoinAccept(NetMessage* msg, const NetSender_t& sender)
{
	UNUSED(msg);
	UNUSED(sender);

	LogTaggedPrintf("NET", "Host at address %s accepted join request", sender.address.ToString().c_str());
	return true;
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Function callback for when a new connection is added to the session
//
bool OnNewConnection(NetMessage* msg, const NetSender_t& sender)
{
	std::string name;
	uint8_t index;
	std::string address;

	msg->ReadString(name);
	msg->Read(index);
	msg->ReadString(address);

	// Don't add the new connection if we already have one at that index
	if (sender.netSession->GetConnection(index) != nullptr)
	{
		return false;
	}

	NetConnectionInfo_t info;
	info.address = NetAddress_t(address.c_str());
	info.name = name;
	info.sessionIndex = index;

	NetConnection* newConnection = sender.netSession->CreateConnection(info);

	newConnection->SetConnectionState(CONNECTION_READY);

	ConsolePrintf("%s connected", name.c_str());
	LogTaggedPrintf("NET", "%s connected with address %s", info.name.c_str(), address.c_str());

	return true;
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Function callback for when the host is finished setting up a connection
//
bool OnHostFinishedSettingClientUp(NetMessage* msg, const NetSender_t& sender)
{
	uint8_t myIndex;

	if (!msg->Read(myIndex))
	{
		ConsoleErrorf("Couldn't read Join Accept indices from message");
		return false;
	}

	NetConnection* myConnection = sender.netSession->GetMyConnection();

	// Bind my connection
	sender.netSession->BindConnection(myIndex, myConnection);

	// Also update the host's name
	std::string hostName;
	msg->ReadString(hostName);

	NetConnection* hostConnection = sender.netSession->GetHostConnection();
	hostConnection->UpdateName(hostName);

	// Now make all the connections...
	uint8_t connectionCount;
	msg->Read(connectionCount);

	for (int i = 0; i < connectionCount; ++i)
	{
		std::string addressString, name;
		uint8_t index;

		msg->ReadString(name);
		msg->Read(index);
		msg->ReadString(addressString);

		NetConnectionInfo_t info;
		info.address = NetAddress_t(addressString.c_str());
		info.sessionIndex = index;
		info.name = name;

		NetConnection* connection = sender.netSession->CreateConnection(info);
		connection->SetConnectionState(CONNECTION_READY);
	}
	
	// Read off the host's time
	float hostTime;
	msg->Read(hostTime);

	if (hostTime > sender.netSession->GetLastHostTime())
	{
		sender.netSession->m_lastHostTime = hostTime + 0.5f * hostConnection->GetRTT();
		sender.netSession->m_desiredClientTime = sender.netSession->m_lastHostTime;
		sender.netSession->m_currentClientTime = sender.netSession->m_lastHostTime;
	}

	// No other work to do, so mark connections as ready
	myConnection->SetConnectionState(CONNECTION_READY);
	hostConnection->SetConnectionState(CONNECTION_READY);

	ConsolePrintf("Connected to host %s at address %s", hostName.c_str(), sender.address.ToString().c_str());
	LogTaggedPrintf("NET", "Connected to host %s at address %s", hostName.c_str(), sender.address.ToString().c_str());

	// Let the host know we're ready, and what our name is
	NetMessage* finishedMsg = new NetMessage(sender.netSession->GetMessageDefinition("client_join_finished"));
	finishedMsg->WriteString(myConnection->GetName());

	hostConnection->Send(finishedMsg);

	return true;
}


//-----------------------------------------------------------------------------------------------
// Handler for when the connection is ready on the sender's end, and we can mark it ready here
//
bool OnClientFinishedTheirSetup(NetMessage* msg, const NetSender_t& sender)
{
	NetConnection* connection = sender.netSession->GetConnection(sender.connectionIndex);

	if (connection == nullptr)
	{
		return false;
	}

	// Get the client's name
	std::string clientName;
	msg->ReadString(clientName);

	connection->UpdateName(clientName);

	ConsolePrintf("%s connected", clientName.c_str());
	LogTaggedPrintf("NET", "%s connected with address %s", clientName.c_str(), connection->GetAddress().ToString().c_str());

	// Mark the connection ready
	connection->SetConnectionState(CONNECTION_READY);

	// Send all the NetObject construction messages
	std::vector<NetMessage*> createMessages = sender.netSession->GetNetObjectSystem()->GetMessagesToConstructAllNetObjects();

	for (int i = 0; i < createMessages.size(); ++i)
	{
		connection->Send(createMessages[i]);
	}

	// Have the host tell everyone of the new addition
	NetMessage* message = new NetMessage(sender.netSession->GetMessageDefinition("new_connection"));
	message->WriteString(clientName);
	message->Write(connection->GetSessionIndex());
	message->WriteString(connection->GetAddress().ToString().c_str());

	sender.netSession->BroadcastMessage(message);

	return true;
}


//-----------------------------------------------------------------------------------------------
// Callback for when a hang up message is received
//
bool OnHangUp(NetMessage* msg, const NetSender_t& sender)
{
	UNUSED(msg);

	NetConnection* connection = sender.netSession->GetConnection(sender.connectionIndex);

	ConsolePrintf("%s disconnected", connection->GetName().c_str());
	LogTaggedPrintf("NET", "%s disconnected", connection->GetName().c_str());

	sender.netSession->DestroyConnection(connection);

	return true;
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Callback for when a net object create message is received
//
bool OnNetObjectCreate(NetMessage* msg, const NetSender_t& sender)
{
	NetObjectSystem *netObjSystem = sender.netSession->GetNetObjectSystem();

	uint8_t typeID = 0xff;
	uint16_t networkID = 0xffff;

	if (!msg->Read(typeID) || !msg->Read(networkID))
	{
		ERROR_AND_DIE("Error: OnNetObjectCreate() couldn't read the necessary ID's");
	}

	const NetObjectType_t* type = netObjSystem->GetNetObjectTypeForTypeID(typeID);

	// Read off the rest of the game receive data from the message
	void* localObject = type->readCreate(*msg);

	if (localObject != nullptr) 
	{
		NetObject *netObj = new NetObject(type, networkID, localObject, false);

		netObjSystem->RegisterNetObject(netObj);

		return true;
	}

	return false;
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Callback for when a NetObject Destroy message is received
//
bool OnNetObjectDestroy(NetMessage* msg, const NetSender_t& sender)
{
	NetObjectSystem* netObjSystem = sender.netSession->GetNetObjectSystem();

	uint16_t networkID = 0xffff;

	if (!msg->Read(networkID))
	{
		ERROR_AND_DIE("Error: OnNetObjectCreate() couldn't read the network ID");
	}

	NetObject* netObject = netObjSystem->UnregisterNetObject(networkID);

	ASSERT_OR_DIE(netObject != nullptr, "Error: OnNetObjectDestroy() couldnt' find NetObject");

	netObject->GetNetObjectType()->readDestroy(msg, netObject->GetLocalObject());

	delete netObject;
	return true;
}



//- C FUNCTION ----------------------------------------------------------------------------------
// Callback for when a NetObject Update message is received
//
bool OnNetObjectUpdate(NetMessage* msg, const NetSender_t& sender)
{
	NetObjectSystem* netObjSystem = sender.netSession->GetNetObjectSystem();

	uint16_t networkID = 0xffff;

	if (!msg->Read(networkID))
	{
		ERROR_AND_DIE("Error: OnNetObjectCreate() couldn't read the network ID");
	}

	NetObject* netObject = netObjSystem->GetNetObjectForNetworkID(networkID);

	if (netObject == nullptr)
	{
		return false;
	}

	const NetObjectType_t* type = netObject->GetNetObjectType();
	type->readSnapshot(*msg, netObject->GetLastReceivedSnapshot());

	return true;
}