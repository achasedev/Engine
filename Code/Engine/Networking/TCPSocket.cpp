/************************************************************************/
/* File: TCPSocket.cpp
/* Author: Andrew Chase
/* Date: August 23rd, 2018
/* Description: Implementation of the TCPSocket class
/************************************************************************/
#include "Engine/Core/LogSystem.hpp"
#include "Engine/Networking/TCPSocket.hpp"
#include "Engine/Networking/NetAddress.hpp"
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
// Constructor from member values, used for client sockets from an accept
//
TCPSocket::TCPSocket(Socket_t* socketHandle, NetAddress_t& netAddress, bool isListening /*= false*/)
	: m_socketHandle(socketHandle)
	, m_address(netAddress)
	, m_isListening(isListening)
{
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
	if (!IsClosed())
	{
		LogTaggedPrintf("NET", "Warning: TCPSocket::Listen called on a TCPSocket with an existing connection");
		Close();
	}

	// Get an address to bind to
	NetAddress_t addr;
	if (!NetAddress_t::GetLocalBindableAddress(&addr, port)) {
		return false;
	}

	// Now we have a bindable address, we can try to bind it; 
	// First, we create a socket like we did before; 
	m_socketHandle = (Socket_t*) ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Next, we bind it - which means we assign an address to it; 
	sockaddr_storage saddr;
	size_t addrlen;
	addr.ToSockAddr((sockaddr*)&saddr, &addrlen);

	int result = ::bind((SOCKET)m_socketHandle, (sockaddr*)&saddr, (int) addrlen);
	if (result == SOCKET_ERROR) 
	{	
		int errorCode = WSAGetLastError();
		LogTaggedPrintf("NET", "Error: TCPSocket::Listen couldn't bind the socket to address %s, error code &i", addr.ToString().c_str(), errorCode);

		Close();
		return false;
	}

	// We now have a bound socket - this means we can start listening on it; 
	// This allows the socket to queue up connections
	result = ::listen((SOCKET)m_socketHandle, maxQueued);
	if (result == SOCKET_ERROR) {

		int errorCode = WSAGetLastError();

		LogTaggedPrintf("NET", "Error: TCPSocket::Listen couldn't bind the socket to address %s, error code %i", addr.ToString().c_str(), errorCode);
		Close();
		return false;
	}

	// Successfully listening
	m_isListening = true;
	return m_isListening;
}


//-----------------------------------------------------------------------------------------------
// Attempts to accept a queued connection, returns nullptr if no connection is queued
//
TCPSocket* TCPSocket::Accept()
{
	sockaddr_storage clientAddr;
	int clientAddrLen = sizeof(sockaddr_storage);

	SOCKET clientSocketHandle = ::accept((SOCKET)m_socketHandle, (sockaddr*)&clientAddr, &clientAddrLen);

	if (clientSocketHandle == SOCKET_ERROR)
	{
		int errorCode = ::WSAGetLastError();
		LogTaggedPrintf("NET", "Error: TCPSocket::Accept() couldn't accept client connection, error code %i", errorCode);
		return nullptr;
	}

	// Client successfully accepted
	NetAddress_t clientNetAddress = NetAddress_t((const sockaddr*)&clientAddr);
	TCPSocket* clientSocket = new TCPSocket((Socket_t*)clientSocketHandle, clientNetAddress, false);
	
	return clientSocket;
}


//-----------------------------------------------------------------------------------------------
// Receives data from the current connection
//
int TCPSocket::Receive(void *buffer, size_t const maxByteSize)
{
	if (IsClosed())
	{
		LogTaggedPrintf("NET", "Warning: TCPSocket::Receive called on a closed connection");
		return 0;
	}

	// with TCP/IP, data sent together is not guaranteed to arrive together.  
	// so make sure you check the return value.  This will return SOCKET_ERROR
	// if the host disconnected, or if we're non-blocking and no data is there. 
	int sizeReceived = ::recv((SOCKET)m_socketHandle, (char*) buffer, (int) maxByteSize, 0);

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
		Close();
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
		return;
	}

	// Close it
	::closesocket((SOCKET)m_socketHandle);

	// Clear the member variables
	m_address = NetAddress_t();
	m_socketHandle = (Socket_t*)INVALID_SOCKET;
	m_isListening = false;
}


//-----------------------------------------------------------------------------------------------
// Sends the given data out of this socket using its current connection
//
int TCPSocket::Send(const void* data, const size_t byteSize)
{
	if (IsClosed())
	{
		LogTaggedPrintf("NET", "Warning: TCPSocket::Send called on a socket that has no active connection");
		return 0;
	}

	int amountSent = ::send((SOCKET)m_socketHandle, (const char*)data, (int)byteSize, 0);

	if (amountSent == SOCKET_ERROR)
	{
		int errorCode = ::WSAGetLastError();
		LogTaggedPrintf("NET", "Error: TCPSocket::Send couldn't send, error %i", errorCode);

		// For now assume all send errors are disconnects
		Close();
	}

	return amountSent;
}


//-----------------------------------------------------------------------------------------------
// Returns true if this socket's connection is currently closed
//
bool TCPSocket::IsClosed() const
{
	return m_socketHandle == (Socket_t*)INVALID_SOCKET;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the TCPSocket is currently bound to an address and is listening for connections
//
bool TCPSocket::IsListening() const
{
	return m_isListening;
}
