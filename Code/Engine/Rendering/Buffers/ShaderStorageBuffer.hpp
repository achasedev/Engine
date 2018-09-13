/************************************************************************/
/* File: ShaderStorageBuffer.hpp
/* Author: Andrew Chase
/* Date: September 8th, 2018
/* Bugs: None
/* Description: Class to represent an OpenGL SSBO
/************************************************************************/
#pragma once
#include "ThirdParty/gl/glcorearb.h"

class ShaderStorageBuffer
{
public:
	//-----Public Methods-----

	ShaderStorageBuffer();
	~ShaderStorageBuffer();	// Destructor - Deletes it from the GPU

	GLuint	GetHandle() const;

	// Moves data into this render buffer, and sends it to the GPU
	bool	CopyToGPU(size_t const byte_count, void const *data);
	bool	Clear(size_t const byte_count);

	void	Bind(int bindSlot);

	void*	MapBufferData();
	void	UnmapBufferData();


protected:
	//-----Protected Data-----

	size_t m_bufferSize;	// Size of this buffer, is updated in CopyToGPU
	GLuint m_handle;		// The handle to this buffer on the GPU (lazy instantiated)

};