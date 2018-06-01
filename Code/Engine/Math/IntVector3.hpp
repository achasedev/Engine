/************************************************************************/
/* File: IntVector3.hpp
/* Author: Andrew Chase
/* Date: February 15th, 2017
/* Description: Class representing a vector of three int elements, x, y and z
/************************************************************************/
#pragma once
#include "Engine/Math/IntVector2.hpp"


//-----------------------------------------------------------------------------------------------
class IntVector3
{

public:
	//-----Public Methods-----

	// Construction/Destruction
	~IntVector3() {}														// destructor: do nothing (for speed)
	IntVector3();															// default constructor: Initialize to 0
	IntVector3( const IntVector3& copyFrom );								// copy constructor
	explicit IntVector3( int initialX, int initialY, int initialZ );		// explicit constructor
	explicit IntVector3( float initialX, float initialY, float initialZ );	// explicit float constructor

																			// Operators
	const IntVector3 operator+( const IntVector3& vecToAdd ) const;			// vec2 + vec2
	const IntVector3 operator-( const IntVector3& vecToSubtract ) const;	// vec2 - vec2
	const IntVector3 operator*( int uniformScale ) const;					// vec2 * int

	void operator+=( const IntVector3& vecToAdd );							// vec2 += vec2
	void operator-=( const IntVector3& vecToSubtract );						// vec2 -= vec2
	void operator*=( const int uniformScale );								// vec2 *= int

	void operator=( const IntVector3& copyFrom );							// vec2 = vec2
	bool operator==( const IntVector3& compare ) const;						// vec2 == vec2
	bool operator!=( const IntVector3& compare ) const;						// vec2 != vec2

	friend const IntVector3 operator*( int uniformScale, const IntVector3& vecToScale );	// int * vec2

	float	GetLength() const;							// Calculates the magnitude of the vector
	float	GetLengthSquared() const;					// Calculates the squared magnitude of the vector

	static IntVector3 GetRandomVector(int maxDeviation);

	// Helper accessors
	IntVector2 xy() const;


public:
	//-----Public Data-----

	// Static constants
 	const static IntVector3 ZERO;


public:
	int x;
	int y;
	int z;
};
