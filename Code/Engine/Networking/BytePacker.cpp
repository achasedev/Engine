/************************************************************************/
/* File: BytePacker.cpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Implementation of the BytePacker class
/************************************************************************/
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Networking/BytePacker.hpp"
#include <stdlib.h>
#include <cstring>

//-----------------------------------------------------------------------------------------------
// Constructor
//
BytePacker::BytePacker(eEndianness endianness /*= LITTLE_ENDIAN*/)
	: m_endianness(endianness)
{
}


//-----------------------------------------------------------------------------------------------
// Constructor that initializes the buffer to the given size
//
BytePacker::BytePacker(size_t initialSize, eEndianness endianness /*= LITTLE_ENDIAN*/)
	: m_endianness(endianness)
	, m_bufferCapacity(initialSize)
{
	m_buffer = (uint8_t*)malloc(initialSize);
}


//-----------------------------------------------------------------------------------------------
// Constructor, from a given buffer
//
BytePacker::BytePacker(size_t initialSize, void *buffer, eEndianness endianness /*= LITTLE_ENDIAN*/)
	: m_bufferCapacity(initialSize)
	, m_endianness(endianness)
	, m_buffer((uint8_t*)buffer)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
BytePacker::~BytePacker()
{
	if (m_buffer != nullptr)
	{
		free(m_buffer);
		m_buffer = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the endianness of the bytepacker for future reads and writes
//
void BytePacker::SetEndianness(eEndianness endianness)
{
	m_endianness = endianness;
}


//-----------------------------------------------------------------------------------------------
// Writes the given data to the buffer, checking for endianness switching
//
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


//-----------------------------------------------------------------------------------------------
// Returns a pointer to the write location of the buffer
//
void* BytePacker::GetWriteHead()
{
	return (void*)(m_buffer + m_writeHead);
}


//-----------------------------------------------------------------------------------------------
// Moves the writehead forward by byteCountToMove
//
void BytePacker::AdvanceWriteHead(size_t byteCountToMove)
{
	m_writeHead += byteCountToMove;
}


//-----------------------------------------------------------------------------------------------
// Reads the bytes from the buffer at the read head and returns it in out_data
// Advances the readhead by the amount read
//
size_t BytePacker::ReadBytes(void *out_data, size_t maxByteCount)
{
	// Get the data at the right head
	size_t amountRead = Peek(out_data, maxByteCount);

	// Advance the read head
	m_readHead += amountRead;

	return amountRead;
}


//-----------------------------------------------------------------------------------------------
// Reads the bytes from the buffer at the read head and returns it in out_data
//
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


//-----------------------------------------------------------------------------------------------
// Moves the read head forward by maxByteCount, clamping to the write head
//
void BytePacker::AdvanceReadHead(size_t maxByteCount)
{
	size_t remainingReadableBytes = GetRemainingReadableByteCount();
	size_t amountToMove = (remainingReadableBytes < maxByteCount ? remainingReadableBytes : maxByteCount);

	m_readHead += amountToMove;
}


//-----------------------------------------------------------------------------------------------
// Writes an encoded size to the buffer at the write head location
//
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


//-----------------------------------------------------------------------------------------------
// Reads an encoded size into out_size, at the location of the read head
//
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


//-----------------------------------------------------------------------------------------------
// Writes the given string to the buffer, returning true if it writes successfully
//
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


//-----------------------------------------------------------------------------------------------
// Reads a string located at the read head into out_string
//
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


//-----------------------------------------------------------------------------------------------
// Resets the write (and read) head to the start of the buffer
//
void BytePacker::ResetWrite()
{
	m_writeHead = 0;
	ResetRead();
}


//-----------------------------------------------------------------------------------------------
// Resets the read head only to the start of the buffer
//
void BytePacker::ResetRead()
{
	m_readHead = 0;
}


//-----------------------------------------------------------------------------------------------
// Returns the endianness of the buffer
//
eEndianness BytePacker::GetEndianness() const
{
	return m_endianness;
}


//-----------------------------------------------------------------------------------------------
// Returns the number of bytes currently written to the buffer
//
size_t BytePacker::GetWrittenByteCount() const
{
	return m_writeHead;
}


//-----------------------------------------------------------------------------------------------
// Returns the number of bytes that can be written to the buffer still (without expanding)
//
size_t BytePacker::GetRemainingWritableByteCount() const
{
	return (m_bufferCapacity - m_writeHead);
}


//-----------------------------------------------------------------------------------------------
// Returns the number of bytes still readable in the buffer
//
size_t BytePacker::GetRemainingReadableByteCount() const
{
	return (m_writeHead - m_readHead);
}


//-----------------------------------------------------------------------------------------------
// Returns the pointer to the buffer
//
void* BytePacker::GetBuffer()
{
	return m_buffer;
}


//-----------------------------------------------------------------------------------------------
// Expands the buffer to fit the requestedSize at minimum (may expand to larget size)
//
bool BytePacker::Reserve(size_t requestedSize)
{
	if (m_bufferCapacity >= requestedSize)
	{
		return false;
	}

	return ExpandBuffer(requestedSize - m_bufferCapacity);
}


//-----------------------------------------------------------------------------------------------
// Expands the current buffer to hold the additional size passed
//
bool BytePacker::ExpandBuffer(size_t requestedAddition)
{	
	// Double the buffer size or expand to hold the addition, whichever is greater
	size_t amountToAdd = (requestedAddition > m_bufferCapacity ? requestedAddition : m_bufferCapacity);

	void* newBuffer = malloc(m_bufferCapacity + amountToAdd);

	memcpy(newBuffer, m_buffer, m_bufferCapacity);

	free(m_buffer);
	m_buffer = (uint8_t*)newBuffer;
	m_bufferCapacity += amountToAdd;

	return true;
}
