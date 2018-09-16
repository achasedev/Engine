// NetConnection.cpp

#include "Engine/Networking/Socket.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetConnection.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
NetConnection::NetConnection(UDPSocket* boundSocket, NetAddress_t& targetAddress)
{
	m_boundSocket = boundSocket;
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
NetConnection::~NetConnection()
{
}


//-----------------------------------------------------------------------------------------------
// Sends the given message out of the bound socket to the target address
// Returns the number of bytes sent
//
int NetConnection::Send(NetMessage* msg)
{
	size_t dataSize = msg->GetSize();
	void* data = msg->GetData();
	int amountSent = m_boundSocket->send_to(m_targetAddress, data, dataSize);

	return amountSent;
}


//-----------------------------------------------------------------------------------------------
// Receives on the bound socket up to maxSize amount, and returns the size read
// Outputs the read data in out_msg
//
int NetConnection::Receive(NetMessage* out_msg, size_t maxSize)
{
	void* buffer = malloc(maxSize);
	int amountReceived = m_boundSocket->receive_from(&m_targetAddress, buffer, maxSize);

	// Create the message
	out_msg = new NetMessage(amountReceived, buffer);

	return amountReceived;
}


//-----------------------------------------------------------------------------------------------
// Returns the target address for this connection
//
NetAddress_t NetConnection::GetTargetAddress()
{
	return m_targetAddress;
}
