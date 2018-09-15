/************************************************************************/
/* File: VertexBuffer.hpp
/* Author: Andrew Chase
/* Date: March 25th, 2018
/* Description: Class to represent a GPU-side buffer of vertices
/************************************************************************/
#pragma once
#include "Engine/Rendering/Core/Vertex.hpp"
#include "Engine/Rendering/Buffers/RenderBuffer.hpp"


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
		bool succeeded = RenderBuffer::CopyToGPU(byteCount, (const void*)vertices);

		// Only update if data was copied
		if (succeeded)
		{
			m_vertexCount = vertexCount;
			m_vertexLayout = layout;
		}

		return succeeded;
	}

	bool CopyFromGPUBuffer(size_t const byte_count, unsigned int sourceHandle) = delete;

	template<typename VERT_TYPE>
	bool CopyFromGPUBuffer(unsigned int vertexCount, unsigned int sourceHandle)
	{
		const VertexLayout* layout = &VERT_TYPE::LAYOUT;

		size_t byteCount = vertexCount * layout->GetStride();
		bool succeeded = RenderBuffer::CopyFromGPUBuffer(byteCount, sourceHandle);

		// Only update if data was copied
		if (succeeded)
		{
			m_vertexCount = vertexCount;
			m_vertexLayout = layout;
		}

		return succeeded;
	}

	void				SetVertexCount(unsigned int vertexCount);

	unsigned int		GetVertexCount() const { return m_vertexCount; }
	const VertexLayout* GetVertexLayout() const { return m_vertexLayout; }


private:
	//-----Private Data-----

	unsigned int m_vertexCount;
	const VertexLayout* m_vertexLayout;

};
