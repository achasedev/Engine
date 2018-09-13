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
	if (byte_count <= 0)
	{
		return false;
	}

	// In case we lose our handle somehow...
	if (m_handle == NULL) {
		glGenBuffers(1, &m_handle);
		GL_CHECK_ERROR();
	}

	// Bind the buffer to a slot, and copy memory
	// GL_DYNAMIC_DRAW means the memory is likely going to change a lot
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_handle);
	glBufferData(GL_SHADER_STORAGE_BUFFER, byte_count, data, GL_DYNAMIC_DRAW);

	GL_CHECK_ERROR();

	// buffer_size is a size_t member variable I keep around for 
	// convenience
	m_bufferSize = byte_count;
	return true;
}


//-----------------------------------------------------------------------------------------------
// Clears all the data in the buffer to 0's
//
bool ShaderStorageBuffer::Clear(size_t const byte_count)
{
	// Avoid putting too much on the stack
	if (byte_count <= (1024 * 16))
	{
		void* data = alloca(byte_count);
		memset(data, 0, byte_count);

		return CopyToGPU(byte_count, data);
	}

	void* data = malloc(byte_count);
	memset(data, 0, byte_count);

	bool success = CopyToGPU(byte_count, data);
	free(data);

	return success;
}


//-----------------------------------------------------------------------------------------------
// Binds the given buffer to the slot indicated
//
void ShaderStorageBuffer::Bind(int bindSlot)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, (GLuint)bindSlot, (GLuint)m_handle);
}


//-----------------------------------------------------------------------------------------------
// Returns a pointer to the head of the data on the GPU
//
void* ShaderStorageBuffer::MapBufferData()
{
	// Ensure this buffer is bound
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_handle);

	void* bufferData = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
	GL_CHECK_ERROR();

	return bufferData;
}

void ShaderStorageBuffer::UnmapBufferData()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_handle);

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	GL_CHECK_ERROR();
}