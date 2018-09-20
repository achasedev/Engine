/************************************************************************/
/* File: UDPSocket.cpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Implementation of the UDPSocket class
/************************************************************************/
#include "Engine/Core/LogSystem.hpp"
#include "Engine/Networking/UDPSocket.hpp"

#ifndef WIN_32_LEAN_AND_MEAN
#define WIN_32_LEAN_AND_MEAN
#endif
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>


//-----------------------------------------------------------------------------------------------
// Binds the socket to the address given, incrementally increasing the port up to port_range, or if
// an available one is found
//
bool UDPSocket::Bind(NetAddress_t const &addr, uint16_t port_range /*= 0U*/)
{
	// create the socket 
	SOCKET my_socket = socket(AF_INET,		// IPv4 to send...
		SOCK_DGRAM,							// ...Datagrams... 
		IPPROTO_UDP);						// ...using UDP.

	if (my_socket == INVALID_SOCKET)
	{
		return false;
	}

	// TODO, try to bind all ports within the range.  
	// Shown - just trying one; 
	sockaddr_storage sock_addr;
	size_t sock_addr_len;
	addr.ToSockAddr((sockaddr*)&sock_addr, &sock_addr_len);

	// try to bind - if it succeeds - great.  If not, try the next port in the range.
	int result = ::bind(my_socket, (sockaddr*)&sock_addr, (int)sock_addr_len);
	if (0 == result) {
		m_socketHandle = (Socket_t*)my_socket;
		m_address = addr;
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Sends the given data to the address specified by netAddr
//
size_t UDPSocket::SendTo(NetAddress_t const &netAddr, void const *data, size_t const byte_count)
{
	if (IsClosed())
	{
		LogTaggedPrintf("NET", "Error: UDPSocket::SendTo() called on a closed UDP socket.");
		return 0;
	}

	sockaddr_storage addr;
	size_t addr_len;
	netAddr.ToSockAddr((sockaddr*)&addr, &addr_len);

	SOCKET sock = (SOCKET)m_socketHandle;

	int sent = ::sendto(
		sock,					// Socket we're sending from
		(char const*)data, 		// Data we want to send
		(int)byte_count, 		// byte count to send
		0, 						// unused flags
		(sockaddr*)&addr, 		// Address we're sending to
		addr_len);				// Length of addr we're sending to

	if (sent > 0)
	{
		ASSERT_RECOVERABLE(sent == byte_count, "UDPSocket::SendTo() couldn't sent all the bytes.");
		return (size_t)sent;
	}
	else
	{
		int errorCode;
		if (WasLastErrorFatal(errorCode))
		{
			LogTaggedPrintf("NET", "Error: UDPSocket::SendTo() received fatal error %i.", errorCode);
			Close();
			return 0;
		}
	}

}


//-----------------------------------------------------------------------------------------------
// Checks for any datagrams received on the given socket, returning the number of bytes received
//
size_t UDPSocket::ReceiveFrom(NetAddress_t *out_addr, void *buffer, size_t const max_read_size)
{
	if (IsClosed())
	{
		LogTaggedPrintf("NET", "Error: UDPSocket::ReceiveFrom() called on a closed UDPSocket.");
		return 0;
	}

	sockaddr_storage fromAddr;
	int addrLen = sizeof(sockaddr_storage);

	SOCKET sock = (SOCKET)m_socketHandle;

	int received = ::recvfrom(
		sock,						// Socket I am receiving on
		(char*)buffer,				// Buffer to read into
		(int)max_read_size,			// Max amount I can read
		0,							// Unused flags
		(sockaddr*)&fromAddr,		// Who sent it
		&addrLen);					// Their address length

	if (received > 0)
	{
		NetAddress_t((sockaddr*)&fromAddr);
		return received;
	}
	else
	{
		int errorCode;
		if (WasLastErrorFatal(errorCode))
		{
			Close();
		}

		return 0;
	}
}
