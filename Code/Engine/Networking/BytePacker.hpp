#pragma once
#include "Engine/Networking/Endianness.hpp"
#include <stdint.h>

class BytePacker
{

public:
	//-----Public Methods-----

	BytePacker(eEndianness endianness = LITTLE_ENDIAN);
	BytePacker(size_t bufferSize, eEndianness endianness = LITTLE_ENDIAN);
	BytePacker(size_t bufferSize, void *buffer, eEndianness endianness = LITTLE_ENDIAN);
	~BytePacker();


	// sets how much of the buffer is readable; 
	void			SetEndianness(eEndianness);
	bool			SetReadableByteCount(size_t byteCount);

	// tries to write data to the end of the buffer;  Returns success
	bool			WriteBytes(size_t byteCount, void const *data);

	// Tries to read into out_data.  Returns how much
	// ended up being read; 
	size_t			ReadBytes(void *out_data, size_t maxByteCount);

	size_t			WriteSize(size_t size); // returns how many bytes used
	size_t			ReadSize(size_t *out_size); // returns how many bytes read, fills out_size

	// See notes on encoding!
	bool			WriteString(char const *str);
	size_t			ReadString(char *out_str, size_t maxByteSize); // max_str_size should be enough to contain the null terminator as well; 

	// HELPERS
	// suggested method names for commonly needed information; 
	void			ResetWrite();  // resets writing to the beginning of the buffer.  Make sure read head stays valid (<= write_head)
	void			ResetRead();   // resets reading to the beginning of the buffer

	eEndianness		GetEndianness() const;
	size_t			GetWrittenByteCount() const;				// how much have I written to this buffer
	size_t			GetRemainingWritableByteCount() const;		// how much more can I write to this buffer (if growble, this returns UINFINITY)
	size_t			GetRemainingReadableByteCount() const;		// how much more data can I read;


private:
	//-----Private Data----
	
	void* m_buffer = nullptr;
	size_t m_bufferSize;

	uint8_t* m_readHead = nullptr;
	uint8_t* m_writeHead = nullptr;

	size_t m_remainingReadableByteCount;
	size_t m_remainingWritableByteCount;

	eEndianness m_endianness;

};