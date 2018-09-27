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
NetMessage::NetMessage(uint8_t definitionIndex, void* payload, const int16_t& payloadSize)
	: BytePacker(MESSAGE_MTU, m_payload, false, LITTLE_ENDIAN)
	, m_definitionIndex(definitionIndex)
{
	// Put the payload contents in
	memcpy(m_payload, payload, payloadSize);
}


//-----------------------------------------------------------------------------------------------
// Constructor for a NetMessage with a definition
//
NetMessage::NetMessage(uint8_t definitionIndex)
	: BytePacker(MESSAGE_MTU, m_payload, false, LITTLE_ENDIAN)
	, m_definitionIndex(definitionIndex)
{
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
	return m_definitionIndex;
}


//-----------------------------------------------------------------------------------------------
// Sets the definition members
//
void NetMessage::SetDefinitionIndex(uint8_t definitionIndex)
{
	m_definitionIndex = definitionIndex;
}
