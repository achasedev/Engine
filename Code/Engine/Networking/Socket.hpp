/************************************************************************/
/* File: Socket.hpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Class to represent a Windows network socket
/************************************************************************/
#pragma once
#include "Engine/Networking/NetAddress.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
#include <stdint.h>

#define GAME_PORT 10084
#define ETHERNET_MTU 1500	// maximum transmission unit - determined by hardware part of OSI model.
							// 1500 is the MTU of EthernetV2, and is the minimum one - so we use it; 

#define PACKET_MTU (ETHERNET_MTU - 40 - 8) 

// IPv4 Header Size: 20B
// IPv6 Header Size: 40B
// TCP Header Size: 20B-60B
// UDP Header Size: 8B 
// Ethernet: 28B, but MTU is already adjusted for it
// so packet size is 1500 - 40 - 8 => 1452B (why?)

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

class Socket
{
public:
	//-----Public Methods-----

	Socket();
	virtual ~Socket(); // closes

	void SetBlocking(bool blocking);

	void Close(); // closesocket and set m_handle = INVALID_SOCKET
	bool IsClosed() const; // m_handle == (socket_t)INVALID_SOCKET; 
	bool IsBlocking() const;

	NetAddress_t GetNetAddress() const;


protected:
	//-----Protected Methods-----

	NetAddress_t	m_address; // address assocated with this socket; 
	Socket_t*		m_socketHandle; // initialized to INVALID_SOCKET

	// used if you want to set options while closed, 
	// used to keep track so you can apply_options() after the socket
	// gets created; 
	eSocketOptions m_options;

};

bool WasLastErrorFatal(int& errorCode);
