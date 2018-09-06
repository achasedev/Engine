#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Networking/BytePacker.hpp"
#include <stdlib.h>
#include <cstring>

BytePacker::BytePacker(eEndianness endianness /*= LITTLE_ENDIAN*/)
	: m_endianness(endianness)
{
}

BytePacker::BytePacker(size_t initialSize, eEndianness endianness /*= LITTLE_ENDIAN*/)
	: m_endianness(endianness)
	, m_bufferCapacity(initialSize)
{
	m_buffer = (uint8_t*)malloc(initialSize);
}

BytePacker::BytePacker(size_t initialSize, void *buffer, eEndianness endianness /*= LITTLE_ENDIAN*/)
	: m_bufferCapacity(initialSize)
	, m_endianness(endianness)
	, m_buffer((uint8_t*)buffer)
{
}

BytePacker::~BytePacker()
{
	if (m_buffer != nullptr)
	{
		free(m_buffer);
		m_buffer = nullptr;
	}
}

void BytePacker::SetEndianness(eEndianness endianness)
{
	m_endianness = endianness;
}


bool BytePacker::WriteBytes(size_t byteCount, void const *data)
{
	// Not enough room in the buffer, so keep expanding
	if (GetRemainingWritableByteCount() < byteCount)
	{
		ExpandBuffer(byteCount);
	}

	// Copy the data into the buffer
	memcpy((void*)(m_buffer + m_writeHead), data, byteCount);

	// Convert the new data to the endianness of the buffer
	ToEndianness(byteCount, (void*)(m_buffer + m_writeHead), m_endianness);

	// Advance the write head
	m_writeHead += byteCount;

	return true;
}

void* BytePacker::GetWriteHead()
{
	return (void*)(m_buffer + m_writeHead);
}

void BytePacker::AdvanceWriteHead(size_t byteCountToMove)
{
	m_writeHead += byteCountToMove;
}

size_t BytePacker::ReadBytes(void *out_data, size_t maxByteCount)
{
	// Get the data at the right head
	size_t amountRead = Peek(out_data, maxByteCount);

	// Advance the read head
	m_readHead += amountRead;

	return amountRead;
}

size_t BytePacker::Peek(void* out_data, size_t maxByteCount)
{
	// Read as much as we can - amount requested or the rest of the readable buffer
	size_t remainingReadableBytes = GetRemainingReadableByteCount();
	size_t amountToRead = (maxByteCount < remainingReadableBytes ? maxByteCount : remainingReadableBytes);

	// Copy the data out
	memcpy(out_data, (void*)(m_buffer + m_readHead), amountToRead);

	// Check the endianness
	FromEndianness(amountToRead, out_data, m_endianness);

	return amountToRead;
}

void BytePacker::AdvanceReadHead(size_t maxByteCount)
{
	size_t remainingReadableBytes = GetRemainingReadableByteCount();
	size_t amountToMove = (remainingReadableBytes < maxByteCount ? remainingReadableBytes : maxByteCount);

	m_readHead += amountToMove;
}

size_t BytePacker::WriteSize(size_t size)
{
	size_t bytesWritten = 0;
	size_t remainingValue = size;
	do 
	{
		uint8_t toWrite = remainingValue & 0x7F;
		remainingValue = remainingValue >> 7;

		if (remainingValue > 0)
		{
			toWrite |= 0x80;
		}

		bool success = WriteBytes(1, &toWrite);

		if (!success)
		{
			ERROR_AND_DIE("Error: BytePacker::WriteSize() exceeded max BytePacker size.");
		}

		bytesWritten += 1;

	} while (remainingValue > 0);
	 
	return bytesWritten;
}

size_t BytePacker::ReadSize(size_t *out_size)
{
	size_t bytesRead = 0;
	size_t total = 0;
	uint8_t valueRead;

	int iterationCount = 0;
	do
	{
		bytesRead += ReadBytes(&valueRead, 1);

		size_t addition = (valueRead & 0x7F);
		total |= addition << (7 * iterationCount);
		iterationCount++;

	} while ((valueRead & 0x80) != 0);

	*out_size = total;
	return bytesRead;
}

bool BytePacker::WriteString(const std::string& string)
{
	int characterCount = (int)string.size();
	WriteSize(characterCount);

	// Make sure the buffer has enough room
	if (GetRemainingWritableByteCount() < characterCount)
	{
		ExpandBuffer(characterCount);
	}

	// Copy the data into the buffer
	memcpy((void*)(m_buffer + m_writeHead), string.c_str(), characterCount);

	// Advance the write head
	m_writeHead += characterCount;

	return true;
}


size_t BytePacker::ReadString(std::string& out_string)
{
	size_t stringLength;
	ReadSize(&stringLength);

	// Copy the data out
	out_string = std::string((char*)(m_buffer + m_readHead), stringLength);

	// Advance the read head passed the full string to prevent errors
	m_readHead += stringLength;

	return stringLength;
}

void BytePacker::ResetWrite()
{
	m_writeHead = 0;
	ResetRead();
}

void BytePacker::ResetRead()
{
	m_readHead = 0;
}

eEndianness BytePacker::GetEndianness() const
{
	return m_endianness;
}

size_t BytePacker::GetWrittenByteCount() const
{
	return m_writeHead;
}

size_t BytePacker::GetRemainingWritableByteCount() const
{
	return (m_bufferCapacity - m_writeHead);
}

size_t BytePacker::GetRemainingReadableByteCount() const
{
	return (m_writeHead - m_readHead);
}

void* BytePacker::GetBuffer()
{
	return m_buffer;
}

bool BytePacker::Reserve(size_t requestedSize)
{
	if (m_bufferCapacity >= requestedSize)
	{
		return false;
	}

	return ExpandBuffer(requestedSize - m_bufferCapacity);
}

bool BytePacker::ExpandBuffer(size_t requestedAddition)
{	
	size_t amountToAdd = (requestedAddition > m_bufferCapacity ? requestedAddition : m_bufferCapacity);

	void* newBuffer = malloc(m_bufferCapacity + amountToAdd);

	memcpy(newBuffer, m_buffer, m_bufferCapacity);

	free(m_buffer);
	m_buffer = (uint8_t*)newBuffer;
	m_bufferCapacity += amountToAdd;

	return true;
}
