/************************************************************************/
/* File: NetMessage.hpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Class to represent a single message from a NetSession
/************************************************************************/
#pragma once
#include "Engine/Networking/BytePacker.hpp"
#include <string>

// Limit messages to 1KB
#define MESSAGE_MTU 1024

class BytePacker;
class NetConnection;
class NetMessage;

class NetMessage : public BytePacker
{
public:
	//-----Public Methods-----

	NetMessage();
	NetMessage(NetMessage&& moveFrom);
	NetMessage(uint8_t definitionIndex);
	NetMessage(uint8_t definitionIndex, void* payload, const int16_t& payloadSize);
	NetMessage(const NetMessage& copy) = delete;
	~NetMessage();

	NetMessage& operator=(NetMessage&& moveFrom);
	NetMessage& operator=(const NetMessage&) = delete;

	// Accessors
	uint8_t							GetDefinitionIndex() const;

	// Mutators
	void							SetDefinitionIndex(uint8_t definitionIndex);


private:
	//-----Private Data-----

	uint8_t							m_payload[MESSAGE_MTU];

	uint8_t							m_definitionIndex = 0;

};
