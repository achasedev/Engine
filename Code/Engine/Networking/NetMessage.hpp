/************************************************************************/
/* File: NetMessage.hpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Class to represent a single message from a NetSession
/************************************************************************/
#pragma once
#include "Engine/Networking/BytePacker.hpp"
#include "Engine/Networking/NetAddress.hpp"
#include <string>

// Limit messages to 1KB
#define MESSAGE_MTU 1024

class BytePacker;
class NetConnection;
class NetMessage;
class NetSession;

struct NetMessageDefinition_t;


class NetMessage : public BytePacker
{
public:
	//-----Public Methods-----

	NetMessage();
	NetMessage(NetMessage&& moveFrom);
	NetMessage(const NetMessageDefinition_t* definition);
	NetMessage(const NetMessageDefinition_t* definition, void* payload, const int16_t& payloadSize, uint16_t reliableID = 0);
	NetMessage(const NetMessage& copy) = delete;
	~NetMessage();

	NetMessage& operator=(NetMessage&& moveFrom);
	NetMessage& operator=(const NetMessage&) = delete;

	// Accessors
	uint8_t							GetDefinitionID() const;
	const NetMessageDefinition_t*	GetDefinition() const;
	std::string						GetName() const;
	float							GetLastSentTime() const;
	uint16_t						GetReliableID() const;

	// Producers
	bool							RequiresConnection() const;
	bool							IsReliable() const;
	bool							IsInOrder() const;

	uint16_t						GetHeaderSize() const;
	uint16_t						GetPayloadSize() const;

	// Mutators
	void							ResetTimeLastSent();
	void							AssignReliableID(uint16_t reliableID);


private:
	//-----Private Data-----

	uint8_t							m_payload[MESSAGE_MTU];

	uint16_t						m_reliableID;
	float							m_lastSentTime;
	const NetMessageDefinition_t*	m_definition;

};
