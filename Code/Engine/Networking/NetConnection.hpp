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
	bool						IsReadyToFlush() const;

	// Heartbeat
	void						SetHeartbeat(float hertz);
	bool						HasHeartbeatElapsed() const;

	// Reliable delivery
	bool						UpdateAckData(const PacketHeader_t& header);


private:
	//-----Private Methods-----

	// For reliable delivery
	PacketHeader_t				CreateHeaderForNextSend(uint8_t messageCount);
	void						OnPacketSend(NetPacket* packet);
	void						OnAckReceived(uint16_t ack);


private:
	//-----Private Data-----

	std::vector<NetMessage*>	m_outboundUnreliables;
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
	uint16_t m_highestReceivedAck;
	uint16_t m_receivedBitfield = 0;

	NetPacket* m_sendButUnackedPackets[MAX_UNACKED_HISTORY];

	float m_lastSendTime = 0.f;
	float m_lastReceiveTime = 0.f;

	float m_loss = 0.f;
	float m_rtt = 0.f;

};
