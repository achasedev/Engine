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
typedef bool(*NetMessage_cb)(NetMessage* msg, const NetSender_t& sender);

struct NetMessageDefinition_t
{
	NetMessageDefinition_t(uint8_t _id, const std::string& _name, NetMessage_cb _callback, eNetMessageOption _options)
		: id(_id), name(_name), callback(_callback), options(_options) {}

	uint8_t				id;
	std::string			name = "";
	NetMessage_cb		callback = nullptr;
	eNetMessageOption	options;
};

class NetMessage : public BytePacker
{
public:
	//-----Public Methods-----

	NetMessage();
	NetMessage(NetMessage&& moveFrom);
	NetMessage(const NetMessageDefinition_t* definition);
	NetMessage(const NetMessageDefinition_t* definition, void* payload, const int16_t& payloadSize);
	NetMessage(const NetMessage& copy) = delete;
	~NetMessage();

	NetMessage& operator=(NetMessage&& moveFrom);
	NetMessage& operator=(const NetMessage&) = delete;

	// Accessors
	uint8_t							GetDefinitionID() const;
	const NetMessageDefinition_t*	GetDefinition() const;

	// Producers
	bool							RequiresConnection() const;
	bool							IsReliable() const;
	bool							IsInOrder() const;


private:
	//-----Private Data-----

	uint8_t							m_payload[MESSAGE_MTU];

	const NetMessageDefinition_t*	m_definition;

};
