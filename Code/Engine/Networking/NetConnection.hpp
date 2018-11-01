/************************************************************************/
/* File: NetConnection.hpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Class to represent a single connection in a NetSession
/************************************************************************/
#pragma once
#include "Engine/Networking/NetPacket.hpp"
#include "Engine/Networking/NetAddress.hpp"
#include "Engine/Networking/NetSequenceChannel.hpp"
#include <vector>

class UDPSocket;
class BytePacker;
class NetSession;
class NetMessage;
class Stopwatch;

#define MAX_UNACKED_HISTORY (256)
#define MAX_RELIABLES_PER_PACKET (32)
#define RELIABLE_WINDOW (32)
#define MAX_SEQUENCE_CHANNELS (32)

struct PacketTracker_t
{
	bool AddReliableID(uint16_t reliableID)
	{
		if (m_reliablesInPacket == MAX_RELIABLES_PER_PACKET)
		{
			return false;
		}

		m_sentReliableIDs[m_reliablesInPacket] = reliableID;
		++m_reliablesInPacket;

		return true;
	}

	void Clear()
	{
		packetAck = INVALID_PACKET_ACK;
		timeSent = -1.0f;
		m_reliablesInPacket = 0;
	}

	uint16_t	packetAck = INVALID_PACKET_ACK;
	float		timeSent = -1.f;

	uint16_t m_sentReliableIDs[MAX_RELIABLES_PER_PACKET];
	unsigned int m_reliablesInPacket = 0;
};

class NetConnection
{
public:
	//-----Public Methods-----

	NetConnection(NetAddress_t& address, NetSession* session, uint8_t connectionIndex);
	~NetConnection();


	// Queues the message to be processed later
	void						Send(NetMessage* msg);
	void						FlushMessages();


	// Returns the target address this connection is associated with
	NetAddress_t				GetAddress();

	// Network tick
	void						SetNetTickRate(float hertz);
	bool						HasNetTickElapsed() const;

	// Heartbeat
	bool						HasHeartbeatElapsed() const;

	// Reliable delivery
	bool						OnPacketReceived(const PacketHeader_t& header);
	bool						HasReliableIDAlreadyBeenReceived(uint16_t reliableID) const;
	void						AddProcessedReliableID(uint16_t reliableID);

	// RTT/Loss
	bool						HasOutboundMessages() const;
	bool						NeedsToForceSend() const;

	// In order traffic
	NetSequenceChannel*			GetSequenceChannel(uint8_t sequenceChannelID);
	bool						IsNextMessageInSequence(NetMessage* message);
	void						QueueInOrderMessage(NetMessage* message);

	// For drawing
	std::string					GetDebugInfo() const;


private:
	//-----Private Methods-----

	// For reliable delivery
	PacketHeader_t				CreateHeaderForNextSend(uint8_t messageCount);
	void						OnPacketSend(const PacketHeader_t& header);
	void						OnAckConfirmed(uint16_t ack);

	PacketTracker_t*			CreateTrackerForAck(uint16_t ack);
	PacketTracker_t*			GetTrackerForAck(uint16_t ack);
	void						InvalidateTracker(uint16_t ack);
	bool						NextSendIsWithinReliableWindow() const;

	// RTT/Loss
	void						UpdateLossCalculation();


private:
	//-----Private Data-----

	std::vector<NetMessage*>	m_outboundUnreliables;
	std::vector<NetMessage*>	m_unsentReliables;
	std::vector<NetMessage*>	m_unconfirmedReliables;

	std::vector<uint16_t>		m_receivedReliableIDs;

	NetAddress_t				m_address;

	NetSession*					m_owningSession = nullptr;
	uint8_t						m_indexInSession;

	// For net tick
	float						m_timeBetweenSends = 0.f;
	Stopwatch*					m_sendTimer = nullptr;

	Stopwatch*					m_heartbeatTimer = nullptr;

	// Reliable delivery
	uint16_t m_nextAckToSend = 0;
	uint16_t m_highestReceivedAck = INVALID_PACKET_ACK;
	uint16_t m_receivedBitfield = 0;

	uint16_t m_nextReliableIDToSend = 0;
	uint16_t m_highestReceivedReliableID = 0xffff;

	NetSequenceChannel m_sequenceChannels[MAX_SEQUENCE_CHANNELS];

	PacketTracker_t m_packetTrackers[MAX_UNACKED_HISTORY];

	Stopwatch* m_lastSentTimer = nullptr;
	Stopwatch* m_lastReceivedTimer = nullptr;

	int m_packetsSent = 0;
	int m_lossCount = 0;

	float m_loss = 0.f;
	float m_rtt = 0.f;

	bool m_forceSendNextTick = false;

	static constexpr float RTT_BLEND_FACTOR = 0.01f;
	static constexpr unsigned int LOSS_WINDOW_COUNT = 50;

};
