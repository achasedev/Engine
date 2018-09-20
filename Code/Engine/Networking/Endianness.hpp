/************************************************************************/
/* File: Endianness.hpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Endianness utility functions
/************************************************************************/
#pragma once

// Two endianness
enum eEndianness
{
	LITTLE_ENDIAN = 0,
	BIG_ENDIAN
};

eEndianness		GetPlatformEndianness();
void			ToEndianness(const size_t byteSize, void* data, eEndianness endianness);
void			FromEndianness(const size_t byteSize, void* data,eEndianness endianness);
