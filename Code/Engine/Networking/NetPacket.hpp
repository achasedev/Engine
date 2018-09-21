#pragma once
#include "Engine/Networking/BytePacker.hpp"

// maximum transmission unit - determined by hardware part of OSI model.
// 1500 is the MTU of EthernetV2, and is the minimum one - so we use it; 
#ifndef ETHERNET_MTU
#define ETHERNET_MTU 1500
#endif

#ifndef PACKET_MTU
#define PACKET_MTU (ETHERNET_MTU - 40 - 8) 
#endif

// Predeclarations
class NetMessage;

// Header for the entire packet
struct PacketHeader_t
{
	uint8_t senderConnectionIndex;
	uint8_t unreliableMessageCount;
};


class NetPacket : public BytePacker
{
public:
	//-----Public Methods-----

	NetPacket();
	
	void WriteHeader(const PacketHeader_t& header);
	bool ReadHeader(PacketHeader_t& out_header);

	bool WriteMessage(const NetMessage& message);
	bool ReadMessage(NetMessage& out_message);


private:
	//-----Private Data-----

	uint8_t		m_localBuffer[PACKET_MTU];
	uint32_t	m_senderConnectionIndex;
	uint32_t	m_receiverConnectionIndex;

};
