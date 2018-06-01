/************************************************************************/
/* File: AABB3.cpp
/* Author: Andrew Chase
/* Date: March 26th, 2018
/* Description: Implementation of the AABB3 class
/************************************************************************/
#include "Engine/Math/AABB3.hpp"


// Static constants
const AABB3 AABB3::UNIT_CUBE = AABB3(Vector3(0.f, 0.f, 0.f), Vector3(1.f, 1.f, 1.f));


//-----------------------------------------------------------------------------------------------
// Constructor, given mins and maxs
//
AABB3::AABB3(const Vector3& mins, const Vector3& maxs)
	: mins(mins), maxs(maxs)
{
}


//-----------------------------------------------------------------------------------------------
// Constructor, given center and dimensions
//
AABB3::AABB3(const Vector3& center, float radiusX, float radiusY, float radiusZ)
	: mins(Vector3((center.x - radiusX), (center.y - radiusY), (center.z - radiusZ)))
	, maxs(Vector3((center.x + radiusX), (center.y + radiusY), (center.z + radiusZ)))
{
}


//-----------------------------------------------------------------------------------------------
// Returns the dimensions of this "box"
//
Vector3 AABB3::GetDimensions() const
{
	return (maxs - mins);
}


//-----------------------------------------------------------------------------------------------
// Returns the center (average) of the two points
//
Vector3 AABB3::GetCenter() const
{
	return (maxs + mins) * 0.5f;
}


//-----------------------------------------------------------------------------------------------
// Returns the FrontBottomLeft point
//
Vector3 AABB3::GetFrontBottomLeft() const
{
	return mins;
}


//-----------------------------------------------------------------------------------------------
// Returns the FrontBottomRight point
//
Vector3 AABB3::GetFrontBottomRight() const
{
	return Vector3(maxs.x, mins.y, mins.z);
}


//-----------------------------------------------------------------------------------------------
// Returns the FrontTopRight point
//
Vector3 AABB3::GetFrontTopRight() const
{
	return Vector3(maxs.x, maxs.y, mins.z);
}


//-----------------------------------------------------------------------------------------------
// Returns the FrontTopLeft point
//
Vector3 AABB3::GetFrontTopLeft() const
{
	return Vector3(mins.x, maxs.y, mins.z);
}


//-----------------------------------------------------------------------------------------------
// Returns the BackBottomLeft point
//
Vector3 AABB3::GetBackBottomLeft() const
{
	return Vector3(mins.x, mins.y, maxs.z);
}


//-----------------------------------------------------------------------------------------------
// Returns the BackBottomRight point
//
Vector3 AABB3::GetBackBottomRight() const
{
	return Vector3(maxs.x, mins.y, maxs.z);
}


//-----------------------------------------------------------------------------------------------
// Returns the BackTopRight point
//
Vector3 AABB3::GetBackTopRight() const
{
	return maxs;
}


//-----------------------------------------------------------------------------------------------
// Returns the BackTopLeft point
//
Vector3 AABB3::GetBackTopLeft() const
{
	return Vector3(mins.x, maxs.y, maxs.z);
}
