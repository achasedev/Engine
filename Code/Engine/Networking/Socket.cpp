/************************************************************************/
/* File: Socket.cpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Implementation of the Socket class
/************************************************************************/
#include "Engine/Networking/Socket.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/LogSystem.hpp"

#define WIN_32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>


//-----------------------------------------------------------------------------------------------
// For error checking against non-fatal errors with non-blocking sockets
//
bool WasLastErrorFatal(int& errorCode)
{
	errorCode = ::WSAGetLastError();

	if (errorCode == WSAEWOULDBLOCK || errorCode == WSAEMSGSIZE || errorCode == WSAECONNRESET)
	{
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
// Constructor
//
Socket::Socket()
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Socket::~Socket()
{

}


//-----------------------------------------------------------------------------------------------
// Sets whether or not this socket will block on receives, accepts, etc
//
void Socket::SetBlocking(bool blockingState)
{
	// 0 is blocking, 1 is non-blocking
	u_long state = (blockingState ? 0 : 1);
	::ioctlsocket((SOCKET)m_socketHandle, FIONBIO, &state);

	if (blockingState)
	{
		SetBits(m_options, (1 << 0));
	}
	else
	{
		ClearBits(m_options, (1 << 0));
	}
}


//-----------------------------------------------------------------------------------------------
// Closes the current connection on this socket
//
void Socket::Close()
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
}


//-----------------------------------------------------------------------------------------------
// Returns true if this socket's connection is currently closed
//
bool Socket::IsClosed() const
{
	return m_socketHandle == (Socket_t*)INVALID_SOCKET;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this socket is blocking or not
//
bool Socket::IsBlocking() const
{
	return AreBitsSet(m_options, (1 << 0));
}


//-----------------------------------------------------------------------------------------------
// Returns the NetAddress currently being used by this TCPSocket
//
NetAddress_t Socket::GetNetAddress() const
{
	return m_address;
}
