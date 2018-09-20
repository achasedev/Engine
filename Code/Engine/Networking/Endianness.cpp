/************************************************************************/
/* File: Endianness.cpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Implementation of the Endianness utility functions
/************************************************************************/
#include "Engine/Networking/Endianness.hpp"
#include <stdint.h>


//-----------------------------------------------------------------------------------------------
// Returns the native endianness of this machine
//
eEndianness GetPlatformEndianness()
{
	int test = 1;
	uint8_t* buffer = (uint8_t*)(&test);

	if (buffer[0] == 0x01)
	{
		return LITTLE_ENDIAN;
	}
	else
	{
		return BIG_ENDIAN;
	}
}


//-----------------------------------------------------------------------------------------------
// Converts the given data to the endianness specified
// *assumes the passed data is already in LITTLE_ENDIAN*
//
void ToEndianness(const size_t byteSize, void* data, eEndianness endianness)
{
	if (endianness == GetPlatformEndianness())
	{
		return;
	}

	size_t i = 0;
	size_t j = byteSize - 1;

	uint8_t* byteBuffer = (uint8_t*)data;

	while (i < j)
	{
		uint8_t temp = byteBuffer[i];
		byteBuffer[i] = byteBuffer[j];
		byteBuffer[j] = temp;

		i++;
		j--;
	}
}


//-----------------------------------------------------------------------------------------------
// Converts the data from the given endianness to the platform endianness
//
void FromEndianness(const size_t byteSize, void* data, eEndianness endianness)
{
	if (endianness == GetPlatformEndianness())
	{
		return;
	}

	size_t i = 0;
	size_t j = byteSize - 1;

	uint8_t* byteBuffer = (uint8_t*)data;

	while (i < j)
	{
		uint8_t temp = byteBuffer[i];
		byteBuffer[i] = byteBuffer[j];
		byteBuffer[j] = temp;

		i++;
		j--;
	}
}
