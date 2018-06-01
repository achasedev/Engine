/************************************************************************/
/* File: VertexBuffer.hpp
/* Author: Andrew Chase
/* Date: March 25th, 2018
/* Description: Class to represent a GPU-side buffer of vertices
/************************************************************************/
#pragma once
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"


class VertexBuffer : public RenderBuffer
{
public:
	//-----Public Methods-----

	VertexBuffer();

	bool CopyToGPU(size_t const, void const *, GLenum) = delete;

	template <typename VERT_TYPE>
	bool CopyToGPU(int vertexCount, const VERT_TYPE* vertices)
	{
		const VertexLayout* layout = &VERT_TYPE::LAYOUT;

		size_t byteCount = vertexCount * layout->GetStride();
		bool succeeded = RenderBuffer::CopyToGPU(byteCount, (const void*)vertices, GL_ARRAY_BUFFER);

		// Only update if data was copied
		if (succeeded)
		{
			m_vertexCount = vertexCount;
			m_vertexLayout = layout;
		}

		return succeeded;
	}

	unsigned int GetVertexCount() const { return m_vertexCount; }


private:
	//-----Private Data-----

	unsigned int m_vertexCount;

	const VertexLayout* m_vertexLayout;
};
