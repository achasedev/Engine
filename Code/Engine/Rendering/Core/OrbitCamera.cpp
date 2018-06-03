/************************************************************************/
/* File: OrbitCamera.cpp
/* Author: Andrew Chase
/* Date: February 17th, 2017
/* Description: Implementation of the OrbitCamera class
/************************************************************************/
#include "Engine/Rendering/Core/OrbitCamera.hpp"
#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------------------
// Default constructor
//
OrbitCamera::OrbitCamera()
	: m_radius(0.f)
	, m_rotation(0.f)
	, m_azimuth(0.f)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
OrbitCamera::~OrbitCamera()
{
}


//-----------------------------------------------------------------------------------------------
// Sets the target of the OrbitCamera and updates the matrices
//
void OrbitCamera::SetTarget(Vector3 newTarget)
{
	m_target = newTarget;
	LookAtUsingSphericalCoords();
}


//-----------------------------------------------------------------------------------------------
// Sets the radius of the OrbitCamera (distance from target) and updates the matrices
//
void OrbitCamera::SetRadius(float newRadius)
{
	m_radius = ClampFloat(newRadius, m_radiusRange.min, m_radiusRange.max);
	LookAtUsingSphericalCoords();
}


//-----------------------------------------------------------------------------------------------
// Sets the Y-axis rotation of the camera and updates the matrices
//
void OrbitCamera::SetRotation(float newRotation)
{
	m_rotation = GetAngleBetweenZeroThreeSixty(newRotation);
	LookAtUsingSphericalCoords();
}


//-----------------------------------------------------------------------------------------------
// Sets the azimuth (elevation) angle of the camera and updates the matrices
//
void OrbitCamera::SetAzimuth(float newAzimuth)
{
	m_azimuth = ClampFloat(newAzimuth, m_azimuthRange.min, m_azimuthRange.max);
	LookAtUsingSphericalCoords();
}


//-----------------------------------------------------------------------------------------------
// Sets radius, rotation, and azimuth and updates matrices
//
void OrbitCamera::SetSphericalCoordinates(float radius, float rotation, float azimuth)
{
	m_radius	= ClampFloat(radius, m_radiusRange.min, m_radiusRange.max);
	m_rotation	= GetAngleBetweenZeroThreeSixty(rotation);
	m_azimuth	= ClampFloat(azimuth, m_azimuthRange.min, m_azimuthRange.max);

	LookAtUsingSphericalCoords();
}


//-----------------------------------------------------------------------------------------------
// Moves the camera along the radius by a fixed amount
//
void OrbitCamera::MoveAlongRadius(float deltaAmount)
{
	SetRadius(m_radius + deltaAmount);
}


//-----------------------------------------------------------------------------------------------
// Rotates the camera around the Y-axis
//
void OrbitCamera::RotateHorizontally(float deltaDegrees)
{
	SetRotation(m_rotation + deltaDegrees);
}


//-----------------------------------------------------------------------------------------------
// Rotates the camera along the azimuth angle
//
void OrbitCamera::RotateVertically(float deltaDegrees)
{
	SetAzimuth(m_azimuth + deltaDegrees);
}


//-----------------------------------------------------------------------------------------------
// Sets the min/max radius this camera can be positioned at from the target
//
void OrbitCamera::SetRadiusLimits(float minRange, float maxRange)
{
	m_radiusRange = FloatRange(minRange, maxRange);
}


//-----------------------------------------------------------------------------------------------
// Sets the min/max azimuth angle values this camera can be positioned at
//
void OrbitCamera::SetAzimuthLimits(float minAngle, float maxAngle)
{
	m_azimuthRange = FloatRange(minAngle, maxAngle);
}


//-----------------------------------------------------------------------------------------------
// Returns the target position of this camera
//
Vector3 OrbitCamera::GetTarget() const
{
	return m_target;
}


//-----------------------------------------------------------------------------------------------
// Returns the radius of the camera (distance from target)
//
float OrbitCamera::GetRadius() const
{
	return m_radius;
}


//-----------------------------------------------------------------------------------------------
// Returns this camera's rotation (about the Y-axis)
//
float OrbitCamera::GetRotation() const
{
	return m_rotation;
}


//-----------------------------------------------------------------------------------------------
// Recalculates the camera's camera and view matrices given the member variables
//
void OrbitCamera::LookAtUsingSphericalCoords()
{
	// Find the position of the camera in world coordinates
	Vector3 worldPosition = SphericalToCartesian(m_radius, m_rotation, m_azimuth);

	// Update the camera (look-at) matrix and view matrix
	LookAt(worldPosition + m_target, m_target);
}
