#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Networking/BytePacker.hpp"
#include <stdlib.h>
#include <cstring>

BytePacker::BytePacker(eEndianness endianness /*= LITTLE_ENDIAN*/)
	: m_endianness(endianness)
{
}

BytePacker::BytePacker(size_t bufferSize, eEndianness endianness /*= LITTLE_ENDIAN*/)
	: m_endianness(endianness)
	, m_bufferSize(bufferSize)
{
	m_buffer = malloc(bufferSize);
}

BytePacker::BytePacker(size_t bufferSize, void *buffer, eEndianness endianness /*= LITTLE_ENDIAN*/)
	: m_bufferSize(bufferSize)
	, m_endianness(endianness)
	, m_buffer(buffer)
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

bool BytePacker::SetReadableByteCount(size_t byteCount)
{
	m_remainingReadableByteCount = byteCount;
}

bool BytePacker::WriteBytes(size_t byteCount, void const *data)
{
	// Not enough room in the buffer
	if (GetRemainingWritableByteCount() < byteCount)
	{
		return false;
	}

	// Copy the data into the buffer
	memcpy((void*)m_writeHead, data, byteCount);

	// Convert the new data to the endianness of the buffer
	ToEndianness(byteCount, (void*)m_writeHead, m_endianness);

	// Advance the write head
	m_writeHead += byteCount;
}

size_t BytePacker::ReadBytes(void *out_data, size_t maxByteCount)
{
	// Read as much as we can - amount requested or the rest of the readable buffer
	size_t amountToRead = (maxByteCount < m_remainingReadableByteCount ? maxByteCount : m_remainingReadableByteCount);

	// Copy the data out
	memcpy(out_data, (void*)m_readHead, amountToRead);

	// Check the endianness
	FromEndianness(amountToRead, out_data, m_endianness);

	// Advance the read head
	m_readHead += amountToRead;

	return amountToRead;
}

size_t BytePacker::WriteSize(size_t size)
{
	size_t bytesWritten;
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
			ERROR_AND_DIE("Error: BytePacker::WriteSize exceeded buffer size.");
		}

		bytesWritten += 1;

	} while (remainingValue > 0);
	 
	return bytesWritten;
}

size_t BytePacker::ReadSize(size_t *out_size)
{
	size_t bytesRead;
	size_t total = 0;
	uint8_t valueRead;

	do
	{
		bytesRead += ReadBytes(&valueRead, 1);

		total += (valueRead & 0x7F);

	} while ((valueRead & 0x80) != 0);

	*out_size = total;
	return bytesRead;
}

bool BytePacker::WriteString(char const *str)
{
	int characterCount = GetStringLength(str);
	WriteSize(characterCount);

	// Not enough room in the buffer
	if (GetRemainingWritableByteCount() < characterCount)
	{
		return false;
	}

	// Copy the data into the buffer
	memcpy((void*)m_writeHead, str, characterCount);

	// Advance the write head
	m_writeHead += characterCount;

	return true;
}

size_t BytePacker::ReadString(char *out_str, size_t maxByteSize)
{
	size_t stringLength;
	ReadSize(&stringLength);

	size_t countToRead = (stringLength < maxByteSize - 1 ? stringLength : maxByteSize - 1);

	// Copy the data out
	memcpy(out_str, (void*)m_readHead, countToRead);

	// Advance the read head passed the full string
	m_readHead += stringLength;

	return countToRead;
}

void BytePacker::ResetWrite()
{
	m_writeHead = (uint8_t*)m_buffer;
	ResetRead();
}

void BytePacker::ResetRead()
{
	m_readHead = (uint8_t*)m_buffer;
}

eEndianness BytePacker::GetEndianness() const
{
	return m_endianness;
}

size_t BytePacker::GetWrittenByteCount() const
{
	return (m_writeHead - (uint8_t*)m_buffer);
}

size_t BytePacker::GetRemainingWritableByteCount() const
{
	return (m_bufferSize - GetWrittenByteCount());
}

size_t BytePacker::GetRemainingReadableByteCount() const
{
	return (m_readHead - (uint8_t*)m_buffer);
}

