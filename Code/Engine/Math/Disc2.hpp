/************************************************************************/
/* Project: MP1 C27 A2 - More Vector2D, Box2D, and Disc2D
/* File: Disc2.hpp
/* Author: Andrew Chase
/* Date: September 6th, 2017
/* Bugs: None
/* Description: Class to represent a two-dimensional disc
				Note: Boundaries are INCLUSIVE, that is, points along
				the boundary are considered inside the disc
/************************************************************************/
#pragma once
#include "Engine/Math/Vector2.hpp"


class Disc2
{
public:

	//-----Members-----
	Vector2 center;
	float radius;

	//-----Constructors-----
	~Disc2() {}
	Disc2() {}
	Disc2(const Disc2& copyFrom);
	explicit Disc2(float initialX, float initialY, float initialRadius);
	explicit Disc2(const Vector2& initialCenter, float initialRadius);

	//-----Mutators-----
	void StretchToIncludePoint(float x, float y);						// Expands the radius, does not move the disc
	void StretchToIncludePoint(const Vector2& point);					// Also expands without moving the center
	void AddPaddingToRadius(float paddingRadius);						// Addition to the radius
	void Translate(const Vector2& translation);							// Move the center by translation
	void Translate(float translationX, float translationY);				// Moves the center by translation

	//-----Accessors/Queries-----
	bool IsPointInside(float x, float y) const;
	bool IsPointInside(const Vector2& point) const;

	//-----Operators-----
	void operator+=(const Vector2& translation);
	void operator-=(const Vector2& antiTranslation);
	Disc2 operator+(const Vector2& translation) const;
	Disc2 operator-(const Vector2& antiTranslation) const;
};

//-----Utility Functions-----

// Detects whether two discs overlap, i.e. their boundaries intersect or one
// disc is encapsulated within the other
bool DoDiscsOverlap(const Disc2& a, const Disc2& b);		

// Detects whether two discs overlap, using the member data of a disc instead of two disc objects
bool DoDiscsOverlap(const Vector2& aCenter, float aRadius, const Vector2& bCenter, float bRadius);

// Detects whether a disc overlaps a point, boundary inclusive
bool DoesDiscOverlapPoint(const Vector2& center, float radius, const Vector2& point);

// Interpolates the discs by interpolating their center positions and their radii
const Disc2 Interpolate(const Disc2& start, const Disc2& end, float fractionTowardEnd);
