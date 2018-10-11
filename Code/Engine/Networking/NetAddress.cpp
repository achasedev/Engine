/************************************************************************/
/* File: NetAddress.cpp
/* Author: Andrew Chase
/* Date: August 23rd, 2018
/* Description: Implementation of the NetAddress_t struct
/************************************************************************/
#include "Engine/Networking/Net.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Networking/NetAddress.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include <vector>

#define WIN_32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>


//-----------------------------------------------------------------------------------------------
// Constructor
//
NetAddress_t::NetAddress_t()
{
}


//-----------------------------------------------------------------------------------------------
// Constructor from a windows socket address
//
NetAddress_t::NetAddress_t(const sockaddr* addr)
{
	FromSockAddr(addr);
}


//-----------------------------------------------------------------------------------------------
// Constructor from a string representation of the address
// e.g. "10.8.151.155:12345" or "google.com:80"
//
NetAddress_t::NetAddress_t(const char* addressText, bool bindable /*= false*/)
{
	if (IsStringNullOrEmpty(addressText))
	{
		return;
	}

	// Parse the string
	std::vector<std::string> tokens = Tokenize(addressText, ':');

	std::string hostname = tokens[0];
	if (hostname == "localhost")
	{
		Net::GetLocalHostName(hostname);
	}

	// Default the port string to 12345 if one isn't specified
	std::string portString = "12345";
	if (tokens.size() > 1)
	{
		portString = tokens[1];
	}

	// Get the socket address
	sockaddr_in sockAddr;
	int addrLen;
	Net::GetAddressForHost(&sockAddr, &addrLen, hostname.c_str(), portString.c_str(), bindable);

	// Set member variables
	FromSockAddr((const sockaddr*)&sockAddr);

}


//-----------------------------------------------------------------------------------------------
// Compares two addresses, and returns true if they match
//
bool NetAddress_t::operator==(const NetAddress_t& compare) const
{
	return ((ipv4Address == compare.ipv4Address) && (port == compare.port));
}


//-----------------------------------------------------------------------------------------------
// Extracts the member information from this NetAddress and stores it in the socket address provided
//
bool NetAddress_t::ToSockAddr(sockaddr* out_addr, size_t* out_addrLen) const
{
	*out_addrLen = sizeof(sockaddr_in);

	sockaddr_in* ipv4 = (sockaddr_in*)out_addr;
	memset(ipv4, 0, sizeof(sockaddr_in*));

	ipv4->sin_family = AF_INET;
	ipv4->sin_addr.S_un.S_addr = ipv4Address;
	ipv4->sin_port = ::htons(port);

	return true;
}


//-----------------------------------------------------------------------------------------------
// Sets the member variables of this NetAddress to correspond to the socket address data provided
//
bool NetAddress_t::FromSockAddr(const sockaddr* addr)
{
	if (addr->sa_family != AF_INET)
	{
		return false;
	}

	const sockaddr_in *ipv4 = (sockaddr_in const *)addr;
	ipv4Address = ipv4->sin_addr.S_un.S_addr;
	port = ::ntohs(ipv4->sin_port);
	
	return true;
}


//-----------------------------------------------------------------------------------------------
// Converts the NetAddress to a printable string format
//
std::string NetAddress_t::ToString() const
{
	uint8_t *array = (uint8_t*)&ipv4Address;
	std::string string = Stringf("%u.%u.%u.%u:%u", array[0], array[1], array[2], array[3], port);

	return string;
}


//-----------------------------------------------------------------------------------------------
// Returns the NetAddress corresponding to this device's IP address using the port given
//
STATIC bool NetAddress_t::GetLocalAddress(NetAddress_t* out_addr, unsigned short port, bool bindable)
{
	// Get the host name for this device
	std::string localHostName;
	bool foundHostName = Net::GetLocalHostName(localHostName);

	if (!foundHostName)
	{
		return false;
	}

	// Get the IP address for this device
	sockaddr_in sockAddr;
	int sockAddrLen;
	bool foundAddress = Net::GetAddressForHost(&sockAddr, 
		&sockAddrLen, 
		localHostName.c_str(), 
		Stringf("%u", port).c_str(), 
		bindable	// Sets AI_PASSIVE
	); 

	if (!foundAddress)
	{
		return false;
	}

	// Make the NetAddress
	out_addr->FromSockAddr((sockaddr*)&sockAddr);
	return true;
}
