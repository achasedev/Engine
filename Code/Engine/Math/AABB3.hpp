/************************************************************************/
/* File: AABB3.hpp
/* Author: Andrew Chase
/* Date: March 26th, 2018
/* Description: Class to represent a min and max value in 3D space
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"


class AABB3
{
public:
	Vector3 mins;		// Minimum x and y values of the box
	Vector3 maxs;		// Maximum x and y values of the box

	//-----Constructors-----
	~AABB3() {}
	AABB3() {}
	explicit AABB3(const Vector3& mins, const Vector3& maxs);							// Construct using two Vector2D's to represent the bounds
	explicit AABB3(const Vector3& center, float radiusX, float radiusY, float radiusZ);	// Construct using a center and XY offsets (radii)

	//-----Producers-----
	Vector3 GetDimensions() const;
	Vector3 GetCenter() const;

	Vector3 GetFrontBottomLeft() const;
	Vector3 GetFrontBottomRight() const;
	Vector3 GetFrontTopRight() const;
	Vector3 GetFrontTopLeft() const;

	Vector3 GetBackBottomLeft() const;
	Vector3 GetBackBottomRight() const;
	Vector3 GetBackTopRight() const;
	Vector3 GetBackTopLeft() const;

	//-----Static Constants-----
	static const AABB3 UNIT_CUBE;

};
