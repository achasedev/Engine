#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Rendering/Thesis/Ray.hpp"

class RayTraceCamera
{
public:
	//-----Public Methods-----

	RayTraceCamera(Vector3 lookFrom, Vector3 lookAt, Vector3 up, float verticalFOV, float aspect, float aperture, float focusDistance);

	Ray GetRay(float s, float t);

private:
	//-----Private Data-----

	Vector3 m_origin;
	Vector3 m_lowerLeftCorner;
	Vector3 m_horizontalDirection;
	Vector3 m_verticalDirection;
	Vector3 u, v, w;
	
	float m_lensRadius;

};
