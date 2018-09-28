/************************************************************************/
/* File: NetPacket.cpp
/* Author: Andrew Chase
/* Date: September 28th, 2018
/* Description: Implementation of the NetPacket class
/************************************************************************/
#include "Engine/Networking/NetPacket.hpp"
#include "Engine/Networking/NetMessage.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
NetPacket::NetPacket()
	: BytePacker(PACKET_MTU, m_localBuffer, false, LITTLE_ENDIAN)
{
}


//-----------------------------------------------------------------------------------------------
// Writes a packet header to the start of the packet, then sets the write head to the previous
// position it was in
//
void NetPacket::WriteHeader(const PacketHeader_t& header)
{
	size_t writtenBytes = GetWrittenByteCount();

	// Set the write head back to the beginning to write
	ResetWrite();
	size_t headerSize = sizeof(PacketHeader_t);
	WriteBytes(headerSize, &header);

	// Move write head back to where it was
	if (writtenBytes > headerSize)
	{
		AdvanceWriteHead(writtenBytes - headerSize);
	}
}


//-----------------------------------------------------------------------------------------------
// Reads the header, returning it in out_header
//
bool NetPacket::ReadHeader(PacketHeader_t& out_header)
{
	bool success = ReadBytes(&out_header, sizeof(PacketHeader_t));
	return success;
}


//-----------------------------------------------------------------------------------------------
// Writes a message to the packet, including its header
//
bool NetPacket::WriteMessage(const NetMessage* message)
{
	bool success;

	// Write the header + message payload size
	int16_t msgSize = (int16_t) message->GetWrittenByteCount();
	uint16_t msgAndHeaderSize = msgSize + 1;
	success = WriteBytes(2, &msgAndHeaderSize);

	if (!success) 
	{ 
		return false; 
	}

	// Write the message index
	uint8_t msgIndex = message->GetDefinitionIndex();
	success = WriteBytes(sizeof(int8_t), &msgIndex);

	if (!success)
	{
		return false;
	}

	// Write the message payload
	success = WriteBytes(msgSize, message->GetBuffer());

	return success;
}


//-----------------------------------------------------------------------------------------------
// Reads the message and returns it in out_message
//
bool NetPacket::ReadMessage(NetMessage* out_message)
{
	// Read the header + message payload size
	int8_t headerSize = sizeof(int8_t);
	int16_t msgAndHeaderSize;
	size_t amountRead = ReadBytes(&msgAndHeaderSize, sizeof(int16_t));

	if (amountRead == 0)
	{
		return false;
	}

	// Read the message index
	int8_t msgIndex;
	amountRead = ReadBytes(&msgIndex, headerSize);

	if (amountRead == 0)
	{
		return false;
	}

	// Read the message payload
	int16_t msgSize = msgAndHeaderSize - headerSize;
	int8_t payload[MESSAGE_MTU];
	amountRead = ReadBytes(payload, msgSize);

	if (amountRead == 0)
	{
		return false;
	}

	// Construct the message
	*out_message = NetMessage(msgIndex, payload, msgSize);
	out_message->AdvanceWriteHead(msgSize);

	return true;
}


//-----------------------------------------------------------------------------------------------
// Sets the sender connection index of the packet to the one provided
//
void NetPacket::SetSenderConnectionIndex(uint8_t index)
{
	m_senderIndex = index;
}


//-----------------------------------------------------------------------------------------------
// Sets the receiver connection index of the packet to the one provided
//
void NetPacket::SetReceiverConnectionIndex(uint8_t index)
{
	m_receiverIndex = index;
}


//-----------------------------------------------------------------------------------------------
// Returns the sender connection index of the packet
//
uint8_t NetPacket::GetSenderConnectionIndex() const
{
	return m_senderIndex;
}


//-----------------------------------------------------------------------------------------------
// Returns the receiver connection index of the packet
//
uint8_t NetPacket::GetReceiverConnectionIndex() const
{
	return m_receiverIndex;
}
