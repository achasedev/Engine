/************************************************************************/
/* File: ShaderStorageBuffer.cpp
/* Author: Andrew Chase
/* Date: September 8th, 2018
/* Bugs: None
/* Description: Implementation of the ShaderStorageBuffer class
/************************************************************************/
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Rendering/OpenGL/glFunctions.hpp"
#include "Engine/Rendering/Buffers/ShaderStorageBuffer.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
ShaderStorageBuffer::ShaderStorageBuffer()
{
	glGenBuffers(1, &m_handle);
	GL_CHECK_ERROR();
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
ShaderStorageBuffer::~ShaderStorageBuffer()
{
	// cleanup for a buffer; 
	if (m_handle != NULL) {
		glDeleteBuffers(1, &m_handle);
		m_handle = NULL;
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the GPU handle to the buffer
//
GLuint ShaderStorageBuffer::GetHandle() const
{
	return m_handle;
}


//-----------------------------------------------------------------------------------------------
// Copies the given data to the buffer on the GPU
//
bool ShaderStorageBuffer::CopyToGPU(size_t const byte_count, void const *data)
{
	// Don't do anything if there's no data to copy
	if (data == nullptr || byte_count <= 0)
	{
		return false;
	}

	// In case we lose our handle somehow...
	if (m_handle == NULL) {
		glGenBuffers(1, &m_handle);
		GL_CHECK_ERROR();
	}

	// Bind the buffer to a slot, and copy memory
	// GL_DYNAMIC_DRAW means the memory is likely going to change a lot (we'll get
	// during the second project)
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_handle);
	glBufferData(GL_SHADER_STORAGE_BUFFER, byte_count, data, GL_DYNAMIC_DRAW);

	GL_CHECK_ERROR();

	// buffer_size is a size_t member variable I keep around for 
	// convenience
	m_bufferSize = byte_count;
	return true;
}


//-----------------------------------------------------------------------------------------------
// Returns a pointer to the head of the data on the GPU
//
void* ShaderStorageBuffer::GetBufferData()
{
	// Ensure this buffer is bound
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_handle);

	GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

	void* bufferData = glMapBuffer(GL_SHADER_STORAGE_BUFFER, bufMask);
	GL_CHECK_ERROR();

	return bufferData;
}
