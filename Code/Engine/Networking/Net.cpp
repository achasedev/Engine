/************************************************************************/
/* File: Net.cpp
/* Author: Andrew Chase
/* Date: August 20th, 2018
/* Description: Implementation of the Net class
/************************************************************************/
#include "Engine/Networking/Net.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

#define WIN_32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>


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
