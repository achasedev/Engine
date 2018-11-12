/************************************************************************/
/* File: NetMessage.cpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Implementation of the NetMessage class
/************************************************************************/
#include "Engine/Core/Time/Clock.hpp"
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
	m_reliableID = moveFrom.m_reliableID;

	m_sequenceID = moveFrom.m_sequenceID;
	m_sequenceChannelID = moveFrom.m_sequenceChannelID;

	memcpy(m_payload, moveFrom.m_payload, MESSAGE_MTU);

	// Invalidate
	moveFrom.m_buffer = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Copy constructor
//
NetMessage::NetMessage(const NetMessage& copy)
	: BytePacker(MESSAGE_MTU, m_payload, false, LITTLE_ENDIAN)
{
	m_bufferCapacity = copy.m_bufferCapacity;
	m_endianness = copy.m_endianness;
	m_readHead = copy.m_readHead;
	m_writeHead = copy.m_writeHead;
	m_ownsMemory = copy.m_ownsMemory;

	m_definition = copy.m_definition;
	m_reliableID = copy.m_reliableID;

	m_sequenceID = copy.m_sequenceID;
	m_sequenceChannelID = copy.m_sequenceChannelID;

	memcpy(m_payload, copy.m_payload, MESSAGE_MTU);
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
	m_reliableID = moveFrom.m_reliableID;

	m_sequenceID = moveFrom.m_sequenceID;
	m_sequenceChannelID = moveFrom.m_sequenceChannelID;

	memcpy(m_payload, moveFrom.m_payload, m_bufferCapacity);

	// Invalidate
	moveFrom.m_buffer = nullptr;

	return *this;
}


//-----------------------------------------------------------------------------------------------
// Copy override
//
NetMessage& NetMessage::operator=(const NetMessage& copy)
{
	m_bufferCapacity = copy.m_bufferCapacity;
	m_endianness = copy.m_endianness;
	m_readHead = copy.m_readHead;
	m_writeHead = copy.m_writeHead;
	m_ownsMemory = copy.m_ownsMemory;

	m_definition = copy.m_definition;
	m_reliableID = copy.m_reliableID;

	m_sequenceID = copy.m_sequenceID;
	m_sequenceChannelID = copy.m_sequenceChannelID;

	memcpy(m_payload, copy.m_payload, MESSAGE_MTU);

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
// Returns the time the packet was last sent
//
float NetMessage::GetLastSentTime() const
{
	return m_lastSentTime;
}


//-----------------------------------------------------------------------------------------------
// Returns true if this message requires a connection in order to be processed
//
bool NetMessage::RequiresConnection() const
{
	return ((m_definition->options & NET_MSG_OPTION_CONNECTIONLESS) != NET_MSG_OPTION_CONNECTIONLESS);
}


//-----------------------------------------------------------------------------------------------
// Returns true if this message is to be sent reliably
//
bool NetMessage::IsReliable() const
{
	return (m_definition->IsReliable());
}


//-----------------------------------------------------------------------------------------------
// Returns true if this message is to be sent in order
//
bool NetMessage::IsInOrder() const
{
	return (m_definition->IsInOrder());
}


//-----------------------------------------------------------------------------------------------
// Returns the size of the message header, depends on whether it is a reliable message or not
//
uint16_t NetMessage::GetHeaderSize() const
{
	uint16_t size = sizeof(uint8_t);

	if (IsReliable())
	{
		size += sizeof(uint16_t);

		if (IsInOrder())
		{
			size += (sizeof(uint16_t) + sizeof(uint8_t));
		}
	}

	return size;
}


uint16_t NetMessage::GetPayloadSize() const
{
	return (uint16_t) GetWrittenByteCount();
}

void NetMessage::ResetTimeLastSent()
{
	m_lastSentTime = Clock::GetMasterClock()->GetTotalSeconds();
}

void NetMessage::AssignReliableID(uint16_t reliableID)
{
	m_reliableID = reliableID;
}

void NetMessage::AssignSequenceID(uint16_t sequenceID)
{
	m_sequenceID = sequenceID;
}

void NetMessage::AssignSequenceChannelID(uint8_t channelID)
{
	m_sequenceChannelID = channelID;
}

uint16_t NetMessage::GetReliableID() const
{
	return m_reliableID;
}

uint16_t NetMessage::GetSequenceID() const
{
	return m_sequenceID;
}

uint8_t NetMessage::GetSequenceChannelID() const
{
	return m_sequenceChannelID;
}
