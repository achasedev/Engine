/************************************************************************/
/* File: TCPSocket.cpp
/* Author: Andrew Chase
/* Date: August 23rd, 2018
/* Description: Implementation of the TCPSocket class
/************************************************************************/
#include "Engine/Core/LogSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Networking/TCPSocket.hpp"
#include "Engine/Networking/NetAddress.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"

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
TCPSocket::TCPSocket()
{
	m_socketHandle = (Socket_t*)INVALID_SOCKET;
}


//-----------------------------------------------------------------------------------------------
// Constructor from member values, used for client sockets from an accept
//
TCPSocket::TCPSocket(Socket_t* socketHandle, NetAddress_t& netAddress, bool isListening /*= false*/, bool isBlocking /*= true*/)
	: m_socketHandle(socketHandle)
	, m_address(netAddress)
	, m_isListening(isListening)
	, m_isBlocking(isBlocking)
{
	SetBlocking(isBlocking);
}


//-----------------------------------------------------------------------------------------------
// Constructor to create non-blocking sockets, used for RemoteCommandService
//
TCPSocket::TCPSocket(bool shouldBlock)
	: m_isBlocking(shouldBlock)
{
	SetBlocking(shouldBlock);
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
	if (!NetAddress_t::GetLocalAddress(&addr, port, true)) {
		return false;
	}

	// Now we have a bindable address, we can try to bind it; 
	// First, we create a socket like we did before; 
	m_socketHandle = (Socket_t*) ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Ensure the socket is non-blocking if flagged as non-blocking
	SetBlocking(m_isBlocking);

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
	if (IsClosed())
	{
		LogTaggedPrintf("NET", "Error: TCPSocket::Accept() called on a closed socket.");
		return nullptr;
	}

	if (!m_isListening)
	{
		LogTaggedPrintf("NET", "Error: TCPSocket::Accept() called on a socket that isn't listening.");
		Close();
		return nullptr;
	}

	sockaddr_storage clientAddr;
	int clientAddrLen = sizeof(sockaddr_storage);

	SOCKET clientSocketHandle = ::accept((SOCKET)m_socketHandle, (sockaddr*)&clientAddr, &clientAddrLen);

	if (clientSocketHandle == SOCKET_ERROR)
	{
		int errorCode;
		if (WasLastErrorFatal(errorCode))
		{
			LogTaggedPrintf("NET", "Error: TCPSocket::Accept() couldn't accept client connection, error code %i.", errorCode);
		}

		return nullptr;
	}

	// Client successfully accepted
	NetAddress_t clientNetAddress = NetAddress_t((const sockaddr*)&clientAddr);
	TCPSocket* clientSocket = new TCPSocket((Socket_t*)clientSocketHandle, clientNetAddress, false, m_isBlocking); // Flag as non-blocking if accepted socked was non-blocking
	
	return clientSocket;
}


//-----------------------------------------------------------------------------------------------
// Receives data from the current connection
//
int TCPSocket::Receive(void *buffer, size_t const maxByteSize)
{
	if (!IsConnected())
	{
		return 0;
	}

	// with TCP/IP, data sent together is not guaranteed to arrive together.  
	// so make sure you check the return value.  This will return SOCKET_ERROR
	// if the host disconnected, or if we're non-blocking and no data is there. 
	int sizeReceived = ::recv((SOCKET)m_socketHandle, (char*) buffer, (int) maxByteSize, 0);

	if (sizeReceived == SOCKET_ERROR)
	{
		int errorCode;
		if (WasLastErrorFatal(errorCode))
		{
			LogTaggedPrintf("NET", "Error: TCPSocket::Receive() failed unexpectantly, error code %i.", errorCode);
			Close();
			return sizeReceived;
		}
	}

	// If it was less than 0 but not an error, just return 0
	return ClampInt(sizeReceived, 0, (int)maxByteSize);
}


//-----------------------------------------------------------------------------------------------
// Connects this socket to the NetAddress provided
//
bool TCPSocket::Connect(const NetAddress_t& netAddress)
{
	if (IsStillConnecting())
	{
		LogTaggedPrintf("NET", "Warning: TCPSocket::Connect() called on a non-blocking socket that is still trying to connect to a connection");
		Close();
	}
	else if (IsConnected())
	{
		LogTaggedPrintf("NET", "Warning: TCPSocket::Connect() called on a socket that is still connected to a connection");
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

	// Set the blocking state
	SetBlocking(m_isBlocking);

	int result = ::connect((SOCKET)m_socketHandle, (sockaddr*)&saddr, (int)addrlen);
	if (result == SOCKET_ERROR)
	{
		int errorCode;
		if (WasLastErrorFatal(errorCode))
		{
			LogTaggedPrintf("NET", "TCPSocket::Connect() couldn't connect to socket address %s", netAddress.ToString().c_str());
			Close();
			return false;
		}
	}

	if (m_isBlocking)
	{
		LogTaggedPrintf("NET", "Connected to %s", netAddress.ToString().c_str());
	}
	else
	{
		LogTaggedPrintf("NET", "Non-blocking socket connecting to %s...", netAddress.ToString().c_str());
	}

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
	if (!IsConnected())
	{
		LogTaggedPrintf("NET", "Warning: TCPSocket::Send called on a blocking socket that has no active connection");
		return 0;
	}

	int amountSent = ::send((SOCKET)m_socketHandle, (const char*)data, (int)byteSize, 0);

	if (amountSent == SOCKET_ERROR)
	{
		int errorCode;
		if (WasLastErrorFatal(errorCode))
		{
			LogTaggedPrintf("NET", "Error: TCPSocket::Send() couldn't send, error %i", errorCode);
			Close();
			return 0;
		}
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


//-----------------------------------------------------------------------------------------------
// Returns whether this socket is blocking or not
//
bool TCPSocket::IsBlocking() const
{
	return m_isBlocking;
}


//-----------------------------------------------------------------------------------------------
// Sets whether or not this socket will block on receives, accepts, etc
//
void TCPSocket::SetBlocking(bool blockingState)
{
	// 0 is blocking, 1 is non-blocking
	u_long state = (blockingState ? 0 : 1);
	::ioctlsocket((SOCKET)m_socketHandle, FIONBIO, &state);

	m_isBlocking = blockingState;
}


//-----------------------------------------------------------------------------------------------
// Returns the NetAddress currently being used by this TCPSocket
//
NetAddress_t TCPSocket::GetNetAddress() const
{
	return m_address;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this socket is active and still connecting to a connection
//
bool TCPSocket::IsStillConnecting()
{
	// Blocking sockets won't have this check
	if (m_isBlocking)
	{
		return false;
	}

	// If no socket at all, then not connecting
	if (IsClosed())
	{
		return false;
	}

	// Check the socket's status
	SOCKET socket = (SOCKET)m_socketHandle;
	WSAPOLLFD fd;

	fd.fd = socket;
	fd.events = POLLWRNORM;

	if (::WSAPoll(&fd, 1, 0) == SOCKET_ERROR)
	{
		// Socket is bad, so close it
		Close();
		return false;
	}

	if ((fd.revents & POLLHUP) != 0)
	{
		// Socket was [H]ung-[U]p
		Close();
		return false;
	}

	if ((fd.revents & POLLWRNORM) != 0)
	{
		// Socket can read/write, i.e. is connected
		return false;
	}

	// Not connected but socket is still good, so still connecting
	return true;
}


//-----------------------------------------------------------------------------------------------
// Returns true if this socket is connected to a connection
//
bool TCPSocket::IsConnected()
{
	// Not connected if we're closed
	if (IsClosed())
	{
		return false;
	}

	// Socket's good but not finished connecting
	if (IsStillConnecting())
	{
		return false;
	}

	// Socket's good and we can read/write, so connected
	return true;
}
