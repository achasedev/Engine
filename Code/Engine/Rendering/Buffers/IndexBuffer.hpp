/************************************************************************/
/* File: IndexBuffer.hpp
/* Author: Andrew Chase
/* Date: March 25th, 2018
/* Description: Class to represent a GPU buffer for vertex indices
/************************************************************************/
#pragma once
#include "Engine/Rendering/Buffers/RenderBuffer.hpp"

class IndexBuffer : public RenderBuffer
{
public:
	//-----Public Methods-----

	IndexBuffer();

	bool CopyToGPU(size_t const, void const *, GLenum) = delete;
	bool CopyToGPU(int indexCount, const unsigned int* indices);

	bool CopyFromGPUBuffer(size_t const byte_count, unsigned int sourceHandle) = delete;
	bool CopyFromGPUBuffer(unsigned int indexCount, unsigned int sourceHandle);

	void			SetIndexCount(unsigned int indexCount);
	unsigned int	GetIndexCount() const;
	unsigned int	GetIndexStride() const;


private:
	//-----Private Data-----

	unsigned int m_indexCount;
	unsigned int m_indexStride;

};
