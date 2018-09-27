#include "Engine/Networking/NetPacket.hpp"
#include "Engine/Networking/NetMessage.hpp"

NetPacket::NetPacket()
	: BytePacker(PACKET_MTU, m_localBuffer, false, LITTLE_ENDIAN)
{
}

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

bool NetPacket::ReadHeader(PacketHeader_t& out_header)
{
	bool success = ReadBytes(&out_header, sizeof(PacketHeader_t));
	return success;
}

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
	amountRead = ReadBytes(payload, MESSAGE_MTU);

	if (amountRead == 0)
	{
		return false;
	}

	// Construct the message
	*out_message = NetMessage(msgIndex, &payload, msgSize);
	out_message->AdvanceWriteHead(msgSize);
	out_message->AdvanceReadHead(1);

	return true;
}

uint8_t NetPacket::GetConnectionIndex() const
{
	return m_sendReceiveIndex;
}

