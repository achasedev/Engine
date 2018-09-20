/************************************************************************/
/* File: NetMessage.cpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Implementation of the NetMessage class
/************************************************************************/
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/BytePacker.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor - writes the tag to the buffer
//
NetMessage::NetMessage(const std::string& tag)
{
	m_buffer = new BytePacker(LITTLE_ENDIAN);

	// Write the tag to the start of the buffer
	m_buffer->WriteString(tag);
}


//-----------------------------------------------------------------------------------------------
// Default constructor
//
NetMessage::NetMessage()
{
	m_buffer = new BytePacker(LITTLE_ENDIAN);
}


//-----------------------------------------------------------------------------------------------
// Constructor - for receiving off of a connection
//
NetMessage::NetMessage(size_t size, void* data)
{
	m_buffer = new BytePacker(size, data, LITTLE_ENDIAN);
}


//-----------------------------------------------------------------------------------------------
// Destructor - for cleaning up the BytePacker (which is why we pass NetMessages by pointer all the time,
// to avoid accidental deletion of this buffer)
//
NetMessage::~NetMessage()
{
	if (m_buffer != nullptr)
	{
		delete m_buffer;
		m_buffer = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Reads the next byteCount bytes from the BytePacker, returning the amount written
//
size_t NetMessage::Read(size_t const byteCount, void* out_data)
{
	return m_buffer->ReadBytes(out_data, byteCount);
}


//-----------------------------------------------------------------------------------------------
// Writes the data to the BytePacker
//
void NetMessage::Write(const size_t byteCount, void* data)
{
	m_buffer->WriteBytes(byteCount, data);
}


//-----------------------------------------------------------------------------------------------
// Reads a string from the BytePacker
//
void NetMessage::ReadString(std::string& out_string)
{
	m_buffer->ReadString(out_string);
}


//-----------------------------------------------------------------------------------------------
// Returns the current working size of the message (written byte count on the packer)
//
size_t NetMessage::GetSize() const
{
	return m_buffer->GetWrittenByteCount();
}


//-----------------------------------------------------------------------------------------------
// Returns a pointer to the BytePacker data
//
void* NetMessage::GetData() const
{
	return m_buffer->GetBuffer();
}
