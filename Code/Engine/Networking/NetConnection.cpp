/************************************************************************/
/* File: NetConnection.cpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Implementation of the NetConnection class
/************************************************************************/
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetConnection.hpp"
#include "Engine/Networking/UDPSocket.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
NetConnection::NetConnection(UDPSocket* boundSocket, NetAddress_t& targetAddress)
{
	m_boundSocket = boundSocket;
	m_targetAddress = targetAddress;
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
size_t NetConnection::Send(NetMessage* msg)
{
	size_t dataSize = msg->GetWrittenByteCount();
	const void* data = msg->GetBuffer();
	size_t amountSent = m_boundSocket->SendTo(m_targetAddress, data, dataSize);

	return amountSent;
}


//-----------------------------------------------------------------------------------------------
// Receives on the bound socket up to maxSize amount, and returns the size read
// Outputs the read data in out_msg
//
size_t NetConnection::Receive(NetMessage* out_msg, size_t maxSize)
{
	void* buffer = malloc(maxSize);
	size_t amountReceived = m_boundSocket->ReceiveFrom(&m_targetAddress, buffer, maxSize);

	// Create the message
	//out_msg = new NetMessage(amountReceived, buffer);

	return amountReceived;
}


//-----------------------------------------------------------------------------------------------
// Returns the target address for this connection
//
NetAddress_t NetConnection::GetTargetAddress()
{
	return m_targetAddress;
}
