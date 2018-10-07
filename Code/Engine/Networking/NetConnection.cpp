/************************************************************************/
/* File: NetConnection.cpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Implementation of the NetConnection class
/************************************************************************/
#include "Engine/Core/LogSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Networking/UDPSocket.hpp"
#include "Engine/Networking/NetPacket.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetSession.hpp"
#include "Engine/Networking/NetConnection.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
NetConnection::NetConnection(NetAddress_t& address, NetSession* session, uint8_t connectionIndex)
	: m_address(address)
	, m_owningSession(session)
	, m_indexInSession(connectionIndex)
{
	m_sendTimer = new Stopwatch();
	m_heartbeatTimer = new Stopwatch();
	m_heartbeatTimer->SetInterval(m_timeBetweenHeartbeats);
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

	if (m_sendTimer != nullptr)
	{
		delete m_sendTimer;
		m_sendTimer = nullptr;
	}

	if (m_heartbeatTimer != nullptr)
	{
		delete m_heartbeatTimer;
		m_heartbeatTimer = nullptr;
	}
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
	if (m_outboundUnreliables.size() == 0)
	{
		return;
	}

	// Package them all into one NetPacket
	NetPacket* packet = new NetPacket();
	packet->AdvanceWriteHead(sizeof(PacketHeader_t)); // Advance the write head now, and write the header later
	packet->SetSenderConnectionIndex(m_owningSession->GetLocalConnectionIndex());
	packet->SetReceiverConnectionIndex(m_indexInSession);

	uint8_t messagesWritten = 0;

	for (int msgIndex = 0; msgIndex < m_outboundUnreliables.size(); ++msgIndex)
	{
		NetMessage* msg = m_outboundUnreliables[msgIndex];

		// Check if the message will fit
		// 2 bytes for the header and message size, 1 byte for message definition index,
		// and then the payload
		uint16_t totalSize = (uint16_t) (2 + 1 + msg->GetWrittenByteCount());
		if (packet->GetRemainingWritableByteCount() >= totalSize)
		{
			packet->WriteMessage(msg);
			messagesWritten++;
		}
		else
		{
			PacketHeader_t header = CreateHeaderForNextSend(messagesWritten);
			packet->WriteHeader(header);

			OnPacketSend(packet);

			// Send the current packet and start again
			bool sent = m_owningSession->SendPacket(packet);

			if (sent)
			{
				LogTaggedPrintf("NET", "NetConnection sent packet with %i messages", messagesWritten);
			}
			else
			{
				LogTaggedPrintf("NET", "NetConnection couldn't send packet for %i messages", messagesWritten);
			}

			// Reset
			packet->ResetWrite();
			packet->AdvanceWriteHead(sizeof(PacketHeader_t));

			packet->WriteMessage(msg);
			messagesWritten = 1;
		}

		delete msg;
	}
	
	PacketHeader_t header = CreateHeaderForNextSend(messagesWritten);
	packet->WriteHeader(header);

	OnPacketSend(packet);

	bool sent = m_owningSession->SendPacket(packet);

	if (sent)
	{
		LogTaggedPrintf("NET", "NetConnection sent packet with %i messages", messagesWritten);
	}
	else
	{
		LogTaggedPrintf("NET", "NetConnection couldn't send packet for %i messages", messagesWritten);
	}
		

	m_outboundUnreliables.clear();

	// Reset the send timer
	m_sendTimer->Reset();
}


//-----------------------------------------------------------------------------------------------
// Returns the target address for this connection
//
NetAddress_t NetConnection::GetAddress()
{
	return m_address;
}


//-----------------------------------------------------------------------------------------------
// Sets the net tick rate for the connection to correspond to the provided refresh rate
//
void NetConnection::SetNetTickRate(float hertz)
{
	m_timeBetweenSends = (1.f / hertz);
}


//-----------------------------------------------------------------------------------------------
// Returns whether the connection should send based on the tick rate of the connection and the owning
// session
//
bool NetConnection::IsReadyToFlush() const
{
	float sessionTime = m_owningSession->GetTimeBetweenSends();
	float sendInterval = MaxFloat(sessionTime, m_timeBetweenSends);

	return (m_sendTimer->GetElapsedTime() > sendInterval);
}


//-----------------------------------------------------------------------------------------------
// Sets the heartbeat of the connection to correspond to the given frequency
//
void NetConnection::SetHeartbeat(float hertz)
{
	m_timeBetweenHeartbeats = (1.0f / hertz);
	m_heartbeatTimer->SetInterval(m_timeBetweenHeartbeats);
}


//-----------------------------------------------------------------------------------------------
// Returns true if the connection should send a heartbeat
//
bool NetConnection::HasHeartbeatElapsed() const
{
	bool elapsed = m_heartbeatTimer->HasIntervalElapsed();

	if (elapsed)
	{
		m_heartbeatTimer->SetInterval(m_timeBetweenHeartbeats);
	}

	return elapsed;
}


//-----------------------------------------------------------------------------------------------
// Creates the packet header for the next packet to be sent
//
PacketHeader_t NetConnection::CreateHeaderForNextSend(uint8_t messageCount)
{
	PacketHeader_t header;
	header.senderConnectionIndex = m_owningSession->GetLocalConnectionIndex();
	header.unreliableMessageCount = messageCount;
	header.packetAck = m_nextSentAck;

	return header;
}


//-----------------------------------------------------------------------------------------------
// Called when a packet is being sent
//
void NetConnection::OnPacketSend(NetPacket* packet)
{
	++m_nextSentAck;

	if (m_nextSentAck == INVALID_PACKET_ACK)
	{
		++m_nextSentAck;
	}

	// Track the packet ack here
	UNUSED(packet);
}


//-----------------------------------------------------------------------------------------------
// Called when a packet is received associated with this connection
// Returns true if a new ack was received, false otherwise
//
bool NetConnection::UpdateAckData(const PacketHeader_t& header)
{
	// Call on the highest received ack
	OnAckReceived(header.highestReceivedAck);

	// Call on all acks in the history
	for (int i = 0; i < 16; ++i)
	{
		uint16_t bitFlag = 1 << i;
		if (header.receivedHistory & bitFlag)
		{
			OnAckReceived((uint16_t)(header.highestReceivedAck - (i + 1)));
		}
	}

	// Now update the highest packet I have received from my connection
	uint16_t receivedAck = header.packetAck;
	uint16_t distance = receivedAck - m_highestReceivedAck;

	if (distance == 0)
	{
		// Duplicate ack, do nothing
		return false;
	}

	if ((distance & 0x8000) == 0)
	{
		m_highestReceivedAck = receivedAck;

		// Left shift the distance
		m_receivedBitfield = m_receivedBitfield << distance;
		m_receivedBitfield |= (1 << (distance - 1)); // Set the history of the highest order bit
	}
	else
	{
		// Else got a packet older than the highest received
		distance = m_highestReceivedAck - receivedAck;

		// Check if the bit is already set = if so duplicate ack
		uint16_t mask = (1 << (distance - 1));
		
		if ((m_receivedBitfield & mask) == mask)
		{
			// Already acknowledged this packet, so do nothing
			return false;
		}

		// Acknowledge the new packet
		m_receivedBitfield |= mask;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
// Called when an ACK is received off of a packet
//
void NetConnection::OnAckReceived(uint16_t ack)
{
	// Here we'd update the array to do something
	UNUSED(ack);
}
