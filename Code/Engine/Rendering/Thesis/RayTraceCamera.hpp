/************************************************************************/
/* File: RayTraceCamera.hpp
/* Author: Andrew Chase
/* Date: September 4th, 2018
/* Description: Class to represent a camera used for ray trace rendering
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Rendering/Thesis/Ray.hpp"

class RayTraceCamera
{
public:
	//-----Public Methods-----

	RayTraceCamera(Vector3 lookFrom, Vector3 lookAt, Vector3 up, float verticalFOV, float aspect, float aperture, float focusDistance);

	// Returns a ray given the screen coordinates s and t (u and v)
	Ray GetRay(float s, float t);


private:
	//-----Private Data-----

	Vector3 m_origin;					// Where the camera is positioned
	Vector3 m_lowerLeftCorner;			// The lower left corner in camera space of the view plane
	Vector3 m_horizontalDirection;		// The "right" direction in screen space, NOT normalized
	Vector3 m_verticalDirection;		// The "up" direction in screen space, NOT normalized
	Vector3 u, v, w;					// Camera basis vectors
	
	float m_lensRadius;					// For depth of field effects

};
