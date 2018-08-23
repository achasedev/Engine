/************************************************************************/
/* File: TCPSocket.hpp
/* Author: Andrew Chase
/* Date: August 23rd, 2018
/* Description: Class to represent a TCP socket to make connections with
/************************************************************************/
#pragma once
#include "Engine/Networking/NetAddress.hpp"

// For type safety without including Windows.h
class Socket_t;

class TCPSocket
{
public:
	//-----Public Methods-----

	TCPSocket();
	~TCPSocket();


	bool				Listen(unsigned short port, unsigned int maxQueued);
	TCPSocket*			Accept();
	bool				Connect(const NetAddress_t& addr);
	void				Close();

	size_t				Send(const void* data, const size_t byteSize);
	size_t				Receive(void *buffer, size_t const max_byte_size); 	

	bool				IsClosed() const;


private:
	//-----Private Data-----

	Socket_t*		m_socketHandle = nullptr;

	// If listening, the address is YOUR address
	// If connecting (or socket from an accept) this address is THEIR address
	// DO NOT LISTEN AND CONNECT ON THE SAME SOCKET
	NetAddress_t	m_address;

};
