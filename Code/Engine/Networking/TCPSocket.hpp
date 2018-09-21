/************************************************************************/
/* File: TCPSocket.hpp
/* Author: Andrew Chase
/* Date: August 23rd, 2018
/* Description: Class to represent a TCP socket to make connections with
/************************************************************************/
#pragma once
#include "Engine/Networking/Socket.hpp"

class TCPSocket : public Socket
{
public:
	//-----Public Methods-----

	TCPSocket();
	TCPSocket(Socket_t* socketHandle, NetAddress_t& netAddress, bool isListening = false, bool isBlocking = true);
	~TCPSocket();


	bool				Listen(unsigned short port, unsigned int maxQueued);
	TCPSocket*			Accept();
	bool				Connect(const NetAddress_t& addr);

	int					Send(const void* data, const size_t byteSize);
	int					Receive(void *buffer, size_t const max_byte_size);

	bool				IsListening() const;


private:
	//-----Private Methods-----

	bool				IsStillConnecting();
	bool				IsConnected();


private:
	//-----Private Data-----

	bool				m_isListening = false;

};
