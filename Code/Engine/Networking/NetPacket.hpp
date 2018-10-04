/************************************************************************/
/* File: NetPacket.hpp
/* Author: Andrew Chase
/* Date: September 28th 2018
/* Description: Class to represent a packet of NetMessages for a NetSession
/************************************************************************/
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

#ifndef INVALID_CONNECTION_INDEX
#define INVALID_CONNECTION_INDEX (0xff)
#endif

// Predeclarations
class NetMessage;

// Header for the entire packet
struct PacketHeader_t
{
	PacketHeader_t() {}
	PacketHeader_t(uint8_t connectionIndex, uint8_t messageCount)
		: senderConnectionIndex(connectionIndex), unreliableMessageCount(messageCount) {}

	uint8_t senderConnectionIndex = INVALID_CONNECTION_INDEX;
	uint8_t unreliableMessageCount = 0;
};


class NetPacket : public BytePacker
{
public:
	friend class NetSession;

	//-----Public Methods-----

	NetPacket();
	NetPacket(uint8_t* buffer, size_t bufferSize);

	// Header Methods
	void		WriteHeader(const PacketHeader_t& header);
	bool		ReadHeader(PacketHeader_t& out_header);

	// Message Methods
	bool		WriteMessage(const NetMessage* message);
	bool		ReadMessage(NetMessage* out_message);

	// Mutators
	void		SetSenderConnectionIndex(uint8_t index);
	void		SetReceiverConnectionIndex(uint8_t index);

	// Accessors
	uint8_t		GetSenderConnectionIndex() const;
	uint8_t		GetReceiverConnectionIndex() const;


private:
	//-----Private Data-----

	uint8_t		m_localBuffer[PACKET_MTU];
	uint8_t		m_senderIndex = INVALID_CONNECTION_INDEX; 
	uint8_t		m_receiverIndex	= INVALID_CONNECTION_INDEX;

};
