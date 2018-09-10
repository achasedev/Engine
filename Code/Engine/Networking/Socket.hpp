#pragma once
#include "Engine/Networking/NetAddress.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
#include <stdint.h>

// For type safety without including Windows.h
class Socket_t;

enum eSocketOptionBit : uint32_t
{
 	SOCKET_OPTION_BLOCKING = (1 << 0),
// 
// 	// some other options that may be useful to support
// 	// SOCKET_OPTION_BROADCAST - socket can broadcast messages (if supported by network)
// 	// SOCKET_OPTION_LINGER - wait on close to make sure all data is sent
// 	// SOCKET_OPTION_NO_DELAY - disable nagle's algorithm
};
typedef uint32_t eSocketOptions;

// Rip out things from TCPSocket and put it to a base leve; 
class Socket
{
public:
	Socket();
	virtual ~Socket(); // closes

	void SetBlocking(bool blocking);

	void Close(); // closesocket and set m_handle = INVALID_SOCKET
	bool IsClosed() const; // m_handle == (socket_t)INVALID_SOCKET; 
	bool IsBlocking() const;

	NetAddress_t GetNetAddress() const;


protected:
	NetAddress_t	m_address; // address assocated with this socket; 
	Socket_t*		m_socketHandle; // initialized to INVALID_SOCKET

	// used if you want to set options while closed, 
	// used to keep track so you can apply_options() after the socket
	// gets created; 
	eSocketOptions m_options;
};

// Make appropriate changes to TCPSocket if you want
// though nothing *required* in class will use TCPSocket again.

// net/udpsocket.hpp

// when searching for addresses - you are no longer looking for AF_INET
// you look for 

class UDPSocket : public Socket
{
public:
	bool bind(NetAddress_t const &addr,		// address I want to listen for traffic on
		uint16_t port_range = 0U);			// how many additional ports to bind on (so you bind from [addr.port,addr.port + port_range])

	// return 0 on failure, otherwise how large is the next datagram
	// TODO in class
	size_t send_to(NetAddress_t const &addr, void const *data, size_t const byte_count);
	size_t receive_from(NetAddress_t *out_addr, void *buffer, size_t const max_read_size);

};


// class test
#define GAME_PORT 10084
#define ETHERNET_MTU 1500  // maximum transmission unit - determined by hardware part of OSI model.
						 // 1500 is the MTU of EthernetV2, and is the minimum one - so we use it; 
#define PACKET_MTU (ETHERNET_MTU - 40 - 8) 

// IPv4 Header Size: 20B
// IPv6 Header Size: 40B
// TCP Headre Size: 20B-60B
// UDP Header Size: 8B 
// Ethernet: 28B, but MTU is already adjusted for it
// so packet size is 1500 - 40 - 8 => 1452B (why?)
#include "Engine/Math/MathUtils.hpp"

class UDPTest
{
public:

	bool start()
	{
		// get an address to use; 
		NetAddress_t addr;
		NetAddress_t::GetLocalAddress(&addr, GAME_PORT, true);

		if (!m_socket.bind(addr, 10)) {
			ConsolePrintf("Failed to bind.");
			return false;
		}
		else {
			m_socket.SetBlocking(false); // if you have cached options, you could do this
										  // in the constructor; 
			ConsolePrintf("Socket bound: %s", m_socket.GetNetAddress().ToString().c_str());
			return true;
		}
	}

	void stop()
	{
		m_socket.Close();
	}

	void send_to(NetAddress_t const &addr, void const *buffer, uint32_t byte_count)
	{
		m_socket.send_to(addr, buffer, byte_count);
	}

	void update()
	{
		uint8_t buffer[PACKET_MTU];

		NetAddress_t from_addr;
		size_t read = m_socket.receive_from(&from_addr, buffer, PACKET_MTU);

		if (read > 0U) {
			uint32_t max_bytes = MinInt(read, 128);

			uint32_t stringSize = max_bytes * 2U + 3U;
			char* buffer = (char*)malloc(stringSize);
			sprintf_s(buffer, 3U, "0x");

			char *iter = buffer;
			iter += 2U; // skip the 0x
			for (uint32_t i = 0; i < read; ++i) {
				sprintf_s(iter, 3U, "%02X", buffer[i]);
				iter += 2U;
			}
			*iter = NULL;

			ConsolePrintf("Received from %s, Message: %s", from_addr.ToString().c_str(),
				buffer);
		}
	}

public:
	// if you have multiple address, you can use multiple sockets
	// but you have to be clear on which one you're sending from; 
	UDPSocket m_socket;
};
