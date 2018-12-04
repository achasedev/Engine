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
#include "Engine/Networking/NetObjectSystem.hpp"

#define RELIABLE_RESEND_INTERVAL (0.1) // 100 ms
#define RELIABLE_WINDOW (32)


//- C FUNCTION ----------------------------------------------------------------------------------------------
// Function that returns true if the value is less than the second value within the reliable window
//
bool CycleLessThan(uint16_t first, uint16_t second)
{
	uint16_t distance = second - first;
	return (distance & 0x8000) == 0;
}


//-----------------------------------------------------------------------------------------------
// Constructor
//
NetConnection::NetConnection(NetSession* session, const NetConnectionInfo_t& connectionInfo)
	: m_connectionInfo(connectionInfo)
	, m_owningSession(session)
{
	if (m_owningSession != nullptr)
	{
		m_heartbeatTimer.SetInterval(m_owningSession->GetHeartbeatInterval());
	}
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
NetConnection::~NetConnection()
{
	for (int i = 0; i < (int)m_outboundUnreliables.size(); ++i)
	{
		delete m_outboundUnreliables[i];
	}

	m_outboundUnreliables.clear();

	for (int i = 0; i < (int)m_unconfirmedReliables.size(); ++i)
	{
		delete m_unconfirmedReliables[i];
	}

	m_unconfirmedReliables.clear();

	for (int i = 0; i < m_unsentReliables.size(); ++i)
	{
		delete m_unsentReliables[i];
	}

	m_unsentReliables.clear();

	for (int i = 0; i < MAX_SEQUENCE_CHANNELS; ++i)
	{
		m_sequenceChannels[i].ClearOutOfOrderMessages();
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the state of this connection to the state specified
//
void NetConnection::SetConnectionState(eConnectionState state)
{
	m_state = state;
}


//-----------------------------------------------------------------------------------------------
// Queues the message to be sent during a flush
//
void NetConnection::Send(NetMessage* msg)
{
	if (msg->IsReliable())
	{
		if (msg->IsInOrder())
		{
			uint8_t channelIndex = msg->GetSequenceChannelID();

			uint16_t sequenceID = m_sequenceChannels[channelIndex].GetAndIncrementNextIDToSend();
			msg->AssignSequenceID(sequenceID);
		}

		m_unsentReliables.push_back(msg);
	}
	else
	{
		m_outboundUnreliables.push_back(msg);
	}
}


//- C FUNCTION ----------------------------------------------------------------------------------------------
// Returns true if the reliable message should be sent again
//
bool IsReliableReadyForResend(NetMessage* message)
{
	ASSERT_OR_DIE(message->IsReliable(), "Error: Unreliable message was checked for resend");

	float totalTime = Clock::GetMasterClock()->GetTotalSeconds();

	return (totalTime - message->GetLastSentTime() >= RELIABLE_RESEND_INTERVAL);
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
	packet->SetReceiverConnectionIndex(m_connectionInfo.sessionIndex);

	uint8_t messagesWritten = 0;
	PacketTracker_t* tracker = CreateTrackerForAck(m_nextAckToSend);

	// Write unconfirmed messages first
	int unconfirmedCount = (int) m_unconfirmedReliables.size();
	
	for (int unconfirmedIndex = 0; unconfirmedIndex < unconfirmedCount; ++unconfirmedIndex)
	{
		if (IsReliableReadyForResend(m_unconfirmedReliables[unconfirmedIndex]))
		{
			bool success = packet->WriteMessage(m_unconfirmedReliables[unconfirmedIndex]);

			if (success)
			{
				tracker->AddReliableID(m_unconfirmedReliables[unconfirmedIndex]->GetReliableID());
				m_unconfirmedReliables[unconfirmedIndex]->ResetTimeLastSent();
				++messagesWritten;
			}
		}
	}

	// Write unsent messages next, but not if we have too many unconfirmed reliables
	for (int unsentIndex = 0; unsentIndex < (int)m_unsentReliables.size(); ++unsentIndex)
	{
		NetMessage* unsentMessage = m_unsentReliables[unsentIndex];

		if (packet->CanFitMessage(unsentMessage) && NextSendIsWithinReliableWindow())
		{
			unsentMessage->AssignReliableID(m_nextReliableIDToSend);
			++m_nextReliableIDToSend;

			if (packet->WriteMessage(unsentMessage))
			{
				tracker->AddReliableID(unsentMessage->GetReliableID());

				// Update the lists
				m_unsentReliables.erase(m_unsentReliables.begin());
				m_unconfirmedReliables.push_back(unsentMessage);
				--unsentIndex;

				unsentMessage->ResetTimeLastSent();
				++messagesWritten;
			}
		}
	}

	// Write unreliables last
	for (int msgIndex = 0; msgIndex < m_outboundUnreliables.size(); ++msgIndex)
	{
		NetMessage* msg = m_outboundUnreliables[msgIndex];

		// Check if the message will fit
		if (packet->CanFitMessage(msg))
		{
			packet->WriteMessage(msg);
			messagesWritten++;
		}

		delete m_outboundUnreliables[msgIndex];
	}
	
	// If we have any snapshots to send and the room, send them out
	bool done = false;
	NetObjectSystem* netObjSystem = m_owningSession->GetNetObjectSystem();

	while (m_owningSession != nullptr && !done)
	{
		NetMessage snapshotMessage = NetMessage("netobj_update", m_owningSession);
		bool hasUpdate = netObjSystem->GetNextSnapshotUpdateMessage(&snapshotMessage, m_connectionInfo.sessionIndex);

		if (hasUpdate && packet->CanFitMessage(&snapshotMessage))
		{
			packet->WriteMessage(&snapshotMessage);
			messagesWritten++;
		}
		else
		{
			done = true;
		}
	}

	PacketHeader_t header = CreateHeaderForNextSend(messagesWritten);
	packet->WriteHeader(header);

	// Update the latest ack sent for the connection
	OnPacketSend(header);

	m_owningSession->SendPacket(packet);
		
	// Clear the unreliable list, even if not all were sent
	m_outboundUnreliables.clear();

	// Reset the send timer
	m_sendTimer.Reset();
	m_forceSendNextTick = false;
}


//-----------------------------------------------------------------------------------------------
// Returns the name (ID) of the user this connection points to
//
std::string NetConnection::GetName() const
{
	return m_connectionInfo.name;
}


//-----------------------------------------------------------------------------------------------
// Updates the connection's name (ID)
//
void NetConnection::UpdateName(const std::string& name)
{
	m_connectionInfo.name = name;
}


//-----------------------------------------------------------------------------------------------
// Returns the target address for this connection
//
NetAddress_t NetConnection::GetAddress() const
{
	return m_connectionInfo.address;
}


uint8_t NetConnection::GetSessionIndex() const
{
	return m_connectionInfo.sessionIndex;
}


//-----------------------------------------------------------------------------------------------
// Returns the average rtt of this connection
//
float NetConnection::GetRTT() const
{
	return m_rtt;
}


//-----------------------------------------------------------------------------------------------
// Sets the session index of the connection
//
void NetConnection::SetSessionIndex(uint8_t index)
{
	m_connectionInfo.sessionIndex = index;
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

	return (m_sendTimer.GetElapsedTime() >= sendInterval);
}


//-----------------------------------------------------------------------------------------------
// Returns true if the connection should send a heartbeat
//
bool NetConnection::HasHeartbeatElapsed()
{
	bool elapsed = m_heartbeatTimer.HasIntervalElapsed();

	if (elapsed)
	{
		m_heartbeatTimer.SetInterval(m_owningSession->GetHeartbeatInterval());
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
	header.totalMessageCount = messageCount;

	if (messageCount > 0)
	{
		header.packetAck = m_nextAckToSend;
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
	if (header.totalMessageCount == 0 || header.packetAck == INVALID_PACKET_ACK)
	{
		return;
	}

	// Track the packet
	PacketTracker_t* tracker = GetTrackerForAck(header.packetAck);
	tracker->timeSent = Clock::GetMasterClock()->GetTotalSeconds();

	m_packetsSent++;
	if (m_packetsSent >= LOSS_WINDOW_COUNT)
	{
		UpdateLossCalculation();
	}

	// Increment the next ack to send
	++m_nextAckToSend;

	if (m_nextAckToSend == INVALID_PACKET_ACK)
	{
		++m_nextAckToSend;
	}

	// Reset the send timer
	m_lastSentTimer.Reset();
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
	m_lastReceivedTimer.Reset();

	return true;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the connection has any pending outbound messages
//
bool NetConnection::HasOutboundMessages() const
{
	return (m_unsentReliables.size() > 0 || m_unconfirmedReliables.size() > 0 || m_outboundUnreliables.size() > 0);
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
	std::string debugText = Stringf("   %-*i%-*s%-*s%-*.2f%-*.2f%-*.2f%-*.2f%-*i%-*i%-*s",
		6, m_connectionInfo.sessionIndex, 10, m_connectionInfo.name.c_str(), 21, m_connectionInfo.address.ToString().c_str(), 8, 1000.f * m_rtt, 7, m_loss, 7, m_lastReceivedTimer.GetElapsedTime(), 7, m_lastSentTimer.GetElapsedTime(), 8, m_nextAckToSend - 1, 8, m_highestReceivedAck, 10, GetStateAsString().c_str());

	return debugText;
}


//-----------------------------------------------------------------------------------------------
// Called when an ACK is received off of a packet
//
void NetConnection::OnAckConfirmed(uint16_t ack)
{
	PacketTracker_t* tracker = GetTrackerForAck(ack);

	if (tracker == nullptr)
	{
		return;
	}

	// Calculate RTT
	float currentTime = Clock::GetMasterClock()->GetTotalSeconds();

	float timeDilation = (currentTime - tracker->timeSent);

	// Blend in this RTT to our existing RTT
	bool shouldUpdate = true;
	for (int i = 1; i < m_nextAckToSend - ack; ++i)
	{
		int checkIndex = ack + i;
		if (m_packetTrackers[checkIndex].packetAck == INVALID_PACKET_ACK)
		{
			shouldUpdate = false;
			break;
		}
	}

	if (shouldUpdate)
	{
		m_rtt = (1.f - RTT_BLEND_FACTOR) * m_rtt + RTT_BLEND_FACTOR * timeDilation;
	}
	
	// Remove reliable messages that have been confirmed
	for (int reliableIndex = (int)tracker->m_reliablesInPacket - 1; reliableIndex >= 0 ; --reliableIndex)
	{
		uint16_t currID = tracker->m_sentReliableIDs[reliableIndex];

		for (int unconfirmedIndex = 0; unconfirmedIndex < (int)m_unconfirmedReliables.size(); ++unconfirmedIndex)
		{
			if (m_unconfirmedReliables[unconfirmedIndex]->GetReliableID() == currID)
			{
				delete m_unconfirmedReliables[unconfirmedIndex];
				m_unconfirmedReliables.erase(m_unconfirmedReliables.begin() + unconfirmedIndex);
				break;
			}
		}
	}

	// It has been received, so invalidate
	InvalidateTracker(ack);
}


//-----------------------------------------------------------------------------------------------
// Creates and returns a tracker for the given ack
//
PacketTracker_t* NetConnection::CreateTrackerForAck(uint16_t ack)
{
	// If the ack is invalid do nothing
	if (ack == INVALID_PACKET_ACK)
	{
		return nullptr;
	}

	// Check if this has to be acknowledged at all (already ack'd, or just bad ack)
	int index = (ack % MAX_UNACKED_HISTORY);

	// Check to update packet loss (overwrite a packet never ack'd)
	if (m_packetTrackers[index].packetAck != INVALID_PACKET_ACK)
	{
		m_lossCount++;
	}

	PacketTracker_t* tracker = &m_packetTrackers[index];
	tracker->Clear();
	tracker->packetAck = ack;

	return tracker;
}


//-----------------------------------------------------------------------------------------------
// Returns the packet tracker corresponding to the given ack
//
PacketTracker_t* NetConnection::GetTrackerForAck(uint16_t ack)
{
	// If the ack is invalid do nothing
	if (ack == INVALID_PACKET_ACK)
	{
		return nullptr;
	}

	// Check if this has to be acknowledged at all (already ack'd, or just bad ack)
	int index = (ack % MAX_UNACKED_HISTORY);
	if (m_packetTrackers[index].packetAck == INVALID_PACKET_ACK || m_packetTrackers[index].packetAck != ack)
	{
		return nullptr;
	}

	return &m_packetTrackers[index];
}


//-----------------------------------------------------------------------------------------------
// Invalidates the tracker corresponding to the given ack
//
void NetConnection::InvalidateTracker(uint16_t ack)
{
	int index = (ack % MAX_UNACKED_HISTORY);
	
	m_packetTrackers[index].packetAck = INVALID_PACKET_ACK;
}


//-----------------------------------------------------------------------------------------------
// Returns the oldest unconfirmed reliable id that we sent
//
bool NetConnection::NextSendIsWithinReliableWindow() const
{
	// Get the oldest unconfirmed id
	int unconfirmedCount = (int)m_unconfirmedReliables.size();
	uint16_t oldestID = 0;
	bool found = false;
	for (int i = 0; i < unconfirmedCount; ++i)
	{
		if (!found || CycleLessThan(m_unconfirmedReliables[i]->GetReliableID(), oldestID))
		{
			oldestID = m_unconfirmedReliables[i]->GetReliableID();
			found = true;
		}
	}

	if (!found)
	{
		return true;
	}

	// Ensure our next send would fit within the window
	uint16_t maxIDCanSend = oldestID + RELIABLE_WINDOW;
	return CycleLessThan(m_nextReliableIDToSend, maxIDCanSend);
}


//-----------------------------------------------------------------------------------------------
// Returns whether the reliable ID has already been processed (recently) by the connection
//
bool NetConnection::HasReliableIDAlreadyBeenReceived(uint16_t reliableID) const
{
	int receivedCount = (int)m_receivedReliableIDs.size();

	// First check if it's outside the window
	uint16_t minID = m_highestReceivedReliableID - RELIABLE_WINDOW + 1;
	if (CycleLessThan(reliableID, minID))
	{
		return true;
	}

	// Within window, just check if it is in our received list
	for (int receivedIndex = 0; receivedIndex < receivedCount; ++receivedIndex)
	{
		if (m_receivedReliableIDs[receivedIndex] == reliableID)
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Stores the reliable ID on this connection's list of processed IDs
//
void NetConnection::AddProcessedReliableID(uint16_t reliableID)
{
	if (CycleLessThan(m_highestReceivedReliableID, reliableID))
	{
		m_highestReceivedReliableID = reliableID;
	}

	m_receivedReliableIDs.push_back(reliableID);

	for (int i = 0; i < (int) m_receivedReliableIDs.size(); ++i)
	{
		uint16_t minID = m_highestReceivedReliableID - RELIABLE_WINDOW + 1;

		if (CycleLessThan(m_receivedReliableIDs[i], minID))
		{
			m_receivedReliableIDs.erase(m_receivedReliableIDs.begin() + i);
			--i;
		}
	}
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


//-----------------------------------------------------------------------------------------------
// Returns the state of the connection as a string
//
std::string NetConnection::GetStateAsString() const
{
	switch (m_state)
	{
	case CONNECTION_DISCONNECTED:
		return "DC'd";
		break;
	case CONNECTION_BOUND:
		return "BOUND";
		break;
	case CONNECTION_READY:
		return "READY";
		break;
	default:
		return "";
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the sequence channel at the current index
//
NetSequenceChannel* NetConnection::GetSequenceChannel(uint8_t sequenceChannelID)
{
	if (sequenceChannelID > MAX_SEQUENCE_CHANNELS)
	{
		return nullptr;
	}

	return &m_sequenceChannels[sequenceChannelID];
}


//-----------------------------------------------------------------------------------------------
// Returns whether the message is the next message to process within its channel
//
bool NetConnection::IsNextMessageInSequence(NetMessage* message)
{
	if (!message->IsInOrder())
	{
		return true;
	}

	NetSequenceChannel* channel = GetSequenceChannel(message->GetSequenceChannelID());

	ASSERT_OR_DIE(channel != nullptr, "Error: NetConnection::IsNextMessageInSequence() called on an InOrder message with no channel");
	return channel->IsMessageNextExpected(message->GetSequenceID());
}


//-----------------------------------------------------------------------------------------------
// Adds the given message to the appropriate channel to be processed in order later
//
void NetConnection::QueueInOrderMessage(NetMessage* message)
{
	if (!message->IsInOrder())
	{
		return;
	}

	NetSequenceChannel* channel = GetSequenceChannel(message->GetSequenceChannelID());
	if (channel != nullptr)
	{
		channel->AddOutOfOrderMessage(message);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the time since the last received packet on this connection
//
float NetConnection::GetTimeSinceLastReceive() const
{
	return m_lastReceivedTimer.GetElapsedTime();
}
