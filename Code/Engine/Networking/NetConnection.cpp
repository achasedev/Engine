/************************************************************************/
/* File: NetConnection.cpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Implementation of the NetConnection class
/************************************************************************/
#include "Engine/Core/LogSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/Clock.hpp"
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
	m_lastSentTimer = new Stopwatch();
	m_lastReceivedTimer = new Stopwatch();

	m_heartbeatTimer->SetInterval(m_owningSession->GetHeartbeatInterval());
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
NetConnection::~NetConnection()
{
	// Check for any pending messages that didn't get sent
	for (int i = 0; i < (int)m_outboundMessages.size(); ++i)
	{
		delete m_outboundMessages[i];
	}

	m_outboundMessages.clear();

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

	if (m_lastSentTimer != nullptr)
	{
		delete m_lastSentTimer;
		m_lastSentTimer = nullptr;
	}

	if (m_lastReceivedTimer != nullptr)
	{
		delete m_lastReceivedTimer;
		m_lastReceivedTimer = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Queues the message to be sent during a flush
//
void NetConnection::Send(NetMessage* msg)
{
	m_outboundMessages.push_back(msg);
}


//-----------------------------------------------------------------------------------------------
// Sends all pending messages out of the socket
//
void NetConnection::FlushMessages()
{
	// Package them all into one NetPacket
	NetPacket* packet = new NetPacket();
	packet->AdvanceWriteHead(PACKET_HEADER_SIZE); // Advance the write head now, and write the header later
	packet->SetSenderConnectionIndex(m_owningSession->GetLocalConnectionIndex());
	packet->SetReceiverConnectionIndex(m_indexInSession);

	uint8_t messagesWritten = 0;

	for (int msgIndex = 0; msgIndex < m_outboundMessages.size(); ++msgIndex)
	{
		NetMessage* msg = m_outboundMessages[msgIndex];

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

			// Update the latest ack sent for the connection
			OnPacketSend(header);

			// Send the current packet and start again
			bool sent = m_owningSession->SendPacket(packet);

			if (sent)
			{
				ConsolePrintf(Rgba::GREEN, "Sent packet with ack %i", header.packetAck);
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

	// Update the latest ack sent for the connection
	OnPacketSend(header);
	

	bool sent = m_owningSession->SendPacket(packet);

	if (sent)
	{
		ConsolePrintf(Rgba::GREEN, "Sent packet with ack %i", header.packetAck);
		LogTaggedPrintf("NET", "NetConnection sent packet with %i messages", messagesWritten);
	}
	else
	{
		LogTaggedPrintf("NET", "NetConnection couldn't send packet for %i messages", messagesWritten);
	}
		

	m_outboundMessages.clear();

	// Reset the send timer
	m_sendTimer->Reset();
	m_forceSendNextTick = false;
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
bool NetConnection::HasNetTickElapsed() const
{
	float sessionTime = m_owningSession->GetTimeBetweenSends();
	float sendInterval = MaxFloat(sessionTime, m_timeBetweenSends);

	return (m_sendTimer->GetElapsedTime() >= sendInterval);
}


//-----------------------------------------------------------------------------------------------
// Returns true if the connection should send a heartbeat
//
bool NetConnection::HasHeartbeatElapsed() const
{
	bool elapsed = m_heartbeatTimer->HasIntervalElapsed();

	if (elapsed)
	{
		m_heartbeatTimer->SetInterval(m_owningSession->GetHeartbeatInterval());
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

	if (messageCount > 0)
	{
		header.packetAck = m_nextSentAck;
	}
	else
	{
		header.packetAck = INVALID_PACKET_ACK;
	}

	header.highestReceivedAck = m_highestReceivedAck;
	header.receivedHistory = m_receivedBitfield;

	return header;
}


//-----------------------------------------------------------------------------------------------
// Called when a packet is being sent
//
void NetConnection::OnPacketSend(const PacketHeader_t& header)
{
	// Don't do anything to a packet that has no messages (invalid)
	if (header.unreliableMessageCount == 0 || header.packetAck == INVALID_PACKET_ACK)
	{
		return;
	}

	// Track the packet
	PacketTracker_t tracker;
	tracker.packetAck = m_nextSentAck;
	tracker.timeSent = Clock::GetMasterClock()->GetTotalSeconds();

	uint16_t index = (m_nextSentAck % MAX_UNACKED_HISTORY);

	// Check to update packet loss (overwrite a packet never ack'd)
	if (m_sentButUnackedPackets[index].packetAck != INVALID_PACKET_ACK)
	{
		m_lossCount++;
	}

	m_packetsSent++;

	if (m_packetsSent >= LOSS_WINDOW_COUNT)
	{
		UpdateLossCalculation();
	}

	// Loss calculated, now update the tracker in the array
	m_sentButUnackedPackets[index] = tracker;

	// Increment the next ack to send
	++m_nextSentAck;

	if (m_nextSentAck == INVALID_PACKET_ACK)
	{
		++m_nextSentAck;
	}

	// Reset the send timer
	m_lastSentTimer->Reset();
}


//-----------------------------------------------------------------------------------------------
// Called when a packet is received associated with this connection
// Returns true if a new ack was received, false otherwise
//
bool NetConnection::OnPacketReceived(const PacketHeader_t& header)
{
	// Call on the highest received ack
	OnAckConfirmed(header.highestReceivedAck);

	// Call on all acks in the history
	for (int i = 0; i < 16; ++i)
	{
		uint16_t bitFlag = 1 << i;
		if (header.receivedHistory & bitFlag)
		{
			OnAckConfirmed((uint16_t)(header.highestReceivedAck - (i + 1)));
		}
	}

	// Now update the highest packet I have received from my connection, if it's valid
	if (header.packetAck != INVALID_PACKET_ACK)
	{
		uint16_t receivedAck = header.packetAck;
		uint16_t distance = receivedAck - m_highestReceivedAck;

		if (distance == 0)
		{
			// Duplicate ack, do nothing
			ERROR_AND_DIE("Received duplicate ack");
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

		// Force send next tick to maintain RTT
		m_forceSendNextTick = true;
	}
	

	// Reset the last received timer
	m_lastReceivedTimer->Reset();

	return true;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the connection has any pending outbound messages
//
bool NetConnection::HasOutboundMessages() const
{
	return (m_outboundMessages.size() > 0);
}


//-----------------------------------------------------------------------------------------------
// Returns true if the connection should send a packet next frame to maintain RTT
//
bool NetConnection::NeedsToForceSend() const
{
	return m_forceSendNextTick;
}


//-----------------------------------------------------------------------------------------------
// Returns a string representation of the connection's data and state
//
std::string NetConnection::GetDebugInfo() const
{
	std::string debugText = Stringf("   %-*i%-*s%-*.2f%-*.2f%-*.2f%-*.2f%-*i%-*i%-*s",
		6, m_indexInSession, 21, m_address.ToString().c_str(), 8, 1000.f * m_rtt, 7, m_loss, 7, m_lastReceivedTimer->GetElapsedTime(), 7, m_lastSentTimer->GetElapsedTime(), 8, m_nextSentAck - 1, 8, m_highestReceivedAck, 10, GetAsBitString(m_receivedBitfield).c_str());

	return debugText;
}


//-----------------------------------------------------------------------------------------------
// Called when an ACK is received off of a packet
//
void NetConnection::OnAckConfirmed(uint16_t ack)
{
	// If the ack is invalid do nothing
	if (ack == INVALID_PACKET_ACK)
	{
		return;
	}

	// Check if this has to be acknowledged at all (already ack'd, or just bad ack)
	int index = (ack % MAX_UNACKED_HISTORY);
	if (m_sentButUnackedPackets[index].packetAck == INVALID_PACKET_ACK || m_sentButUnackedPackets[index].packetAck != ack)
	{
		return;
	}

	// Calculate RTT
	float currentTime = Clock::GetMasterClock()->GetTotalSeconds();

	float timeDilation = (currentTime - m_sentButUnackedPackets[index].timeSent);

	// Blend in this RTT to our existing RTT
	bool shouldUpdate = true;
	for (int i = 1; i < m_nextSentAck - ack; ++i)
	{
		int checkIndex = ack + i;
		if (m_sentButUnackedPackets[checkIndex].packetAck == INVALID_PACKET_ACK)
		{
			shouldUpdate = false;
			break;
		}
	}

	if (shouldUpdate)
	{
		m_rtt = (1.f - RTT_BLEND_FACTOR) * m_rtt + RTT_BLEND_FACTOR * timeDilation;
		ConsolePrintf("Dilation: %.2f | RTT: %.2f", timeDilation, m_rtt);
	}

	// It has been received, so invalidate
	m_sentButUnackedPackets[index].packetAck = INVALID_PACKET_ACK;
}


//-----------------------------------------------------------------------------------------------
// Updates the calculated loss for this connection given the window
//
void NetConnection::UpdateLossCalculation()
{
	m_loss = (float)m_lossCount / (float)m_packetsSent;

	// Reset the current count for the next window
	m_packetsSent = 0;
	m_lossCount = 0;
}
