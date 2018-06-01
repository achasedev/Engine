/************************************************************************/
/* File: Mesh.cpp
/* Author: Andrew Chase
/* Date: March 25th, 2018
/* Description: Implementation of the Mesh class
/************************************************************************/
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
// Sets this mesh's indices on the GPU
//
void Mesh::SetIndices(unsigned int indexCount, const unsigned int* indices)
{
	m_indexBuffer.CopyToGPU(indexCount, indices);
}


//-----------------------------------------------------------------------------------------------
// Sets this mesh's params for drawing
//
void Mesh::SetDrawInstruction(PrimitiveType type, bool useIndices, unsigned int startIndex, unsigned int elementCount)
{
	m_drawInstruction = DrawInstruction(type, useIndices, startIndex, elementCount);
}


//-----------------------------------------------------------------------------------------------
// Sets this mesh's params for drawing
//
void Mesh::SetDrawInstruction(DrawInstruction instruction)
{
	m_drawInstruction = instruction;
}


//-----------------------------------------------------------------------------------------------
// Returns this Mesh's VertexBuffer
//
const VertexBuffer* Mesh::GetVertexBuffer() const
{
	return &m_vertexBuffer;
}


//-----------------------------------------------------------------------------------------------
// Returns this Mesh's IndexBuffer
//
const IndexBuffer* Mesh::GetIndexBuffer() const
{
	return &m_indexBuffer;
}


//-----------------------------------------------------------------------------------------------
// Returns this Mesh's DrawInstruction struct
//
DrawInstruction Mesh::GetDrawInstruction() const
{
	return m_drawInstruction;
}


//-----------------------------------------------------------------------------------------------
// Returns this Mesh's layout stride (size of the vertex type it uses)
//
const VertexLayout* Mesh::GetVertexLayout() const
{
	return m_vertexLayout;
}
