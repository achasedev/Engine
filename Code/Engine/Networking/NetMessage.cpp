/************************************************************************/
/* File: NetMessage.cpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Implementation of the NetMessage class
/************************************************************************/
#include "Engine/Networking/NetMessage.hpp"

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
// Destructor - for cleaning up the BytePacker
//
NetMessage::~NetMessage()
{
}


//-----------------------------------------------------------------------------------------------
// Returns the index in the NetSession for this message definition
//
uint8_t NetMessage::GetDefinitionIndex() const
{
	return m_definition->sessionIndex;
}


//-----------------------------------------------------------------------------------------------
// Sets the definition members
//
void NetMessage::SetDefinition(const NetMessageDefinition_t* definition)
{
	m_definition = definition;
}
