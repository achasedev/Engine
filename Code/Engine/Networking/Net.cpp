/************************************************************************/
/* File: Net.cpp
/* Author: Andrew Chase
/* Date: August 20th, 2018
/* Description: Implementation of the Net class
/************************************************************************/
#include "Engine/Networking/Net.hpp"
#include "Engine/Core/LogSystem.hpp"
#include "Engine/Networking/NetAddress.hpp"
#include "Engine/Core/DeveloperConsole/Command.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

#define WIN_32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

void Command_GetAddressForHost(Command& cmd);
void Command_ConnectDirectWithWinSock(Command& cmd);

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
	Command::Register("net_address", "Returns the IP address for the given host name.", &Command_GetAddressForHost);
	Command::Register("net_connect", "Connects to a remote host and sends a message.", &Command_ConnectDirectWithWinSock);

	ASSERT_RECOVERABLE(error == 0, "Error: WSAStartup failed to initialze the sock API");
	return (error == 0);
}


//-----------------------------------------------------------------------------------------------
// Shuts down and cleans up the net system
//
void Net::Shutdown()
{
	// Not necessary, but a good habit
	::WSACleanup();
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


//-----------------------------------------------------------------------------------------------
// Command to print this computer's local IP address to the DevConsole
//
void Command_GetAddressForHost(Command& cmd)
{
	std::string hostname;
	cmd.GetParam("h", hostname);

	sockaddr_in address;
	memset(&address, 0, sizeof(sockaddr));
	int addressLength = 0;

	// If no name was specified, default to this device's host name
	if (IsStringNullOrEmpty(hostname))
	{
		bool foundLocalHostname = Net::GetLocalHostName(hostname);

		if (!foundLocalHostname)
		{
			return;
		}
	}

	// Get the address
	bool succeeded = Net::GetAddressForHost(&address, &addressLength, hostname.c_str());

	if (succeeded)
	{
		// Print out the address
		char out[256];
		::inet_ntop(address.sin_family, &(address.sin_addr), out, 256);
		ConsolePrintf(Rgba::GREEN, "IP address for hostname %s is %s", hostname.c_str(), out);
		LogTaggedPrintf("NET", "IP address for hostname %s is %s", hostname.c_str(), out);
	}
	else
	{
		ConsoleErrorf("Couldn't find address for hostname %s", hostname.c_str());
		LogTaggedPrintf("NET", "Couldn't find address for hostname %s", hostname.c_str());
	}
}


void Command_ConnectDirectWithWinSock(Command& cmd)
{
	std::string addr_string, ip, port, message;

	bool messageSpecified = cmd.GetParam("m", message);
	bool addressSpecified = cmd.GetParam("a", addr_string);
	
	if (!messageSpecified || !addressSpecified)
	{
		ConsoleErrorf("No message or address specified, use the -m and -a flags");
		return;
	}

	NetAddress_t addr(addr_string.c_str());

	TODO("Check for valid address");

	// First create the socket
	SOCKET sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		ConsoleErrorf("Couldn't create the socket");
		return;
	}

	sockaddr_storage sockAddrStorage;	// structure large enough to store any sockaddr
	size_t sockAddrLen;					// How large the actual sockaddr_ * structure is

	addr.ToSockAddr((sockaddr*)&sockAddrStorage, &sockAddrLen);

	int result = ::connect(sock, (sockaddr*)&sockAddrStorage, (int)sockAddrLen);
	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		ConsoleErrorf("Couldn't connect");
		::closesocket(sock);
		return;
	}

	ConsolePrintf("Connected to %s", addr_string.c_str());

	// Send the message now
	::send(sock, message.c_str(), (int) message.size(), 0);

	// with TCP/IP, data sent together is not guaranteed to arrive together.  
	// so make sure you check the return value.  This will return SOCKET_ERROR
	// if the host disconnected, or if we're non-blocking and no data is there. 
	char payload[256];
	size_t recvd = ::recv(sock, payload, 256 - 1U, 0);

	// it may not be null terminated and I'm printing it, so just for safety. 
	payload[recvd] = NULL;
	ConsolePrintf("Received: %s", payload);

	// cleanup
	::closesocket(sock);
}