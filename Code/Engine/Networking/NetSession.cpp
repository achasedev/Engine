/************************************************************************/
/* File: NetSession.cpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Implementation of the NetSession class
/************************************************************************/
#include "Engine/Networking/UDPSocket.hpp"
#include "Engine/Networking/NetPacket.hpp"
#include "Engine/Networking/NetSession.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetConnection.hpp"
#include "Engine/Core/LogSystem.hpp"


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
bool NetSession::Bind(unsigned short port)
{
	UDPSocket* newSocket = new UDPSocket();

	NetAddress_t localAddress;
	bool foundLocal = NetAddress_t::GetLocalAddress(&localAddress, port, true);

	if (!foundLocal)
	{
		ConsoleErrorf("Couldn't find local address to bind to");
		return false;
	}

	bool bound = newSocket->Bind(localAddress);

	if (bound)
	{
		m_boundSocket = newSocket;
		LogTaggedPrintf("NET", "NetSession bound to address %s", localAddress.ToString().c_str());
	}
	else
	{
		delete newSocket;
		LogTaggedPrintf("NET", "Error: NetSession::Bind() couldn't bind to address %s", localAddress.ToString().c_str());
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
	NetConnection* connection = m_connections[packet->GetConnectionIndex()];
	NetAddress_t address = connection->GetAddress();

	size_t amountSent = m_boundSocket->SendTo(address, packet->GetBuffer(), packet->GetWrittenByteCount());

	return amountSent > 0;
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
// Create and add a new connection
//
bool NetSession::AddConnection(unsigned int bindingIndex, NetAddress_t address)
{
	// Keep it in range of the vector
	m_connections.resize(bindingIndex + 1);

	if (m_connections[bindingIndex] != nullptr)
	{
		LogTaggedPrintf("NET", "Warning: NetSession::AddConnection() tried to add a connection to an already existing connection index, index was %i", bindingIndex);
		return false;
	}

	NetConnection* newConnection = new NetConnection(address, this, bindingIndex);
	m_connections[bindingIndex] = newConnection;

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
// Receives on all current connections, calling callbacks on all receiving messages depending on their tag
//
void NetSession::ProcessIncoming()
{
	bool done = false;
	while (!done)
	{
		NetPacket packet;
		NetAddress_t senderAddress;

		int amountReceived = m_boundSocket->ReceiveFrom(&senderAddress, packet.GetWriteHead(), PACKET_MTU);

		if (amountReceived > 0)
		{
			ProcessReceivedPacket(&packet);
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
		m_connections[index]->FlushMessages();
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
// Processes the received packet by calling all message callbacks for messages within the packet
//
void NetSession::ProcessReceivedPacket(NetPacket* packet)
{
	// Get the messages out
	PacketHeader_t header;
	packet->ReadHeader(header);
	packet->m_sendReceiveIndex = header.senderConnectionIndex;

	NetConnection* senderConnection = m_connections[header.senderConnectionIndex];
	uint8_t messageCount = header.unreliableMessageCount;

	for (int i = 0; i < messageCount; ++i)
	{
		NetMessage message;
		packet->ReadMessage(&message);

		uint8_t defIndex = message.GetDefinitionIndex();
		const NetMessageDefinition_t* definition = m_messageDefinitions[defIndex];

		message.SetDefinition(defIndex, definition);

		// Call the callback
		definition->callback(&message, senderConnection);
	}
}
