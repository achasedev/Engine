/************************************************************************/
/* File: IndexBuffer.cpp
/* Author: Andrew Chase
/* Date: March 25th, 2018
/* Description: Implementation of the IndexBuffer class
/************************************************************************/
#include "Engine/Renderer/IndexBuffer.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
IndexBuffer::IndexBuffer()
	: m_indexCount(0)
	, m_indexStride(sizeof(unsigned int))
{
}


//-----------------------------------------------------------------------------------------------
// Copies the index data to the GPU
//
bool IndexBuffer::CopyToGPU(int indexCount, const unsigned int* indices)
{
	size_t byteCount = indexCount * m_indexStride;
	bool succeeded = RenderBuffer::CopyToGPU(byteCount, (const void*)indices, GL_ELEMENT_ARRAY_BUFFER);

	// Only update if data was copied
	if (succeeded)
	{
		m_indexCount = indexCount;
	}

	return succeeded;
}
