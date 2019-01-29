/************************************************************************/
/* Project: SD1 A4 - Incursion Foundation
/* File: IntVector2.hpp
/* Author: Andrew Chase
/* Date: September 28th, 2017
/* Bugs: None
/* Description: Class representing a vector of two int elements, x and y
/************************************************************************/
#pragma once

class Vector2;

//-----------------------------------------------------------------------------------------------
class IntVector2
{

public:
	//-----Public Methods-----

	// Construction/Destruction
	~IntVector2() {}														// destructor: do nothing (for speed)
	IntVector2() {}															// default constructor: do nothing (for speed)
	IntVector2( const IntVector2& copyFrom );								// copy constructor (from another vec2)
	explicit IntVector2( int initialX, int initialY );						// explicit constructor (from x, y)
	explicit IntVector2( float initialX, float initialY );					// explicit float constructor
	explicit IntVector2(const Vector2& floatVector);
	IntVector2(int initialValue);
																			// Operators
	const IntVector2 operator+( const IntVector2& vecToAdd ) const;			// vec2 + vec2
	const IntVector2 operator-( const IntVector2& vecToSubtract ) const;	// vec2 - vec2
	const IntVector2 operator*( int uniformScale ) const;					// vec2 * int
	const IntVector2 operator/(int divisor) const;

	void operator+=( const IntVector2& vecToAdd );							// vec2 += vec2
	void operator-=( const IntVector2& vecToSubtract );						// vec2 -= vec2
	void operator*=( const int uniformScale );								// vec2 *= int
	void operator=( const IntVector2& copyFrom );							// vec2 = vec2
	bool operator==( const IntVector2& compare ) const;						// vec2 == vec2
	bool operator!=( const IntVector2& compare ) const;						// vec2 != vec2

	bool operator<( const IntVector2& compare ) const;						// Used in particular for keys in maps

	friend const IntVector2 operator*( int uniformScale, const IntVector2& vecToScale );	// int * vec2

	float	GetLength() const;							// Calculates the magnitude of the vector
	float	GetLengthSquared() const;					// Calculates the squared magnitude of the vector
	float	GetOrientationDegrees() const;				// Returns the degree angle of the IntVector2 as on a unit circle
	void	SetFromText(const char* text);				// Sets the int values from the text passed

	Vector2 GetAsFloats() const;

	static IntVector2 GetRandomVector(int maxDeviation);

public:
	//-----Public Data-----

	// Static constants
	const static IntVector2 ZERO;
	const static IntVector2 STEP_NORTH;
	const static IntVector2 STEP_SOUTH;
	const static IntVector2 STEP_EAST;
	const static IntVector2 STEP_WEST;

	const static IntVector2 STEP_NORTHEAST;
	const static IntVector2 STEP_NORTHWEST;
	const static IntVector2 STEP_SOUTHEAST;
	const static IntVector2 STEP_SOUTHWEST;


public:
	int x;
	int y;
};
