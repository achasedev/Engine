/************************************************************************/
/* File: NetPacket.cpp
/* Author: Andrew Chase
/* Date: September 28th, 2018
/* Description: Implementation of the NetPacket class
/************************************************************************/
#include "Engine/Networking/NetPacket.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetSession.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
NetPacket::NetPacket()
	: BytePacker(PACKET_MTU, m_localBuffer, false, LITTLE_ENDIAN)
{
}


//-----------------------------------------------------------------------------------------------
// Constructor, from a given buffer
//
NetPacket::NetPacket(uint8_t* buffer, size_t bufferSize)
	: BytePacker(PACKET_MTU, m_localBuffer, false, LITTLE_ENDIAN)
{
	memcpy(m_localBuffer, buffer, bufferSize);
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

	// Write each element separately
	Write(header.senderConnectionIndex);
	Write(header.packetAck);
	Write(header.highestReceivedAck);
	Write(header.receivedHistory);
	Write(header.totalMessageCount);

	// Move write head back to where it was
	if (writtenBytes > 8)
	{
		AdvanceWriteHead(writtenBytes - 8);
	}
}


//-----------------------------------------------------------------------------------------------
// Reads the header, returning it in out_header
//
bool NetPacket::ReadHeader(PacketHeader_t& out_header)
{
	size_t totalRead = 0;

	totalRead += Read(out_header.senderConnectionIndex);
	totalRead += Read(out_header.packetAck);
	totalRead += Read(out_header.highestReceivedAck);
	totalRead += Read(out_header.receivedHistory);
	totalRead += Read(out_header.totalMessageCount);

	return (totalRead == PACKET_HEADER_SIZE);
}


//-----------------------------------------------------------------------------------------------
// Writes a message to the packet, including its header
//
bool NetPacket::WriteMessage(const NetMessage* message)
{
	bool success;

	// Write the header + message payload size
	uint16_t msgHeaderSize = message->GetHeaderSize();
	uint16_t msgPayloadSize = message->GetPayloadSize();

	uint16_t totalSize = msgHeaderSize + msgPayloadSize;

	// Check size requirements
	if (totalSize > PACKET_MTU - GetWrittenByteCount())
	{
		return false;
	}

	success = WriteBytes(2, &totalSize);

	if (!success) 
	{ 
		return false; 
	}

	// Write the message index
	uint8_t msgIndex = message->GetDefinitionID();
	success = WriteBytes(sizeof(int8_t), &msgIndex);

	if (!success)
	{
		return false;
	}

	// Write the message payload
	success = WriteBytes(msgPayloadSize, message->GetBuffer());

	return success;
}


//-----------------------------------------------------------------------------------------------
// Reads the message and returns it in out_message
//
bool NetPacket::ReadMessage(NetMessage* out_message, NetSession* session)
{
	// Read the header + message payload size
	int16_t headerAndPayloadSize;
	size_t amountRead = ReadBytes(&headerAndPayloadSize, sizeof(int16_t));

	if (amountRead == 0)
	{
		return false;
	}

	// Read in the message header
	uint8_t msgHeaderSize = 1;

	// First the message index
	int8_t msgIndex;
	amountRead = Read(msgIndex);

	if (amountRead == 0)
	{
		return false;
	}

	const NetMessageDefinition_t* definition = session->GetMessageDefinition(msgIndex);

	// If the message is reliable, we need to get the reliable ID
	uint16_t reliableID = 0;
	if ((definition->options & NET_MSG_OPTION_RELIABLE) == NET_MSG_OPTION_RELIABLE)
	{
		amountRead = Read(reliableID);
		if (amountRead == 0)
		{
			return false;
		}

		// Add on the size of the reliable ID to the header size
		msgHeaderSize += sizeof(uint16_t);
	}

	// Read the message payload
	int16_t payloadSize = headerAndPayloadSize - msgHeaderSize;
	int8_t payload[MESSAGE_MTU];
	amountRead = ReadBytes(payload, payloadSize);

	// Construct the message
	*out_message = NetMessage(definition, payload, payloadSize, reliableID);
	out_message->AdvanceWriteHead(payloadSize);

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
