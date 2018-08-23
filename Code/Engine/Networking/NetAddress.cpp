#include "Engine/Networking/NetAddress.hpp"

#define WIN_32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

NetAddress_t::NetAddress_t()
{
}

NetAddress_t::NetAddress_t(const sockaddr* addr)
{
	FromSockAddr(addr);
}

NetAddress_t::NetAddress_t(const char* string)
{

}

bool NetAddress_t::FromSockAddr(const sockaddr* addr)
{
	ip4Address = (unsigned int)(((sockaddr_in*)addr)->sin_addr.s_addr);
	port = (unsigned short)(((sockaddr_in*)addr)->sin_port);
	
	return true;
}

std::string NetAddress_t::ToString() const
{
	std::string stringAddress;

	char buffer[33];

	_itoa_s((ip4Address >> 24) & 0x000F, buffer, 10);
	stringAddress += buffer;

	_itoa_s((ip4Address >> 16) & 0x000F, buffer, 10);
	stringAddress += buffer;

	_itoa_s((ip4Address >> 8) & 0x000F, buffer, 10);
	stringAddress += buffer;

	_itoa_s(ip4Address & 0x000F, buffer, 10);
	stringAddress += buffer;

	return stringAddress;
}

