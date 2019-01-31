/************************************************************************/
/* File: IntAABB2.cpp
/* Author: Andrew Chase
/* Date: January 30th 2019
/* Description: Implementation of the IntAABB2 class
/************************************************************************/
#include "Engine/Math/IntAABB2.hpp"
#include "Engine/Math/MathUtils.hpp"

const IntAABB2 IntAABB2::ZEROS = IntAABB2(IntVector2::ZERO, IntVector2::ZERO);

//-----------------------------------------------------------------------------------------------
// Constructor
//
IntAABB2::IntAABB2(const IntVector2& _mins, const IntVector2& _maxs)
	: mins(_mins), maxs(_maxs)
{
}


//-----------------------------------------------------------------------------------------------
// Returns a random coordinate inside the area
//
IntVector2 IntAABB2::GetRandomPointInside() const
{
	IntVector2 randomPoint;

	randomPoint.x = GetRandomIntInRange(mins.x, maxs.x);
	randomPoint.y = GetRandomIntInRange(mins.y, maxs.y);

	return randomPoint;
}


//-----------------------------------------------------------------------------------------------
// Returns the absolute center of the IntAABB2 as a float Vector2
//
Vector2 IntAABB2::GetCenterPoint() const
{
	Vector2 centerPoint;

	centerPoint.x = (float)(mins.x + maxs.x) * 0.5f;
	centerPoint.y = (float)(mins.y + maxs.y) * 0.5f;

	return centerPoint;
}


//-----------------------------------------------------------------------------------------------
// Returns true if this contains the containee, border inclusive
//
bool IntAABB2::DoesContain(const IntAABB2& containee) const
{
	if (mins.x <= containee.mins.x && mins.y <= containee.mins.y
		&& maxs.x >= containee.maxs.x && containee.maxs.y >= containee.maxs.y)
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Returns true if these two IntAABB2s overlap, boundary inclusive
//
bool IntAABB2::DoIntAABB2sOverlap(const IntAABB2& a, const IntAABB2& b)
{
	bool overlaps = true;

	if		(a.maxs.x < b.mins.x) { overlaps = false; }
	else if (b.maxs.x < a.mins.x) { overlaps = false; }
	else if (a.maxs.y < b.mins.y) { overlaps = false; }
	else if (b.maxs.y < a.mins.y) { overlaps = false; }

	return overlaps;
}
