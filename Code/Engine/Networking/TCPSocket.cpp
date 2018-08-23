/************************************************************************/
/* File: TCPSocket.cpp
/* Author: Andrew Chase
/* Date: August 23rd, 2018
/* Description: Implementation of the TCPSocket class
/************************************************************************/
#include "Engine/Core/LogSystem.hpp"
#include "Engine/Networking/TCPSocket.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"

#define WIN_32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>


//-----------------------------------------------------------------------------------------------
// Constructor
//
TCPSocket::TCPSocket()
{
	m_socketHandle = (Socket_t*)INVALID_SOCKET;
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
TCPSocket::~TCPSocket()
{
	if (!IsClosed())
	{
		LogTaggedPrintf("NET", "Warning: Socket destroyed without being closed");
		Close();
	}
}


//-----------------------------------------------------------------------------------------------
// Listens for a connection under the given port
//
bool TCPSocket::Listen(unsigned short port, unsigned int maxQueued)
{
	return false;
}


//-----------------------------------------------------------------------------------------------
// Blocks until a connection is received, then accepts
//
TCPSocket* TCPSocket::Accept()
{
	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Receives data from the current connection
//
size_t TCPSocket::Receive(void *buffer, size_t const maxByteSize)
{
	if (IsClosed())
	{
		LogTaggedPrintf("NET", "Warning: TCPSocket::Receive called on a closed connection");
		return 0;
	}

	// with TCP/IP, data sent together is not guaranteed to arrive together.  
	// so make sure you check the return value.  This will return SOCKET_ERROR
	// if the host disconnected, or if we're non-blocking and no data is there. 
	size_t sizeReceived = ::recv((SOCKET)m_socketHandle, (char*) buffer, (int) maxByteSize, 0);

	LogTaggedPrintf("NET", "Received: %s", buffer);

	return sizeReceived;
}


//-----------------------------------------------------------------------------------------------
// Connects this socket to the NetAddress provided
//
bool TCPSocket::Connect(const NetAddress_t& netAddress)
{
	if (!IsClosed())
	{
		LogTaggedPrintf("NET", "Warning: TCPSocket::Connect called on a socket that is still connected to a connection");
		Close();
	}

	// Create a socket
	m_socketHandle = (Socket_t*) ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if ((SOCKET)m_socketHandle == INVALID_SOCKET) {
		LogTaggedPrintf("NET", "Error: Could not create socket");
		return false;
	}

	sockaddr_storage saddr;
	size_t addrlen;         
	netAddress.ToSockAddr((sockaddr*)&saddr, &addrlen);

	int result = ::connect((SOCKET)m_socketHandle, (sockaddr*)&saddr, (int)addrlen);
	if (result == SOCKET_ERROR)
	{
		LogTaggedPrintf("NET", "Couldn't connect to socket address %s", netAddress.ToString().c_str());
		::closesocket((SOCKET)m_socketHandle);
		return false;
	}

	LogTaggedPrintf("NET", "Connected to %s", netAddress.ToString().c_str());
	m_address = netAddress;
	return true;
}


//-----------------------------------------------------------------------------------------------
// Closes the current connection on this socket
//
void TCPSocket::Close()
{
	if (IsClosed())
	{
		LogTaggedPrintf("NET", "Warning: TCPSocket::Close called on a socket already closed");
		return;
	}
	// Close it
	::closesocket((SOCKET)m_socketHandle);

	// Clear the member variables
	m_address = NetAddress_t();
	m_socketHandle = (Socket_t*)INVALID_SOCKET;
}


//-----------------------------------------------------------------------------------------------
// Sends the given data out of this socket using its current connection
//
size_t TCPSocket::Send(const void* data, const size_t byteSize)
{
	if (IsClosed())
	{
		LogTaggedPrintf("NET", "Warning: TCPSocket::Send called on a socket that has no active connection");
		return 0;
	}

	size_t amountSent = ::send((SOCKET)m_socketHandle, (const char*)data, (int)byteSize, 0);
	return amountSent;
}


//-----------------------------------------------------------------------------------------------
// Returns true if this socket's connection is currently closed
//
bool TCPSocket::IsClosed() const
{
	return m_socketHandle == (Socket_t*)INVALID_SOCKET;
}
