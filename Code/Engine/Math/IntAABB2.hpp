/************************************************************************/
/* File: IntAABB2.hpp
/* Author: Andrew Chase
/* Date: January 30th 2019
/* Description: Class to represent an Axis-Aligned Bounding Box with int values
/************************************************************************/
#pragma once
#include "Engine/Math/IntVector2.hpp"

class IntAABB2
{
public:
	//-----Public Methods-----

	IntAABB2() {}
	IntAABB2(const IntVector2& _mins, const IntVector2& _maxs);

	// Producers
	bool		DoesContain(const IntAABB2& containee) const;
	Vector2		GetCenterPoint() const;
	IntVector2	GetRandomPointInside() const;

	// Statics
	static bool DoIntAABB2sOverlap(const IntAABB2& a, const IntAABB2& b);


public:
	//-----Public Data-----

	IntVector2 mins;
	IntVector2 maxs;

	static const IntAABB2 ZEROS;

};
