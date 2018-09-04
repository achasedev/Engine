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
	m_lensRadius = aperture * 0.5f;

	float halfHeight = TanDegrees(verticalFOV / 2);
	float halfWidth = aspect * halfHeight;

	// Define the camera matrix
	m_origin = lookFrom;
	w = (lookAt - lookFrom).GetNormalized();
	u = (CrossProduct(up, w)).GetNormalized();
	v = CrossProduct(w, u);

	m_lowerLeftCorner = m_origin - halfWidth * focusDistance * u - halfHeight * focusDistance * v + focusDistance * w;
	m_horizontalDirection = 2.f * halfWidth * focusDistance * u;
	m_verticalDirection = 2.f * halfHeight * focusDistance * v;
}


//-----------------------------------------------------------------------------------------------
// Returns a ray given the screen space pixel coordinates s and t
//
Ray RayTraceCamera::GetRay(float s, float t)
{
	Vector2 randomDirection = m_lensRadius * GetRandomPointWithinCircle();
	Vector3 randomOffset = u * randomDirection.x + v * randomDirection.y;
	
	return Ray(m_origin + randomOffset, m_lowerLeftCorner + s * m_horizontalDirection + t * m_verticalDirection - m_origin - randomOffset);
}
