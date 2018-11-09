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
#include "Engine/Networking/UDPSocket.hpp"
#include "Engine/Networking/NetPacket.hpp"
#include "Engine/Networking/NetSession.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Networking/NetConnection.hpp"

// Message callbacks
bool OnPing(NetMessage* msg, const NetSender_t& sender);
bool OnPong(NetMessage* msg, const NetSender_t& sender);
bool OnHeartBeat(NetMessage* msg, const NetSender_t& sender);

//-----------------------------------------------------------------------------------------------
// Constructor
//
NetSession::NetSession()
{
	RegisterCoreMessages();
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
NetSession::~NetSession()
{
	m_isReceiving = false;
	if (m_receivingThread.joinable())
	{
		m_receivingThread.join();
	}

	if (m_boundSocket != nullptr)
	{
		m_boundSocket->Close();
		delete m_boundSocket;
		m_boundSocket = nullptr;
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
		stateText += "Ready(?)";
		break;
	default:
		break;
	}
	renderer->DrawTextInBox2D(stateText, bounds, Vector2::ZERO, fontHeight, TEXT_DRAW_OVERRUN, font, Rgba::YELLOW);
	bounds.Translate(Vector2(0.f, -2.f * fontHeight));

	renderer->DrawTextInBox2D("Connections:", bounds, Vector2::ZERO, fontHeight, TEXT_DRAW_OVERRUN, font);
	bounds.Translate(Vector2(0.f, -fontHeight));

	std::string headingText = Stringf("-- %-*s%-*s%-*s%-*s%-*s%-*s%-*s%-*s%-*s",
		6, "INDEX", 21, "ADDRESS", 8, "RTT(ms)", 7, "LOSS", 7, "LRCV", 7, "LSNT", 8, "SNTACK", 8, "RCVACK", 10, "RCVBITS");

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
void NetSession::RegisterMessageDefinition(uint8_t messageID, const std::string& name, NetMessage_cb callback, eNetMessageOption options /*= NET_MSG_OPTION_NONE*/)
{
	// Check for duplicates
	if (m_messageDefinitions[messageID] != nullptr)
	{
		LogTaggedPrintf("NET", "Warning - NetSession::RegisterMessageDefinition() registered duplicate definition id for \"%s\" and \"%s\"", m_messageDefinitions[messageID]->name.c_str(), name.c_str());
		delete m_messageDefinitions[messageID];
	}

	m_messageDefinitions[messageID] = new NetMessageDefinition_t(messageID, name, callback, options);
}


//-----------------------------------------------------------------------------------------------
// Adds a socket binding for sending and listening to connections
//
bool NetSession::BindSocket(unsigned short port, uint16_t portRange)
{
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
// Returns the definition given by name
//
const NetMessageDefinition_t* NetSession::GetMessageDefinition(const std::string& name)
{
	for (int index = 0; index < MAX_MESSAGE_DEFINITIONS; ++index)
	{
		if (m_messageDefinitions[index] != nullptr && m_messageDefinitions[index]->name == name)
		{
			return m_messageDefinitions[index];
		}
	}

	LogTaggedPrintf("NET", "Error - NetSession::GetMessageDefinition() couldn't find definition for name %s", name.c_str());
	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Returns the definition at the given index
//
const NetMessageDefinition_t* NetSession::GetMessageDefinition(const uint8_t index)
{
	if (index >= MAX_MESSAGE_DEFINITIONS)
	{
		LogTaggedPrintf("NET", "Error - NetSession::GetMessageDefinition() received index out of range, index was %i", index);
		return nullptr;
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
// Closes all connections in the session
//
void NetSession::CloseAllConnections()
{
	for (int index = 0; index < MAX_CONNECTIONS; ++index)
	{
		delete m_boundConnections[index];
		m_boundConnections[index] = nullptr;
	}
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
			if (currConnection->HasHeartbeatElapsed())
			{
				const NetMessageDefinition_t* definition = GetMessageDefinition("heartbeat");

				if (definition != nullptr)
				{
					currConnection->Send(new NetMessage(definition));
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
// Creates a connection with the given info and attempts to bind it if valid
//
NetConnection* NetSession::CreateConnection(const NetConnectionInfo_t& connectionInfo)
{
	NetConnection* connection = new NetConnection(this, connectionInfo);
	m_pendingConnections.push_back(connection);

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
	// Clean up convenience pointers
	if (m_myConnection == connection)
	{
		m_myConnection = nullptr;
	}

	if (m_hostConnection == nullptr)
	{
		m_hostConnection = nullptr;
	}

	// Remove the connection from the list if bound connections if bound
	if (connection->IsBound())
	{
		uint8_t index = connection->GetSessionIndex();
		m_boundConnections[index] = nullptr;
	}

	// Remove it from the list of all connections
	for (int i = 0; i < (int)m_pendingConnections.size(); ++i)
	{
		if (m_pendingConnections[i] == connection)
		{
			m_pendingConnections.erase(m_pendingConnections.begin() + i);
			break;
		}
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

	// Also remove the connection from our list of pending connections if it exists there
	for (int i = 0; i < (int)m_pendingConnections.size(); ++i)
	{
		if (m_pendingConnections[i] == connection)
		{
			m_pendingConnections.erase(m_pendingConnections.begin() + i);
			break;
		}
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
		ConsoleErrorf("OnPing couldn't find definition for message named \"pong\"");
		return false;
	}

	NetMessage message(definition);
	NetConnection* connection = sender.netSession->GetConnection(sender.connectionIndex);


	if (connection != nullptr)
	{
		connection->Send(&message);
		ConsolePrintf(Rgba::GREEN, "Sent a message to connection %i", sender.connectionIndex);
	}
	else
	{
		sender.netSession->SendMessageDirect(&message, sender);
		ConsolePrintf(Rgba::GREEN, "Sent an indirect message to address %s", sender.address.ToString().c_str());
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
	//ConsolePrintf("Heartbeat received from %s", sender.address.ToString().c_str());
	UNUSED(msg);
	UNUSED(sender);

	return true;
}