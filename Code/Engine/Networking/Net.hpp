/************************************************************************/
/* File: Net.hpp
/* Author: Andrew Chase
/* Date: August 20th, 2018
/* Description: Static class for handling networking operations
/************************************************************************/
#pragma once
#pragma comment(lib, "ws2_32.lib") // WinSock libraries

#include <string>
struct sockaddr_in;

class Net
{
public:
	//-----Public Methods-----

	static bool Initialize();
	static void Shutdown();
	static bool IsRunning();

	static bool GetLocalHostName(std::string& out_hostname);
	static bool GetAddressForHost(sockaddr_in* out_addr, int* out_addrlen, const char* hostname, const char* service = "12345", bool getBindableAddresses = false);

	
private:
	//-----Private Data-----

	static bool s_isRunning;

};
