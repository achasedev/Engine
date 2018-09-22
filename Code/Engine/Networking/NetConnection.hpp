/************************************************************************/
/* File: NetConnection.hpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Class to represent a single connection in a NetSession
/************************************************************************/
#pragma once
#include "Engine/Networking/NetAddress.hpp"
#include <vector>

class UDPSocket;
class BytePacker;
class NetSession;
class NetMessage;

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


private:
	//-----Private Data-----

	std::vector<NetMessage*>	m_outboundUnreliables;
	NetAddress_t				m_address;

	NetSession*					m_owningSession = nullptr;
	uint8_t						m_indexInSession;

};
