/************************************************************************/
/* File: RenderBuffer.hpp
/* Author: Andrew Chase
/* Date: January 25th, 2017
/* Bugs: None
/* Description: Class to represent a CPU buffer the GPU can access
/************************************************************************/

#pragma once
#include "ThirdParty/gl/glcorearb.h"

class RenderBuffer
{
public:
	//-----Public Methods-----

	RenderBuffer();
	virtual ~RenderBuffer();	// Destructor - Deletes it from the GPU

	GLuint GetHandle() const;
	size_t GetSize() const;

	// Moves data into this render buffer, and sends it to the GPU
	bool CopyToGPU(size_t const byte_count, void const *data);
	
	// Copies the contents of buffer from sourceHandle into this buffer
	bool CopyFromGPUBuffer(size_t const byte_count, unsigned int sourceHandle);

	void Bind(unsigned int bindSlot);

	void*	MapBufferData();
	void	UnmapBufferData();

protected:
	//-----Protected Data-----

	size_t m_bufferSize;	// Size of this buffer, is updated in CopyToGPU
	GLuint m_handle;		// The handle to this buffer on the GPU (lazy instantiated)

};
