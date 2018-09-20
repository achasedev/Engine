/************************************************************************/
/* File: NetMessage.hpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Class to represent a single message from a NetSession
/************************************************************************/
#pragma once
#include <string>
#include <vector>

class BytePacker;

class NetMessage
{
public:
	//-----Public Methods-----

	NetMessage();
	NetMessage(const std::string& tag);
	NetMessage(size_t size, void* data);

	~NetMessage();

	// Reading and writing to the byte packer
	template <typename T>
	void Write(const T& data)
	{
		Write(sizeof(data), &data)
	}

	template <typename T>
	size_t Read(T& out_data)
	{
		return Read(sizeof(out_data), &out_data);
	}

	void			Write(const size_t byteCount, void* data);
	size_t			Read(size_t const byteCount, void* out_data);
	void			ReadString(std::string& out_string);

	// Accessors
	size_t			GetSize() const;
	void*			GetData() const;

private:
	//-----Private Data-----

	BytePacker* m_buffer = nullptr;

};
