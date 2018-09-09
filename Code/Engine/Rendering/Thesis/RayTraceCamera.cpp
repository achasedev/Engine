/************************************************************************/
/* File: RayTraceCamera.cpp
/* Author: Andrew Chase
/* Date: September 4th, 2018
/* Description: Implementation of the RayTraceCamera class
/************************************************************************/
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/Thesis/RayTraceCamera.hpp"



//-----------------------------------------------------------------------------------------------
// Constructor
//
RayTraceCamera::RayTraceCamera(Vector3 lookFrom, Vector3 lookAt, Vector3 up, float verticalFOV, float aspect, float aperture, float focusDistance)
{
	RayTraceCameraData data;
	data.m_lensRadius = aperture * 0.5f;

	float halfHeight = TanDegrees(verticalFOV / 2);
	float halfWidth = aspect * halfHeight;

	// Define the camera matrix
	data.m_origin = lookFrom;
	data.w = (lookAt - lookFrom).GetNormalized();
	data.u = (CrossProduct(up, data.w)).GetNormalized();
	data.v = CrossProduct(data.w, data.u);

	data.m_lowerLeftCorner = data.m_origin - halfWidth * focusDistance * data.u - halfHeight * focusDistance * data.v + focusDistance * data.w;
	data.m_horizontalDirection = 2.f * halfWidth * focusDistance * data.u;
	data.m_verticalDirection = 2.f * halfHeight * focusDistance * data.v;

	m_gpuBuffer.SetCPUAndGPUData(sizeof(data), &data);
}


//-----------------------------------------------------------------------------------------------
// Returns a ray given the screen space pixel coordinates s and t
//
Ray RayTraceCamera::GetRay(float s, float t)
{
	RayTraceCameraData* data = (RayTraceCameraData*)m_gpuBuffer.GetCPUBuffer();
	Vector2 randomDirection = data->m_lensRadius * GetRandomPointWithinCircle();
	Vector3 randomOffset = data->u * randomDirection.x + data->v * randomDirection.y;
	
	return Ray(data->m_origin + randomOffset, data->m_lowerLeftCorner + s * data->m_horizontalDirection + t * data->m_verticalDirection - data->m_origin - randomOffset);
}

unsigned int RayTraceCamera::GetUniformBufferHandle()
{
	return m_gpuBuffer.GetHandle();
}

void RayTraceCamera::UpdateGPUBuffer()
{
	m_gpuBuffer.CheckAndUpdateGPUData();
}

RayTraceCameraData RayTraceCamera::GetData()
{
	return *(m_gpuBuffer.GetCPUBufferAsType<RayTraceCameraData>());
}
