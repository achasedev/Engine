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
	TCPSocket(bool shouldBlock);
	TCPSocket(Socket_t* socketHandle, NetAddress_t& netAddress, bool isListening = false, bool isBlocking = true);
	~TCPSocket();


	bool				Listen(unsigned short port, unsigned int maxQueued);
	TCPSocket*			Accept();
	bool				Connect(const NetAddress_t& addr);
	void				Close();

	int					Send(const void* data, const size_t byteSize);
	int					Receive(void *buffer, size_t const max_byte_size);

	bool				IsClosed() const;
	bool				IsListening() const;
	bool				IsBlocking() const;

	void				SetBlocking(bool blockingState);

	NetAddress_t		GetNetAddress() const;


private:
	//-----Private Methods-----

	bool				IsStillConnecting();
	bool				IsConnected();


private:
	//-----Private Data-----

	Socket_t*		m_socketHandle = nullptr;
	bool			m_isListening = false;
	bool			m_isBlocking = true;



	// If listening, the address is YOUR address
	// If connecting (or socket from an accept) this address is THEIR address
	// DO NOT LISTEN AND CONNECT ON THE SAME SOCKET
	NetAddress_t	m_address;

};
