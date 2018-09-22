/************************************************************************/
/* File: BytePacker.hpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Class to represent a byte packed buffer
/************************************************************************/
#pragma once
#include "Engine/Networking/Endianness.hpp"
#include <string>
#include <stdint.h>

class BytePacker
{

public:
	//-----Public Methods-----

	BytePacker(eEndianness endianness = LITTLE_ENDIAN);
	BytePacker(size_t bufferSize, bool ownsMemory, eEndianness endianness = LITTLE_ENDIAN);
	BytePacker(size_t bufferSize, void *buffer, bool ownsMemory, eEndianness endianness = LITTLE_ENDIAN);
	virtual ~BytePacker();


	// sets how much of the buffer is readable; 
	void			SetEndianness(eEndianness);

	// tries to write data to the end of the buffer;  Returns success
	bool			WriteBytes(size_t byteCount, void const *data);
	void*			GetWriteHead();
	void			AdvanceWriteHead(size_t byteCountToMove);

	template <typename T>
	bool Write(const T& data)
	{
		return WriteBytes(sizeof(T), &data);
	}

	// Tries to read into out_data.  Returns how much
	// ended up being read; 
	size_t			ReadBytes(void *out_data, size_t maxByteCount);
	size_t			Peek(void* out_data, size_t maxByteCount);
	void			AdvanceReadHead(size_t maxByteCount);

	template <typename T>
	bool Read(T& out_data)
	{
		return Read(&out_data, sizeof(T));
	}

	// Size encoding
	size_t			WriteSize(size_t size); // returns how many bytes used
	size_t			ReadSize(size_t *out_size); // returns how many bytes read, fills out_size

	// See notes on encoding!
	bool			WriteString(const std::string& string);
	size_t			ReadString(std::string& out_string); // max_str_size should be enough to contain the null terminator as well; 

	// HELPERS
	void			ResetWrite();  // resets writing to the beginning of the buffer.  Make sure read head stays valid (<= write_head)
	void			ResetRead();   // resets reading to the beginning of the buffer

	eEndianness		GetEndianness() const;
	size_t			GetWrittenByteCount() const;				// how much have I written to this buffer
	size_t			GetRemainingWritableByteCount() const;		// how much more can I write to this buffer (if growble, this returns UINFINITY)
	size_t			GetRemainingReadableByteCount() const;		// how much more data can I read;

	const void*		GetBuffer() const;

	bool			Reserve(size_t requestedSize);
	bool			ExpandBuffer(size_t requestedAddition);


protected:
	//-----Protected Data----
	
	uint8_t*		m_buffer = nullptr;
	size_t			m_bufferCapacity;
	bool			m_ownsMemory = true;

	size_t			m_readHead;
	size_t			m_writeHead;

	eEndianness		m_endianness;

};
