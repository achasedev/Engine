/************************************************************************/
/* File: NetMessage.cpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Implementation of the NetMessage class
/************************************************************************/
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetSession.hpp"

//-----------------------------------------------------------------------------------------------
// Default constructor
//
NetMessage::NetMessage()
	: BytePacker(MESSAGE_MTU, m_payload, false, LITTLE_ENDIAN)
{
}


//-----------------------------------------------------------------------------------------------
// Constructor - for reconstructing messages from a received payload
//
NetMessage::NetMessage(const NetMessageDefinition_t* definition, void* payload, const int16_t& payloadSize)
	: BytePacker(MESSAGE_MTU, m_payload, false, LITTLE_ENDIAN)
	, m_definition(definition)
{
	// Put the payload contents in
	memcpy(m_payload, payload, payloadSize);
}


//-----------------------------------------------------------------------------------------------
// Constructor for a NetMessage with a definition
//
NetMessage::NetMessage(const NetMessageDefinition_t* definition)
	: BytePacker(MESSAGE_MTU, m_payload, false, LITTLE_ENDIAN)
	, m_definition(definition)
{
}


//-----------------------------------------------------------------------------------------------
// Move constructor
//
NetMessage::NetMessage(NetMessage&& moveFrom)
	: BytePacker(MESSAGE_MTU, m_payload, false, LITTLE_ENDIAN)
{
	m_bufferCapacity = moveFrom.m_bufferCapacity;
	m_endianness = moveFrom.m_endianness;
	m_readHead = moveFrom.m_readHead;
	m_writeHead = moveFrom.m_writeHead;
	m_ownsMemory = moveFrom.m_ownsMemory;

	m_definition = moveFrom.m_definition;
	memcpy(m_payload, moveFrom.m_payload, MESSAGE_MTU);

	// Invalidate
	moveFrom.m_buffer = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Destructor - for cleaning up the BytePacker
//
NetMessage::~NetMessage()
{
}


//-----------------------------------------------------------------------------------------------
// Move override
//
NetMessage& NetMessage::operator=(NetMessage&& moveFrom)
{
	m_bufferCapacity = moveFrom.m_bufferCapacity;
	m_endianness = moveFrom.m_endianness;
	m_readHead = moveFrom.m_readHead;
	m_writeHead = moveFrom.m_writeHead;
	m_ownsMemory = moveFrom.m_ownsMemory;

	m_definition = moveFrom.m_definition;
	memcpy(m_payload, moveFrom.m_payload, m_bufferCapacity);

	// Invalidate
	moveFrom.m_buffer = nullptr;

	return *this;
}


//-----------------------------------------------------------------------------------------------
// Returns the index in the NetSession for this message definition
//
uint8_t NetMessage::GetDefinitionID() const
{
	return m_definition->id;
}


//-----------------------------------------------------------------------------------------------
// Returns the definition of this message
//
const NetMessageDefinition_t* NetMessage::GetDefinition() const
{
	return m_definition;
}


//-----------------------------------------------------------------------------------------------
// Returns the name of the definition for this message
//
std::string NetMessage::GetName() const
{
	return m_definition->name;
}


//-----------------------------------------------------------------------------------------------
// Returns true if this message requires a connection in order to be processed
//
bool NetMessage::RequiresConnection() const
{
	return ((m_definition->options & NET_MSG_OPTION_CONNECTIONLESS) != NET_MSG_OPTION_CONNECTIONLESS);
}
