/************************************************************************/
/* File: VertexBuffer.cpp
/* Author: Andrew Chase
/* Date: March 25th, 2018
/* Description: Implementation of the VertexBuffer class
/************************************************************************/
#include "Engine/Rendering/Buffers/VertexBuffer.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
VertexBuffer::VertexBuffer()
	: m_vertexCount(0)
{
}


//-----------------------------------------------------------------------------------------------
// Sets the number of vertices in the vertex buffer
//
void VertexBuffer::SetVertexCount(unsigned int vertexCount)
{
	m_vertexCount = vertexCount;
	m_bufferSize = m_vertexLayout->GetStride() * vertexCount;
}
