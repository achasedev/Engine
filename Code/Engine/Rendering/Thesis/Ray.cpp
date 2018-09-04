/************************************************************************/
/* File: Ray.cpp
/* Author: Andrew Chase
/* Date: September 4th, 2018
/* Description: Implementation of the Ray class
/************************************************************************/
#include "Engine/Rendering/Thesis/Ray.hpp"


//-----------------------------------------------------------------------------------------------
// Default Constructor
//
Ray::Ray()
	: m_position(Vector3::ZERO), m_direction(Vector3::ZERO)
{
}


//-----------------------------------------------------------------------------------------------
// Constructor from data values
//
Ray::Ray(const Vector3& position, const Vector3& direction)
	: m_position(position), m_direction(direction)
{
}


//-----------------------------------------------------------------------------------------------
// Returns the position this ray originates from
//
Vector3 Ray::GetPosition() const
{
	return m_position;
}


//-----------------------------------------------------------------------------------------------
// Returns the direction the ray is going in
//
Vector3 Ray::GetDirection() const
{
	return m_direction;
}


//-----------------------------------------------------------------------------------------------
// Returns the position along the ray at parametrized point t (P(t) = position + direction * t)
//
Vector3 Ray::GetPointAtParameter(float t) const
{
	return m_position + m_direction * t;
}
