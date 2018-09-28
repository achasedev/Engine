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

	m_definitionIndex = moveFrom.m_definitionIndex;
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

	m_definitionIndex = moveFrom.m_definitionIndex;
	memcpy(m_payload, moveFrom.m_payload, m_bufferCapacity);

	// Invalidate
	moveFrom.m_buffer = nullptr;

	return *this;
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
