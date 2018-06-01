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

	// Moves data into this render buffer, and sends it to the GPU
	bool CopyToGPU(size_t const byte_count, void const *data, GLenum bufferType);


protected:
	//-----Protected Data-----

	size_t m_bufferSize;	// Size of this buffer, is updated in CopyToGPU
	GLuint m_handle;		// The handle to this buffer on the GPU (lazy instantiated)

};