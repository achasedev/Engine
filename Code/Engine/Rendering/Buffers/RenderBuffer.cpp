/************************************************************************/
/* File: RenderBuffer.cpp
/* Author: Andrew Chase
/* Date: January 25th, 2017
/* Bugs: None
/* Description: Implementation of the RenderBuffer class
/************************************************************************/
#include "Engine/Rendering/Buffers/RenderBuffer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Rendering/OpenGL/glFunctions.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor - automatically generates a handle to a GPU-side buffer
//
RenderBuffer::RenderBuffer()
{
	glGenBuffers( 1, &m_handle ); 
	GL_CHECK_ERROR();
}


//-----------------------------------------------------------------------------------------------
// Destructor - deletes the buffer from the GPU
//
RenderBuffer::~RenderBuffer()
{
	// cleanup for a buffer; 
	if (m_handle != NULL) {
		glDeleteBuffers( 1, &m_handle ); 
		m_handle = NULL; 
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the handle (id) that identifies this buffer on the GPU
//
GLuint RenderBuffer::GetHandle() const
{
	return m_handle;
}


//-----------------------------------------------------------------------------------------------
// Returns the current size of the buffer
//
size_t RenderBuffer::GetSize() const
{
	return m_bufferSize;
}


//-----------------------------------------------------------------------------------------------
// Moves the data given by "data" onto the GPU
//
bool RenderBuffer::CopyToGPU( size_t const byte_count, void const *data) 
{
	// Don't do anything if there's no data to copy
	if (byte_count <= 0)
	{
		return false;
	}

	// In case we lose our handle somehow...
	if (m_handle == NULL) {
		glGenBuffers( 1, &m_handle ); 
		GL_CHECK_ERROR();
	}

	// Bind the buffer to a slot, and copy memory
	// GL_DYNAMIC_DRAW means the memory is likely going to change a lot (we'll get
	// during the second project)
	glBindBuffer( GL_COPY_WRITE_BUFFER, m_handle ); 
	glBufferData( GL_COPY_WRITE_BUFFER, byte_count, data, GL_DYNAMIC_DRAW ); 

	GL_CHECK_ERROR();

	// buffer_size is a size_t member variable I keep around for 
	// convenience
	m_bufferSize = byte_count; 
	return true; 
}


//-----------------------------------------------------------------------------------------------
// Copies the data from the buffer at sourceHandle to this buffer
//
bool RenderBuffer::CopyFromGPUBuffer(size_t const byte_count, unsigned int sourceHandle)
{
	glBindBuffer(GL_COPY_READ_BUFFER, sourceHandle);
	GL_CHECK_ERROR();

	glBindBuffer(GL_COPY_WRITE_BUFFER, m_handle);
	GL_CHECK_ERROR();

	glBufferData(GL_COPY_WRITE_BUFFER, byte_count, NULL, GL_DYNAMIC_DRAW);
	GL_CHECK_ERROR();

	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, byte_count);
	GL_CHECK_ERROR();

	glBindBuffer(GL_COPY_READ_BUFFER, NULL);
	glBindBuffer(GL_COPY_WRITE_BUFFER, NULL);

	return true;
}


//-----------------------------------------------------------------------------------------------
// Binds this buffer to the given bind slot
//
void RenderBuffer::Bind(unsigned int bindSlot)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, (GLuint)bindSlot, (GLuint)m_handle);
	GL_CHECK_ERROR();
}


//-----------------------------------------------------------------------------------------------
// Returns a pointer to the head of the data on the GPU
//
void* RenderBuffer::MapBufferData()
{
	// Ensure this buffer is bound
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_handle);

	void* bufferData = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
	GL_CHECK_ERROR();

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, NULL);

	return bufferData;
}


//-----------------------------------------------------------------------------------------------
// Unmaps the buffer
//
void RenderBuffer::UnmapBufferData()
{
	glBindBuffer(GL_COPY_WRITE_BUFFER, m_handle);

	glUnmapBuffer(GL_COPY_WRITE_BUFFER);
	GL_CHECK_ERROR();
}
