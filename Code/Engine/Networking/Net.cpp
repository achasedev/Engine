/************************************************************************/
/* File: Net.cpp
/* Author: Andrew Chase
/* Date: August 20th, 2018
/* Description: Implementation of the Net class
/************************************************************************/
#include "Engine/Networking/Net.hpp"
#include "Engine/Core/LogSystem.hpp"
#include "Engine/Networking/TCPSocket.hpp"
#include "Engine/Networking/NetAddress.hpp"
#include "Engine/Core/DeveloperConsole/Command.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

#define WIN_32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

bool Net::s_isRunning = false;

//-----------------------------------------------------------------------------------------------
// Starts up the network system
//
bool Net::Initialize()
{
	// Pick the version we want
	WORD version = MAKEWORD(2, 2);

	// Initialize the (W)in(S)ock(A)PI
	// Data will store information about the system (and some limitations)
	// Will only use a few sockets
	WSADATA data;
	int error = ::WSAStartup(version, &data);

	// Check if it succeeded
	bool success = (error == 0);
	ASSERT_RECOVERABLE(success, "Error: WSAStartup failed to initialze the sock API");

	if (success)
	{
		s_isRunning = true;
	}
	
	return success;
}


//-----------------------------------------------------------------------------------------------
// Shuts down and cleans up the net system
//
void Net::Shutdown()
{
	// Not necessary, but a good habit
	::WSACleanup();

	s_isRunning = false;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the Net system is currently running, false otherwise
//
bool Net::IsRunning()
{
	return s_isRunning;
}


//-----------------------------------------------------------------------------------------------
// Returns in string format the host name of this device
//
bool Net::GetLocalHostName(std::string& out_hostname)
{
	// If no name specified, just look up the name of this device
	char hostname[256];

	if (SOCKET_ERROR == ::gethostname(hostname, 256))
	{
		LogTaggedPrintf("NET", "Couldn't find host name of current device.");
		ConsoleErrorf("Couldn't find host name of current device");
		return false;
	}
	else if (IsStringNullOrEmpty(hostname))
	{
		LogTaggedPrintf("NET", "Host name for current device is empty");
		ConsoleErrorf("Host name for current device is empty");
		return false;
	}

	out_hostname = hostname;
	return true;
}


//-----------------------------------------------------------------------------------------------
// Returns in string format the address for the host given by hostname
//
bool Net::GetAddressForHost(sockaddr_in* out_addr, int* out_addrlen, const char* hostname, const char* service /*= "12345"*/, bool getHostableAddress /*= false*/)
{
	// There are many different ways to communicate with this machine, so we provide
	// hints to the API to filter down to only the addresses we care about
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;			// IPv4 Addresses
	hints.ai_socktype = SOCK_STREAM;	// TCP socket (SOCK_DGRAM for UDP, this doesn't really matter for just getting the host name)

	if (getHostableAddress)
	{
		hints.ai_flags = AI_PASSIVE;		// An address we can host on
	}

	//hints.ai_flags |= AI_NUMERICHOST;		// Will speed up this function since it won't have to lookup the address

	addrinfo* result = nullptr;
	int status = ::getaddrinfo(hostname, service, &hints, &result);
	if (status != 0)
	{
		return false;
	}

	// Result is a linked list of addresses that match our filter
	bool foundAddress = false;
	addrinfo* itr = result;
	while (itr != nullptr)
	{
		// Could filter more here if we like, or return all of them and try them in order
		// For example, if you're using VPN you'll get two unique addresses for yourself
		// If we're using AF_INET, the address is a sockaddr_in

		if (itr->ai_family == AF_INET)
		{
			foundAddress = true;
			sockaddr_in* ipv4 = (sockaddr_in*)(itr->ai_addr);

			// Return the actual address
			memcpy(out_addr, ipv4, sizeof(sockaddr_in));
			*out_addrlen = sizeof(sockaddr_in);
			foundAddress = true;
			break;
		}

		itr = itr->ai_next;
	}

	::freeaddrinfo(result);
	return foundAddress;
}
