#include "Engine/Networking/Net.hpp"
#include "Engine/Networking/NetAddress.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include <vector>

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
	std::vector<std::string> tokens = Tokenize(string, ':');

	std::string hostname = tokens[0];
	std::string portString = tokens[1];
	
	sockaddr_in sockAddr;
	int addrLen;
	Net::GetAddressForHost(&sockAddr, &addrLen, hostname.c_str(), portString.c_str(), false);

	FromSockAddr((const sockaddr*)&sockAddr);

	std::string test = ToString();
}

bool NetAddress_t::ToSockAddr(sockaddr* out_addr, size_t* out_addrLen)
{
	*out_addrLen = sizeof(sockaddr_in);

	sockaddr_in* ipv4 = (sockaddr_in*)out_addr;
	memset(ipv4, 0, sizeof(sockaddr_in*));

	ipv4->sin_family = AF_INET;
	ipv4->sin_addr.S_un.S_addr = ip4Address;
	ipv4->sin_port = ::htons(port);

	return true;
}

bool NetAddress_t::FromSockAddr(const sockaddr* addr)
{
	if (addr->sa_family != AF_INET)
	{
		return false;
	}

	const sockaddr_in *ipv4 = (sockaddr_in const *)addr;
	ip4Address = ipv4->sin_addr.S_un.S_addr;
	port = ::ntohs(ipv4->sin_port);
	
	return true;
}

std::string NetAddress_t::ToString() const
{
	uint8_t *array = (uint8_t*)&ip4Address;
	std::string string = Stringf("%u.%u.%u.%u:%u", array[0], array[1], array[2], array[3], port);

	return string;
}
