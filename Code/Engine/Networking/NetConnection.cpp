/************************************************************************/
/* File: NetConnection.cpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Implementation of the NetConnection class
/************************************************************************/
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetConnection.hpp"
#include "Engine/Networking/UDPSocket.hpp"
#include "Engine/Networking/NetPacket.hpp"
#include "Engine/Networking/NetSession.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
NetConnection::NetConnection(NetAddress_t& address, NetSession* session, uint8_t connectionIndex)
	: m_address(address)
	, m_owningSession(session)
	, m_indexInSession(connectionIndex)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
NetConnection::~NetConnection()
{
	// Check for any pending messages that didn't get sent
	for (int i = 0; i < (int)m_outboundUnreliables.size(); ++i)
	{
		delete m_outboundUnreliables[i];
	}

	m_outboundUnreliables.clear();
}


//-----------------------------------------------------------------------------------------------
// Queues the message to be sent during a flush
//
void NetConnection::Send(NetMessage* msg)
{
	m_outboundUnreliables.push_back(msg);
}


//-----------------------------------------------------------------------------------------------
// Sends all pending messages out of the socket
//
void NetConnection::FlushMessages()
{
	// Package them all into one NetPacket
	NetPacket* packet = new NetPacket();
	packet->AdvanceWriteHead(2); // Advance the write head now, and write the header later
	unsigned int messagesWritten = 0;

	for (int msgIndex = 0; msgIndex < m_outboundUnreliables.size(); ++msgIndex)
	{
		NetMessage* msg = m_outboundUnreliables[msgIndex];

		// Check if the message will fit
		// 2 bytes for the header and message size, 1 byte for message definition index,
		// and then the payload
		uint16_t totalSize = 2 + 1 + msg->GetWrittenByteCount();
		if (packet->GetRemainingWritableByteCount() >= totalSize)
		{
			packet->WriteMessage(msg);
			messagesWritten++;
		}
		else
		{
			PacketHeader_t header(m_indexInSession, messagesWritten);
			packet->WriteHeader(header);

			// Send the current packet and start again
			m_owningSession->SendPacket(packet);

			// Reset
			packet->ResetWrite();
			packet->AdvanceWriteHead(2);

			packet->WriteMessage(msg);
			messagesWritten = 1;
		}

		delete msg;
	}
	
	PacketHeader_t header(m_indexInSession, messagesWritten);
	packet->WriteHeader(header);

	m_owningSession->SendPacket(packet);

	m_outboundUnreliables.clear();
}


//-----------------------------------------------------------------------------------------------
// Returns the target address for this connection
//
NetAddress_t NetConnection::GetAddress()
{
	return m_address;
}
