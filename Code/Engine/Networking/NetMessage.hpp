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
	NetMessage(const NetMessageDefinition_t* definition, void* payload, const int16_t& payloadSize);
	NetMessage(const NetMessage& copy);
	~NetMessage();

	NetMessage& operator=(NetMessage&& moveFrom);
	NetMessage& operator=(const NetMessage&);

	// Accessors
	uint8_t							GetDefinitionID() const;
	const NetMessageDefinition_t*	GetDefinition() const;
	std::string						GetName() const;
	float							GetLastSentTime() const;
	uint16_t						GetReliableID() const;
	uint16_t						GetSequenceID() const;
	uint8_t							GetSequenceChannelID() const;

	// Producers
	bool							RequiresConnection() const;
	bool							IsReliable() const;
	bool							IsInOrder() const;

	uint16_t						GetHeaderSize() const;
	uint16_t						GetPayloadSize() const;

	// Mutators
	void							ResetTimeLastSent();
	void							AssignReliableID(uint16_t reliableID);
	void							AssignSequenceID(uint16_t sequenceID);
	void							AssignSequenceChannelID(uint8_t channelID);


private:
	//-----Private Data-----

	uint8_t							m_payload[MESSAGE_MTU];

	uint16_t						m_reliableID;

	uint16_t						m_sequenceID;
	uint8_t							m_sequenceChannelID = 0;

	float							m_lastSentTime;
	const NetMessageDefinition_t*	m_definition;

};
