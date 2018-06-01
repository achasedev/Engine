/************************************************************************/
/* Project: SD1 A4 - Incursion Foundation
/* File: IntVector2.hpp
/* Author: Andrew Chase
/* Date: September 28th, 2017
/* Bugs: None
/* Description: Class representing a vector of two int elements, x and y
/************************************************************************/
#pragma once
#include "Engine/Math/Vector2.hpp"

//-----------------------------------------------------------------------------------------------
class IntVector2
{

public:
	// Construction/Destruction
	~IntVector2() {}											// destructor: do nothing (for speed)
	IntVector2() {}											// default constructor: do nothing (for speed)
	IntVector2( const IntVector2& copyFrom );						// copy constructor (from another vec2)
	explicit IntVector2( int initialX, int initialY );		// explicit constructor (from x, y)

															// Operators
	const IntVector2 operator+( const IntVector2& vecToAdd ) const;		// vec2 + vec2
	const IntVector2 operator-( const IntVector2& vecToSubtract ) const;	// vec2 - vec2
	const IntVector2 operator*( int uniformScale ) const;			// vec2 * float
	const IntVector2 operator/( int inverseScale ) const;			// vec2 / float
	void operator+=( const IntVector2& vecToAdd );						// vec2 += vec2
	void operator-=( const IntVector2& vecToSubtract );				// vec2 -= vec2
	void operator*=( const int uniformScale );					// vec2 *= float
	void operator/=( const int uniformDivisor );					// vec2 /= float
	void operator=( const IntVector2& copyFrom );						// vec2 = vec2
	bool operator==( const IntVector2& compare ) const;				// vec2 == vec2
	bool operator!=( const IntVector2& compare ) const;				// vec2 != vec2

	friend const IntVector2 operator*( int uniformScale, const IntVector2& vecToScale );	// float * vec2

	float GetLength() const;				// calculates the magnitude of the vector
	float GetLengthSquared() const;			// calculates the squared magnitude of the vector
	float NormalizeAndGetLength();			// Normalizes the vector and returns its original length
	IntVector2 GetNormalized() const;			// calculates the normalized form of the vector
	float GetOrientationDegrees() const;
	Vector2 GetAsFloats() const;

	const static IntVector2 ZERO;
	const static IntVector2 STEP_NORTH;
	const static IntVector2 STEP_SOUTH;
	const static IntVector2 STEP_EAST;
	const static IntVector2 STEP_WEST;

	const static IntVector2 STEP_NORTHEAST;
	const static IntVector2 STEP_NORTHWEST;
	const static IntVector2 STEP_SOUTHEAST;
	const static IntVector2 STEP_SOUTHWEST;



public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	int x;
	int y;
};
