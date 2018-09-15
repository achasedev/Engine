/************************************************************************/
/* File: Mesh.hpp
/* Author: Andrew Chase
/* Date: March 25th, 2018
/* Description: Class to represent a set of vertices/indices for rendering
/************************************************************************/
#pragma once
#include "Engine/Rendering/Buffers/IndexBuffer.hpp"
#include "Engine/Rendering/Buffers/VertexBuffer.hpp"


struct DrawInstruction
{
	DrawInstruction() {}
	DrawInstruction(PrimitiveType type, bool useIndices, unsigned int startIndex, unsigned int elementCount)
		: m_primType(type), m_startIndex(startIndex), m_elementCount(elementCount), m_usingIndices(useIndices) {}

	PrimitiveType	m_primType;
	unsigned int	m_startIndex;
	unsigned int	m_elementCount;
	bool			m_usingIndices;
};


class Mesh
{
public:
	//-----Public Methods-----

	// Mutators
	void SetIndices(unsigned int indexCount,	const unsigned int* indices);

	template <typename VERT_TYPE>
	void SetVertices(unsigned int vertexCount,	const VERT_TYPE* vertices)
	{
		bool succeeded = m_vertexBuffer.CopyToGPU(vertexCount, vertices);
		
		if (succeeded)
		{
			m_vertexLayout = &VERT_TYPE::LAYOUT;
		}
	}

	void SetIndicesFromGPUBuffer(unsigned int indexCount, unsigned int sourceBufferHandle);

	template <typename VERT_TYPE>
	void SetVerticesFromGPUBuffer(unsigned int vertexCount, unsigned int sourceBufferHandle)
	{
		bool succeeded = m_vertexBuffer.CopyFromGPUBuffer<VERT_TYPE>(vertexCount, sourceBufferHandle);

		if (succeeded)
		{
			m_vertexLayout = &VERT_TYPE::LAYOUT;
		}
	}

	template <typename VERT_TYPE>
	void InitializeBuffersForCompute(unsigned int vertexBindSlot, unsigned int initialVertexCount, unsigned int indexBindSlot, unsigned int initialIndexCount)
	{
		m_vertexBuffer.Bind(vertexBindSlot);
		m_vertexBuffer.CopyToGPU<VERT_TYPE>(initialVertexCount, nullptr);

		m_vertexLayout = &VERT_TYPE::LAYOUT;

		m_indexBuffer.Bind(indexBindSlot);
		m_indexBuffer.CopyToGPU(initialIndexCount, nullptr);
	}

	void UpdateCounts(unsigned int vertexCount, unsigned int indexCount);

	void SetDrawInstruction(DrawInstruction instruction);
	void SetDrawInstruction(PrimitiveType type, bool useIndices, unsigned int startIndex, unsigned int elementCount);

	// Accessors
	const VertexBuffer*		GetVertexBuffer() const;
	const IndexBuffer*		GetIndexBuffer() const;
	DrawInstruction		GetDrawInstruction() const;
	const VertexLayout*	GetVertexLayout() const;


private:
	//-----Private Data-----

	VertexBuffer		m_vertexBuffer;
	IndexBuffer			m_indexBuffer;
	DrawInstruction		m_drawInstruction;

	const VertexLayout* m_vertexLayout = &VertexLit::LAYOUT;

};
