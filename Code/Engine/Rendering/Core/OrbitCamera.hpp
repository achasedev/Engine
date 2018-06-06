/************************************************************************/
/* File: OrbitCamera.hpp
/* Author: Andrew Chase
/* Date: February 17th, 2017
/* Description: Class to represent a camera that pivots around a central target
/************************************************************************/
#pragma once
#include "Engine/Rendering/Core/Camera.hpp"
#include "Engine/Math/FloatRange.hpp"


class OrbitCamera : public Camera
{
public:
	//-----Public Methods-----

	OrbitCamera();
	~OrbitCamera();

	// Setters
	void SetTarget(Vector3 newTarget); 
	void SetRadius(float newRadius);
	void SetRotation(float newRotation);
	void SetAzimuth(float newAzimuth);
	void SetSphericalCoordinates(float radius, float rotation, float azimuth); 

	void MoveAlongRadius(float deltaAmount);
	void RotateHorizontally(float deltaDegrees);
	void RotateVertically(float deltaDegrees);

	void SetRadiusLimits(float minRange, float maxRange);
	void SetAzimuthLimits(float minAngle, float maxAngle);

	// Accessors
	Vector3 GetTarget() const;
	float	GetRadius() const;
	float	GetHorizontalRotation() const;


private:
	//-----Private Methods-----

	void LookAtUsingSphericalCoords();	// Updates the Camera and View matrix, used when a member is updated


public:
	//-----Private Data-----

	Vector3 m_target; 

	float m_radius;      // distance from target
	float m_rotation;    // rotation around Y
	float m_azimuth;     // rotation toward up after previous rotation

	FloatRange m_radiusRange;
	FloatRange m_azimuthRange;
};
