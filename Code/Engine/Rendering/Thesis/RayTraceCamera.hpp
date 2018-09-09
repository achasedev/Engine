/************************************************************************/
/* File: RayTraceCamera.hpp
/* Author: Andrew Chase
/* Date: September 4th, 2018
/* Description: Class to represent a camera used for ray trace rendering
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Rendering/Thesis/Ray.hpp"
#include "Engine/Rendering/Buffers/UniformBuffer.hpp"

class RayTraceCamera
{
public:
	//-----Public Methods-----

	RayTraceCamera(Vector3 lookFrom, Vector3 lookAt, Vector3 up, float verticalFOV, float aspect, float aperture, float focusDistance);

	// Returns a ray given the screen coordinates s and t (u and v)
	Ray GetRay(float s, float t);


	unsigned int GetUniformBufferHandle();

	void UpdateGPUBuffer();


private:
	//-----Private Data-----

	UniformBuffer m_gpuBuffer;

};
