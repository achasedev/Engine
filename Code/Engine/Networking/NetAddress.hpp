#pragma once
#include <string>

struct sockaddr;

struct NetAddress_t
{
public:
	//-----Public Methods-----

	NetAddress_t();
	NetAddress_t(const sockaddr* addr);
	NetAddress_t(const char* string);

	bool ToSockAddr(sockaddr* out_addr, size_t* out_addrLen);
	bool FromSockAddr(const sockaddr* addr);

	std::string ToString() const;

	static NetAddress_t GetLocal();

public:
	//-----Public Data-----

	unsigned int ip4Address = 0;
	unsigned short port = 0;

};