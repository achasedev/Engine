/************************************************************************/
/* File: UDPSocket.hpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Class to represent a Windows UDP socket
/************************************************************************/
#pragma once
#include "Engine/Networking/Socket.hpp"

class UDPSocket : public Socket
{
public:
	//-----Public Methods-----

	bool Bind(
		NetAddress_t const &addr,		// address I want to listen for traffic on
		uint16_t port_range = 0U		// how many additional ports to bind on (so you bind from [addr.port,addr.port + port_range])
	);			

	size_t SendTo(NetAddress_t const &addr, void const *data, size_t const byte_count);
	size_t ReceiveFrom(NetAddress_t *out_addr, void *buffer, size_t const max_read_size);

};
