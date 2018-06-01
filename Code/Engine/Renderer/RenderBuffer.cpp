/************************************************************************/
/* File: RenderBuffer.cpp
/* Author: Andrew Chase
/* Date: January 25th, 2017
/* Bugs: None
/* Description: Implementation of the RenderBuffer class
/************************************************************************/
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/glFunctions.hpp"


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
// Moves the data given by "data" onto the GPU
//
bool RenderBuffer::CopyToGPU( size_t const byte_count, void const *data, GLenum bufferType) 
{
	// handle is a GLuint member - used by OpenGL to identify this buffer
	// if we don't have one, make one when we first need it [lazy instantiation]
	if (m_handle == NULL) {
		glGenBuffers( 1, &m_handle ); 
	}

	// Bind the buffer to a slot, and copy memory
	// GL_DYNAMIC_DRAW means the memory is likely going to change a lot (we'll get
	// during the second project)
	glBindBuffer( bufferType, m_handle ); 
	glBufferData( bufferType, byte_count, data, GL_DYNAMIC_DRAW ); 

	// buffer_size is a size_t member variable I keep around for 
	// convenience
	m_bufferSize = byte_count; 
	return true; 
}
