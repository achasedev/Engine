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


//-----------------------------------------------------------------------------------------------
// Constructor
//
NetSession::NetSession()
{
	// Register the heartbeat definition
	RegisterMessageDefinition("heartbeat", OnHeartBeat);
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
NetSession::~NetSession()
{
	m_isReceiving = false;
	m_receivingThread.join();

	m_boundSocket->Close();
	delete m_boundSocket;
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

	renderer->DrawTextInBox2D(Stringf("Socket Address: %s", m_boundSocket->GetNetAddress().ToString().c_str()).c_str(), bounds, Vector2::ZERO, fontHeight, TEXT_DRAW_OVERRUN, font, Rgba::YELLOW);
	bounds.Translate(Vector2(0.f, -2.f * fontHeight));

	renderer->DrawTextInBox2D("Connections:", bounds, Vector2::ZERO, fontHeight, TEXT_DRAW_OVERRUN, font);
	bounds.Translate(Vector2(0.f, -fontHeight));

	std::string headingText = Stringf("-- %-*s%-*s%-*s%-*s%-*s%-*s%-*s%-*s%-*s",
		6, "INDEX", 21, "ADDRESS", 8, "RTT(ms)", 7, "LOSS", 7, "LRCV", 7, "LSNT", 8, "SNTACK", 8, "RCVACK", 10, "RCVBITS");

	renderer->DrawTextInBox2D(headingText.c_str(), bounds, Vector2::ZERO, fontHeight, TEXT_DRAW_OVERRUN, font);
	bounds.Translate(Vector2(0.f, -fontHeight));

	int numConnections = (int) m_connections.size();

	for (int index = 0; index < numConnections; ++index)
	{
		if (m_connections[index] != nullptr)
		{
			std::string connectionInfo = m_connections[index]->GetDebugInfo();

			renderer->DrawTextInBox2D(connectionInfo.c_str(), bounds, Vector2::ZERO, fontHeight, TEXT_DRAW_OVERRUN, font);
			bounds.Translate(Vector2(0.f, -fontHeight));
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Adds a call back for message handling
//
void NetSession::RegisterMessageDefinition(const std::string& name, NetMessage_cb callback)
{
	// Check for duplicates
	for (int index = 0; index < (int) m_messageDefinitions.size(); ++index)
	{
		if (m_messageDefinitions[index]->name == name)
		{
			LogTaggedPrintf("NET", "Warning - NetSession::RegisterMessageDefinition() registered duplicate definition name \"%s\"", name.c_str());

			delete m_messageDefinitions[index];
			m_messageDefinitions.erase(m_messageDefinitions.begin() + index);
			break;
		}
	}

	m_messageDefinitions.push_back(new NetMessageDefinition_t(name, callback));
}


//-----------------------------------------------------------------------------------------------
// Adds a socket binding for sending and listening to connections
//
bool NetSession::Bind(unsigned short port, uint16_t portRange)
{
	UDPSocket* newSocket = new UDPSocket();

	NetAddress_t localAddress;
	bool foundLocal = NetAddress_t::GetLocalAddress(&localAddress, port, true);

	if (!foundLocal)
	{
		//ConsoleErrorf("Couldn't find local address to bind to");
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

	// We're bound, so now we're available for connections
	// Now is a good time to sort the definition vector
	SortDefinitions();

	return bound;
}


//-----------------------------------------------------------------------------------------------
// Sends the packet out of the bound socket
//
bool NetSession::SendPacket(const NetPacket* packet)
{
	NetConnection* connection = m_connections[packet->GetReceiverConnectionIndex()];
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
	packet.AdvanceWriteHead(sizeof(PacketHeader_t));

	packet.WriteMessage(message);

	PacketHeader_t header;
	header.unreliableMessageCount = 1;
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
	for (int index = 0; index < (int)m_messageDefinitions.size(); ++index)
	{
		if (m_messageDefinitions[index]->name == name)
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
	if (index > (uint8_t)m_messageDefinitions.size())
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
	for (uint8_t index = 0; index < (uint8_t)m_messageDefinitions.size(); ++index)
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
// Create and add a new connection
//
bool NetSession::AddConnection(uint8_t connectionIndex, NetAddress_t address)
{
	// Keep it in range of the vector
	if (connectionIndex >= (uint8_t)m_connections.size())
	{
		m_connections.resize(connectionIndex + 1);
	}

	if (m_connections[connectionIndex] != nullptr)
	{
		LogTaggedPrintf("NET", "Warning: NetSession::AddConnection() tried to add a connection to an already existing connection index, index was %i", connectionIndex);
		return false;
	}

	NetConnection* newConnection = new NetConnection(address, this, connectionIndex);
	m_connections[connectionIndex] = newConnection;

	// Check if this is our address, and update it if so
	if (m_boundSocket->GetNetAddress() == address)
	{
		m_localConnectionIndex = connectionIndex;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
// Closes all connections in the session
//
void NetSession::CloseAllConnections()
{
	for (int index = 0; index < (int)m_messageDefinitions.size(); ++index)
	{
		delete m_connections[index];
	}

	m_connections.clear();
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

	return m_connections[index];
}


//-----------------------------------------------------------------------------------------------
// Returns the index for the connection of this instance
//
uint8_t NetSession::GetLocalConnectionIndex() const
{
	return m_localConnectionIndex;
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
	for (int index = 0; index < m_connections.size(); ++index)
	{
		NetConnection* currConnection = m_connections[index];

		if (currConnection != nullptr)
		{
			// Check heartbeat
			if (currConnection->HasHeartbeatElapsed())
			{
				uint8_t definitionIndex;
				bool found = GetMessageDefinitionIndex("heartbeat", definitionIndex);

				if (found)
				{
					currConnection->Send(new NetMessage(definitionIndex));
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
	else
	{
		ConsoleErrorf("Packet in queue but not ready!");
	}

	m_receiveLock.unlock();
	return packetReady;
}


//-----------------------------------------------------------------------------------------------
// Sorts the NetMessageDefinition collection to validate the indices
//
void NetSession::SortDefinitions()
{
	for (int i = 0; i < (int)m_messageDefinitions.size() - 1; ++i)
	{
		for (int j = i + 1; j < (int)m_messageDefinitions.size(); ++j)
		{
			if (m_messageDefinitions[j]->name < m_messageDefinitions[i]->name)
			{
				const NetMessageDefinition_t* temp = m_messageDefinitions[i];
				m_messageDefinitions[i] = m_messageDefinitions[j];
				m_messageDefinitions[j] = temp;
			}
		}
	}
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
	if (connIndex != INVALID_CONNECTION_INDEX && (connIndex >= m_connections.size() || m_connections[connIndex] == nullptr))
	{
		return false;
	}

	// Peek at the header and verify the messages
	uint8_t messageCount = header.unreliableMessageCount;

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

	// Update the connection's acknowledgment data if there is one
	if (header.senderConnectionIndex != INVALID_CONNECTION_INDEX)
	{
		m_connections[header.senderConnectionIndex]->OnPacketReceived(header);
	}

	// Process the messages
	uint8_t messageCount = header.unreliableMessageCount;

	for (int i = 0; i < messageCount; ++i)
	{
		NetMessage message;
		packet->ReadMessage(&message);

		uint8_t defIndex = message.GetDefinitionIndex();
		const NetMessageDefinition_t* definition = m_messageDefinitions[defIndex];

		message.SetDefinitionIndex(defIndex);

		NetSender_t sender;
		sender.address = senderAddress;
		sender.netSession = this;
		sender.connectionIndex = header.senderConnectionIndex;

		// Call the callback
		definition->callback(&message, sender);
	}
}
