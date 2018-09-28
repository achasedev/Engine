/************************************************************************/
/* File: NetSession.cpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Implementation of the NetSession class
/************************************************************************/
#include "Engine/Core/LogSystem.hpp"
#include "Engine/Networking/UDPSocket.hpp"
#include "Engine/Networking/NetPacket.hpp"
#include "Engine/Networking/NetSession.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetConnection.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
NetSession::NetSession()
{
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
		//ConsolePrintf(Rgba::GREEN, "Net Session bound to address %s", newSocket->GetNetAddress().ToString().c_str());
		LogTaggedPrintf("NET", "NetSession bound to address %s", newSocket->GetNetAddress().ToString().c_str());
	}
	else
	{
		delete newSocket;
		//ConsoleErrorf("Net Session could not bind to address %s", newSocket->GetNetAddress().ToString().c_str());
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
	m_connections.resize(connectionIndex + 1);

	if (m_connections[connectionIndex] != nullptr)
	{
		LogTaggedPrintf("NET", "Warning: NetSession::AddConnection() tried to add a connection to an already existing connection index, index was %i", connectionIndex);
		return false;
	}

	NetConnection* newConnection = new NetConnection(address, this, connectionIndex);
	m_connections[connectionIndex] = newConnection;

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
		NetPacket packet;
		NetAddress_t senderAddress;

		size_t amountReceived = m_boundSocket->ReceiveFrom(&senderAddress, packet.GetWriteHead(), PACKET_MTU);

		if (amountReceived > 0)
		{
			packet.AdvanceWriteHead(amountReceived);

			if (VerifyPacket(&packet))
			{
				ProcessReceivedPacket(&packet, senderAddress);
			}
			else
			{
				LogTaggedPrintf("NET", "Received a bad packet from address %s, message was %i bytes", senderAddress.ToString().c_str(), amountReceived);
			}
		}
		else
		{
			done = true;
		}
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
		if (m_connections[index] != nullptr)
		{
			m_connections[index]->FlushMessages();
		}
	}
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
