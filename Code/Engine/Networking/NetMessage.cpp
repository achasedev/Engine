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
	: BytePacker(MESSAGE_MTU, m_payload, LITTLE_ENDIAN)
{
}


//-----------------------------------------------------------------------------------------------
// Constructor - for reconstructing messages from a received payload
//
NetMessage::NetMessage(uint8_t definitionIndex, void* payload, const int16_t& payloadSize)
	: BytePacker(MESSAGE_MTU, m_payload, LITTLE_ENDIAN)
	, m_definitionIndex(definitionIndex)
{
	// Put the payload contents in
	memcpy(m_payload, payload, payloadSize);
}


//-----------------------------------------------------------------------------------------------
// Destructor - for cleaning up the BytePacker (which is why we pass NetMessages by pointer all the time,
// to avoid accidental deletion of this buffer)
//
NetMessage::~NetMessage()
{
	if (m_buffer != nullptr)
	{
		delete m_buffer;
		m_buffer = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the index in the NetSession for this message definition
//
uint8_t NetMessage::GetDefinitionIndex() const
{
	return m_definitionIndex;
}
