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
bool WasLastErrorFatalTemp(int& errorCode)
{
	errorCode = ::WSAGetLastError();

	if (errorCode == WSAEWOULDBLOCK || errorCode == WSAEMSGSIZE || errorCode == WSAECONNRESET)
	{
		return false;
	}

	return true;
}



bool UDPSocket::bind(NetAddress_t const &addr, uint16_t port_range /*= 0U*/)
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

size_t UDPSocket::send_to(NetAddress_t const &netAddr, void const *data, size_t const byte_count)
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
		if (WasLastErrorFatalTemp(errorCode))
		{
			LogTaggedPrintf("NET", "Error: UDPSocket::SendTo() received fatal error %i.", errorCode);
			Close();
			return 0;
		}
	}

}

size_t UDPSocket::receive_from(NetAddress_t *out_addr, void *buffer, size_t const max_read_size)
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
		if (WasLastErrorFatalTemp(errorCode))
		{
			Close();
		}

		return 0;
	}
}

Socket::Socket()
{
	// Eventually will default to being non-blocking
	//SetBlocking(true);
}

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
