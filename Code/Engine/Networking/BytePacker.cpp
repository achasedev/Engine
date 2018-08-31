#include "Engine/Networking/BytePacker.hpp"
#include <stdlib.h>

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
	m_readableByteCount = byteCount;

	return false;
}

bool BytePacker::WriteBytes(size_t byteCount, void const *data)
{
	return false;
}

size_t BytePacker::ReadBytes(void *out_data, size_t maxByteCount)
{
	return 0;
}

size_t BytePacker::WriteSize(size_t size)
{
	return 0;
}

