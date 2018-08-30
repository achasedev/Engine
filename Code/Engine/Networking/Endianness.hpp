#pragma once


enum eEndianness
{
	LITTLE_ENDIAN = 0,
	BIG_ENDIAN
};

eEndianness GetPlatformEndianness();

void ToPlatformEndianness(const size_t byteSize, void* data, eEndianness endianness);
void FromPlatformEndianness(const size_t byteSize, void* data,eEndianness endianness);
