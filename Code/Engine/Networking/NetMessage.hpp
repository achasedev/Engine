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

// Callback for the NetSession
typedef bool(*NetMessage_cb)(NetMessage* msg, NetConnection* sender);

struct NetMessageDefinition_t
{
	NetMessageDefinition_t(const std::string& _name, NetMessage_cb _callback)
		: name(_name), callback(_callback) {}

	std::string		name;
	uint8_t			sessionIndex;
	NetMessage_cb	callback;
};


class NetMessage : public BytePacker
{
public:
	//-----Public Methods-----

	NetMessage();
	NetMessage(const NetMessageDefinition_t* definition, void* payload, const int16_t& payloadSize);
	~NetMessage();

	// Accessors
	uint8_t							GetDefinitionIndex() const;

	// Mutators
	void							SetDefinition(const NetMessageDefinition_t* definition);


private:
	//-----Private Data-----

	uint8_t							m_payload[MESSAGE_MTU];

	const NetMessageDefinition_t*	m_definition;

};
