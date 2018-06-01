/************************************************************************/
/* Project: MP1 C27 A2 - More Vector2D, Box2D, and Disc2D
/* File: AABB2.hpp
/* Author: Andrew Chase
/* Date: September 4th, 2017
/* Bugs: None
/* Description: Class to represent an Axis-Aligned Bounding Box in 2D
				Note: Boundaries are INCLUSIVE, that is, points along
					  the boundary are considered inside the box
/************************************************************************/
#pragma once
#include "Engine/Math/Vector2.hpp"


class AABB2
{
public:
	Vector2 mins;															// Minimum x and y values of the box
	Vector2 maxs;															// Maximum x and y values of the box

	//-----Constructors-----
	~AABB2() {}
	AABB2() {}
	explicit AABB2(float minX, float minY, float maxX, float maxY);			// Construct using four float values for the bounds
	explicit AABB2(const Vector2& mins, const Vector2& maxs);				// Construct using two Vector2D's to represent the bounds
	explicit AABB2(const Vector2& center, float radiusX, float radiusY);	// Construct using a center and XY offsets (radii)


	//-----Mutators-----
	void StretchToIncludePoint(float x, float y);							// Stretches the box to contain the point at (x,y)
	void StretchToIncludePoint(const Vector2& point);						// Stretches the box to contain 'point'
	void AddPaddingToSides(float xPaddingRadius, float yPaddingRadius);		// Adds offset values to all sides of the box
	void Translate(const Vector2& translation);								// Moves the box linearly by a single Vector2 value
	void Translate(float translationX, float TranslationY);					// Moves the box linearly by float values


	//-----Accessors-----
	bool IsPointInside(float x, float y) const;								// Returns true if the point at (x,y) is inside the box
	bool IsPointInside(const Vector2& point) const;							// Returns true if 'point' is inside the box
	Vector2 GetDimensions() const;											// Returns the dimensions of the box as (width, height)
	Vector2 GetCenter() const;												// Returns the center point position of the box as (x,y)


	//-----Operators-----
	void operator+=(const Vector2& translation);							// Works like Translate, linear move
	void operator-=(const Vector2& antiTranslation);						// Inverse of Translate, moves the box opposite of antiTranslation
	AABB2 operator+(const Vector2& translation) const;						// Returns a copy of this box with an offset added to it
	AABB2 operator-(const Vector2& antiTranslation) const;					// Returns a copy of this box with an offset subtracted to it


	static const AABB2 UNIT_SQUARE_CENTERED;								// Square centered at (0,0) with width 2
	static const AABB2 HALF_UNIT_SQUARE_CENTERED;							// Square centered at (0,0) with width 1
	static const AABB2 UNIT_SQUARE_OFFCENTER;								// Square with bottom left corner at (0,0), width 1
};


bool DoAABBsOverlap(const AABB2& boxOne, const AABB2& boxTwo);				// Utility function - returns true if boxes a and b overlap
