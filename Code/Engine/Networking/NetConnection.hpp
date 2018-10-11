/************************************************************************/
/* File: NetConnection.hpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Class to represent a single connection in a NetSession
/************************************************************************/
#pragma once
#include "Engine/Networking/NetAddress.hpp"
#include "Engine/Networking/NetPacket.hpp"
#include <vector>

class UDPSocket;
class BytePacker;
class NetSession;
class NetMessage;
class Stopwatch;

#define MAX_UNACKED_HISTORY (256)

struct PacketTracker_t
{
	uint16_t	packetAck = INVALID_PACKET_ACK;
	float		timeSent = -1.f;
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
	void						SetHeartbeat(float hertz);
	bool						HasHeartbeatElapsed() const;

	// Reliable delivery
	bool						OnPacketReceived(const PacketHeader_t& header);

	// RTT/Loss
	bool						HasOutboundMessages() const;
	bool						NeedsToForceSend() const;

	// For drawing
	std::string					GetDebugInfo() const;


private:
	//-----Private Methods-----

	// For reliable delivery
	PacketHeader_t				CreateHeaderForNextSend(uint8_t messageCount);
	void						OnPacketSend(const PacketHeader_t& header);
	void						OnAckReceived(uint16_t ack);

	// RTT/Loss
	void						UpdateLossCalculation();


private:
	//-----Private Data-----

	std::vector<NetMessage*>	m_outboundMessages;
	NetAddress_t				m_address;

	NetSession*					m_owningSession = nullptr;
	uint8_t						m_indexInSession;

	// For net tick
	float						m_timeBetweenSends = 0.f;
	Stopwatch*					m_sendTimer = nullptr;

	float						m_timeBetweenHeartbeats = 0.5f;
	Stopwatch*					m_heartbeatTimer = nullptr;

	// Reliable delivery
	uint16_t m_nextSentAck = 0;
	uint16_t m_highestReceivedAck = 0;
	uint16_t m_receivedBitfield = 0;

	PacketTracker_t m_sentButUnackedPackets[MAX_UNACKED_HISTORY];

	Stopwatch* m_lastSentTimer = nullptr;
	Stopwatch* m_lastReceivedTimer = nullptr;

	int m_packetsSent = 0;
	int m_lossCount = 0;

	float m_loss = 0.f;
	float m_rtt = 0.f;

	bool m_forceSendNextTick = false;

	static constexpr float RTT_BLEND_FACTOR = 0.1f;
	static constexpr unsigned int LOSS_WINDOW_COUNT = 50;

};
