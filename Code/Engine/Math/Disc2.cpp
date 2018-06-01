/************************************************************************/
/* Project: MP1 C27 A2 - More Vector2D, Box2D, and Disc2D
/* File: Disc2.cpp
/* Author: Andrew Chase
/* Date: September 6th, 2017
/* Bugs: None
/* Description: Implementation of the Disc2 class
/************************************************************************/
#include "Engine/Math/Disc2.hpp"


//-----------------------------------------------------------------------------------------------
// Copy constructor for a Disc2
//
Disc2::Disc2(const Disc2& copyFrom)
	: center(copyFrom.center)
	, radius(copyFrom.radius)
{
}


//-----------------------------------------------------------------------------------------------
// Explicit constructor for a Disc2, using float data
//
Disc2::Disc2(float initialX, float initialY, float initialRadius)
	: center(Vector2(initialX, initialY))
	, radius(initialRadius)
{
}


//-----------------------------------------------------------------------------------------------
// Explicit constructor for a Disc2, using Vector2 and float data
//
Disc2::Disc2(const Vector2& initialCenter, float initialRadius)
	: center(initialCenter)
	, radius(initialRadius)
{
}


//-----------------------------------------------------------------------------------------------
// Stretches (increases the radius) of a Disc2 to include the point (x,y) within its bounds
//
void Disc2::StretchToIncludePoint(float x, float y)
{
	float distanceBetween = GetDistance(center, Vector2(x, y));

	// If distance between is greater than the radius, it is outside the boundary
	if (distanceBetween > radius)
	{
		// Set the radius so the point is on the Disc2's boundary
		radius = distanceBetween;
	}
}


//-----------------------------------------------------------------------------------------------
// Stretches (increases the radius) of a Disc2 to include 'point' within its bounds
//
void Disc2::StretchToIncludePoint(const Vector2& point)
{
	float distanceBetween = GetDistance(center, point);

	// If distance between is greater than the radius, it is outside the boundary
	if (distanceBetween > radius)
	{
		// Set the radius so the point is on the Disc2's boundary
		radius = distanceBetween;
	}
}


//-----------------------------------------------------------------------------------------------
// Additively increases the radius of the circle by paddingRadius
//
void Disc2::AddPaddingToRadius(float paddingRadius)
{
	radius += paddingRadius;
}


//-----------------------------------------------------------------------------------------------
// Moves (translates) the center point of the Disc2 by the Vector2 'translation'
//
void Disc2::Translate(const Vector2& translation)
{
	center += translation;
}


//-----------------------------------------------------------------------------------------------
// Moves (translates) the center point of the Disc2 component-wise by adding translationX and
// translationY accordingly
//
void Disc2::Translate(float translationX, float translationY)
{
	center.x += translationX;
	center.y += translationY;
}


//-----------------------------------------------------------------------------------------------
// Determines whether the point (x,y) is within the bounds of this Disc2
//
bool Disc2::IsPointInside(float x, float y) const
{
	float distanceBetween = GetDistance(center, Vector2(x, y));

	bool isInside = true;

	if (radius < distanceBetween)
	{
		isInside = false;
	}

	return isInside;
}


//-----------------------------------------------------------------------------------------------
// Determines whether the point 'point' is within the bounds of this Disc2
//
bool Disc2::IsPointInside(const Vector2& point) const
{
	float distanceBetween = GetDistance(center, point);

	bool isInside = true;

	if (radius < distanceBetween)
	{
		isInside = false;
	}

	return isInside; 
}


//-----------------------------------------------------------------------------------------------
// Adds the Vector2 'translation' to the center - equivalent to Translate()
//
void Disc2::operator+=(const Vector2& translation)
{
	center += translation;
}


//-----------------------------------------------------------------------------------------------
// Subtracts the Vector2 'antiTranslation' to the center
//
void Disc2::operator-=(const Vector2& antiTranslation)
{
	center -= antiTranslation;
}


//-----------------------------------------------------------------------------------------------
// Returns a copy of the original Disc2 translated by 'translation'
//
Disc2 Disc2::operator+(const Vector2& translation) const
{
	Disc2 translatedDisc;;

	translatedDisc.radius = radius;

	translatedDisc.center = (center + translation);

	return translatedDisc;
}


//-----------------------------------------------------------------------------------------------
// Returns a copy of the original Disc2 with 'antiTranslation' subtracted from its center
//
Disc2 Disc2::operator-(const Vector2& antiTranslation) const
{
	Disc2 antiTranslatedDisc;;

	antiTranslatedDisc.radius = radius;

	antiTranslatedDisc.center = (center - antiTranslation);

	return antiTranslatedDisc;
}


//-----------------------------------------------------------------------------------------------
// Determines whether two Disc2 objects overlap in 2D-space.
// That is, either their boundaries overlap or one contains another.
//
bool DoDiscsOverlap(const Disc2& a, const Disc2& b)
{
	float distanceBetween = GetDistance(a.center, b.center);

	float sumOfRadii = a.radius + b.radius;

	bool doOverlap = true;

	if (sumOfRadii < distanceBetween)
	{
		doOverlap = false;
	}

	return doOverlap;
}


//-----------------------------------------------------------------------------------------------
// Determines whether two discs overlap in 2D-space by checking their radii and centers.
//
bool DoDiscsOverlap(const Vector2& aCenter, float aRadius, const Vector2& bCenter, float bRadius)
{
	float distanceBetween = GetDistanceSquared(aCenter, bCenter);

	float sumOfRadii = aRadius + bRadius;
	sumOfRadii *= sumOfRadii;

	bool doOverlap = true;

	if (sumOfRadii < distanceBetween)
	{
		doOverlap = false;
	}

	return doOverlap;
}


//-----------------------------------------------------------------------------------------------
// Checks if a disc given by 'center' and 'radius' overlaps the point 'point'
bool DoesDiscOverlapPoint(const Vector2& center, float radius, const Vector2& point)
{
	float distanceSquared = GetDistanceSquared(center, point);
	float radiusSquared = (radius * radius);
	
	return (distanceSquared <= radiusSquared);
}
