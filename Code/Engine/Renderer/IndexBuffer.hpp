/************************************************************************/
/* File: IndexBuffer.hpp
/* Author: Andrew Chase
/* Date: March 25th, 2018
/* Description: Class to represent a GPU buffer for vertex indices
/************************************************************************/
#pragma once
#include "Engine/Renderer/RenderBuffer.hpp"

class IndexBuffer : public RenderBuffer
{
public:
	//-----Public Methods-----

	IndexBuffer();

	bool CopyToGPU(size_t const, void const *, GLenum) = delete;
	bool CopyToGPU(int indexCount, const unsigned int* indices);


private:
	//-----Private Data-----

	unsigned int m_indexCount;
	unsigned int m_indexStride;

};
