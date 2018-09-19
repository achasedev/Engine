#pragma once
#include "Engine/Networking/NetAddress.hpp"

class UDPSocket;
class BytePacker;
class NetMessage;

class NetConnection
{
public:
	//-----Public Methods-----

	NetConnection(UDPSocket* boundSocket, NetAddress_t& targetAddress);
	~NetConnection();

	// Send and receive out of the bound socket for this connection
	int Send(NetMessage* msg);
	int Receive(NetMessage* out_msg, size_t maxSize);

	// Returns the target address this connection is associated with
	NetAddress_t GetTargetAddress();


private:
	//-----Private Data-----

	UDPSocket*		m_boundSocket = nullptr; // For now, only UDP supported, though could be generalized (?) to both UDP and TCP
	NetAddress_t	m_targetAddress;

};
