/************************************************************************/
/* File: UniformBuffer.hpp
/* Author: Andrew Chase
/* Date: April 4th, 2017
/* Description: Class to represent a GPU Uniform/Constant buffer
/************************************************************************/
#pragma once
#include "Engine/Rendering/Buffers/RenderBuffer.hpp"

class UniformBuffer
{
public:
	//-----Public Methods-----

	UniformBuffer();
	~UniformBuffer();

	GLuint GetHandle() const;

	//-----Mutators-----
	void InitializeCPUBuffer(size_t byteSize);
	void SetCPUData(size_t byteSize, const void* data);
	void UpdateCPUData(size_t offset, size_t byteSize, const void* data);
	void SetCPUAndGPUData(size_t byteSize, const void* data);
	void CheckAndUpdateGPUData();

	//-----Accessors-----
	void*		GetCPUBuffer();				// Sets the dirty bit
	const void* GetConstCPUBuffer() const;	// Pure get, no mutability
	size_t		GetByteSize() const;


public:
	//-----Template Helpers-----

	// For setting data without passing size
	template <typename T>
	void InitializeCPUBufferForType() { InitializeCPUBuffer(sizeof(T)); }

	template <typename T>
	void SetCPUData(const T& data) { SetCPUData(sizeof(T), &data); }

	// For getting the cpu buffer as a pointer type, mutable
	template <typename T>
	T* GetCPUBufferAsType() { return (T*) GetCPUBuffer(); }

	// For getting the cpu buffer as a pointer type, constant
	template <typename T>
	const T* GetConstCPUBufferAsType() { return (const T*) GetConstCPUBuffer(); }


protected: // Protected so MaterialPropertyBlock can access
	//-----Protected Data-----

	RenderBuffer	m_gpuBuffer;
	void*			m_cpuBuffer = nullptr;
	size_t			m_bufferSize;

	bool			m_isCPUDirty; // True when the cpu buffer changed but not pushed to GPU buffer
};
