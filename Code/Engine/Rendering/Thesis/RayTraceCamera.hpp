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

struct RayTraceCameraData
{
	Vector3 m_origin;					// Where the camera is positioned
	float padding0;
	Vector3 m_lowerLeftCorner;			// The lower left corner in camera space of the view plane
	float padding1;
	Vector3 m_horizontalDirection;		// The "right" direction in screen space, NOT normalized
	float padding2;
	Vector3 m_verticalDirection;		// The "up" direction in screen space, NOT normalized
	float padding3;
	Vector3 u;							// Camera basis vectors
	float padding4;
	Vector3 v;
	float padding5;
	Vector3 w;
	float m_lensRadius;					// For depth of field effects
};

class RayTraceCamera
{
public:
	//-----Public Methods-----

	RayTraceCamera(Vector3 lookFrom, Vector3 lookAt, Vector3 up, float verticalFOV, float aspect, float aperture, float focusDistance);

	// Returns a ray given the screen coordinates s and t (u and v)
	Ray GetRay(float s, float t);


	unsigned int GetUniformBufferHandle();
	void UpdateGPUBuffer();

	RayTraceCameraData GetData();

private:
	//-----Private Data-----

	UniformBuffer m_gpuBuffer;

};
