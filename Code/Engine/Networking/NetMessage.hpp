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
	std::string name;
	NetMessage_cb callback;
};


class NetMessage : public BytePacker
{
public:
	//-----Public Methods-----

	NetMessage();
	NetMessage(uint8_t definitionIndex, void* payload, const int16_t& payloadSize);
	~NetMessage();

	// Accessors
	uint8_t							GetDefinitionIndex() const;


private:
	//-----Private Data-----

	uint8_t							m_payload[MESSAGE_MTU];
	std::string						m_definitionName;
	uint8_t							m_definitionIndex;
	const NetMessageDefinition_t*	m_definition;

};
