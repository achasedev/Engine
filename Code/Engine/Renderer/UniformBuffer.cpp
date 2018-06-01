/************************************************************************/
/* File: UniformBuffer.cpp
/* Author: Andrew Chase
/* Date: April 4th, 2017
/* Description: Implementation of the UniformBuffer class
/************************************************************************/
#include <cstring>
#include <stdlib.h>
#include "Engine/Renderer/UniformBuffer.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
UniformBuffer::UniformBuffer()
	: m_isCPUDirty(false)
	, m_cpuBuffer(nullptr)
	, m_bufferSize(0)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
UniformBuffer::~UniformBuffer()
{
	// Clean up CPU buffer
	if (m_cpuBuffer != nullptr)
	{
		free(m_cpuBuffer);
		m_cpuBuffer = nullptr;
	}

	// GPU buffer should be cleaned up on destruction
}


//-----------------------------------------------------------------------------------------------
// Returns the GPU handle of the UniformBuffer's GPU-side buffer
//
GLuint UniformBuffer::GetHandle() const
{
	return m_gpuBuffer.GetHandle();
}


//-----------------------------------------------------------------------------------------------
// Allocates memory for the CPU buffer
//
void UniformBuffer::InitializeCPUBuffer(size_t byteSize)
{
	if (byteSize == 0) { return; }

	if (m_cpuBuffer != nullptr)
	{
		free(m_cpuBuffer);
	}

	m_cpuBuffer = malloc(byteSize);

	memset(m_cpuBuffer, 0, byteSize);

	m_bufferSize = byteSize;
	m_isCPUDirty = true;
}


//-----------------------------------------------------------------------------------------------
// Copies the given data into the CPU-side buffer, does not push to the GPU
//
void UniformBuffer::SetCPUData(size_t byteSize, const void* data)
{
	if (data == nullptr && byteSize == 0) { return; }

	if (m_cpuBuffer == nullptr)
	{
		m_cpuBuffer = malloc(byteSize);
	}
	else if (m_bufferSize != byteSize)
	{
		free(m_cpuBuffer);
		m_cpuBuffer = malloc(byteSize);
	}

	// Copy the data into the cpu buffer
	std::memcpy(m_cpuBuffer, data, byteSize);
	
	m_bufferSize = byteSize;
	m_isCPUDirty = true;
}


//-----------------------------------------------------------------------------------------------
// Sets the data at the given offset and size in the cpu data to the data specified
//
void UniformBuffer::UpdateCPUData(size_t byteOffset, size_t byteSize, const void* data)
{
	if (data == nullptr && byteSize == 0) { return; }
	size_t requestedLimit = byteOffset + byteSize;

	if (m_cpuBuffer == nullptr)
	{
		m_cpuBuffer = malloc(requestedLimit);
		m_bufferSize = requestedLimit;
	}
	// If the offset plus the size of the data added exceeds the size of this buffer, expand
	else if (requestedLimit > m_bufferSize)
	{
		void* temp = malloc(requestedLimit);			// Make a new buffer that is large enough
		std::memcpy(temp, m_cpuBuffer, m_bufferSize);	// Copy the old data in

		free(m_cpuBuffer);		// Free old data
		m_cpuBuffer = temp;
		m_bufferSize = requestedLimit; // Update size
	}

	unsigned char* destination = (unsigned char*)m_cpuBuffer;	// Need a byte pointer to the cpu buffer

	std::memcpy(&destination[byteOffset], data, byteSize);
	m_isCPUDirty = true;
}


//-----------------------------------------------------------------------------------------------
// Copies the data into the CPU-side buffer and pushes it to the GPU buffer
//
void UniformBuffer::SetCPUAndGPUData(size_t byteSize, const void* data)
{
	if (data == nullptr || byteSize == 0) { return; }

	// Set the CPU buffer
	SetCPUData(byteSize, data);

	// Copy it to the GPU
	CheckAndUpdateGPUData();
}


//-----------------------------------------------------------------------------------------------
// Pushes the CPU-side buffer data to the GPU if it is dirty, does nothing otherwise
//
void UniformBuffer::CheckAndUpdateGPUData()
{
	if (m_isCPUDirty)
	{
		m_gpuBuffer.CopyToGPU(m_bufferSize, m_cpuBuffer);
		m_isCPUDirty = false;
	}
}


//-----------------------------------------------------------------------------------------------
// Returns a mutable pointer to the CPU-side buffer
//
void* UniformBuffer::GetCPUBuffer()
{
	m_isCPUDirty = true;
	return m_cpuBuffer;
}


//-----------------------------------------------------------------------------------------------
// Returns a constant pointer to the CPU-side buffer
//
const void* UniformBuffer::GetConstCPUBuffer() const
{
	return m_cpuBuffer;
}


//-----------------------------------------------------------------------------------------------
// Returns the current size of the CPU buffer
//
size_t UniformBuffer::GetByteSize() const
{
	return m_bufferSize;
}
