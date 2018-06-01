/************************************************************************/
/* Project: SD1 A4 - Incursion Foundation
/* File: IntVector2.cpp
/* Author: Andrew Chase
/* Date: September 28th, 2017
/* Bugs: None
/* Description: Implementation of the IntVector2 class
/************************************************************************/
#include <math.h>
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/MathUtils.hpp"


// Initialize the static constants
const IntVector2 IntVector2::ZERO = IntVector2(0, 0);
const IntVector2 IntVector2::STEP_NORTH = IntVector2(0, 1);
const IntVector2 IntVector2::STEP_SOUTH = IntVector2(0, -1);
const IntVector2 IntVector2::STEP_EAST = IntVector2(1, 0);
const IntVector2 IntVector2::STEP_WEST = IntVector2(-1, 0);
const IntVector2 IntVector2::STEP_NORTHEAST = IntVector2(1, 1);
const IntVector2 IntVector2::STEP_NORTHWEST = IntVector2(-1, 1);
const IntVector2 IntVector2::STEP_SOUTHEAST = IntVector2(1, -1);
const IntVector2 IntVector2::STEP_SOUTHWEST = IntVector2(-1, -1);

//-----------------------------------------------------------------------------------------------
// Copy constructor
IntVector2::IntVector2( const IntVector2& copy )
	: x( copy.x )
	, y( copy.y )
{
}


//-----------------------------------------------------------------------------------------------
// Explicit constructor
IntVector2::IntVector2( int initialX, int initialY )
	: x( initialX )
	, y( initialY )
{
}


//------------------------------ Operator Overloads ---------------------------------------------

//-----------------------------------------------------------------------------------------------
const IntVector2 IntVector2::operator + ( const IntVector2& vecToAdd ) const
{
	return IntVector2( (x + vecToAdd.x), (y + vecToAdd.y) );
}


//-----------------------------------------------------------------------------------------------
const IntVector2 IntVector2::operator-( const IntVector2& vecToSubtract ) const
{
	return IntVector2( (x - vecToSubtract.x), (y - vecToSubtract.y) );
}


//-----------------------------------------------------------------------------------------------
const IntVector2 IntVector2::operator*( int uniformScale ) const
{
	return IntVector2( (x * uniformScale), (y * uniformScale) );
}


//-----------------------------------------------------------------------------------------------
const IntVector2 IntVector2::operator/( int inverseScale ) const
{
	float multScaler = (1.f / inverseScale);
	return IntVector2( static_cast<int>(x * multScaler), static_cast<int>(y * multScaler) );
}


//-----------------------------------------------------------------------------------------------
void IntVector2::operator+=( const IntVector2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void IntVector2::operator-=( const IntVector2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void IntVector2::operator*=( const int uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void IntVector2::operator/=( const int uniformDivisor )
{
	float multScaler = (1.f / uniformDivisor);

	x = static_cast<int>(x * multScaler);
	y *= static_cast<int>(y * multScaler);
}


//-----------------------------------------------------------------------------------------------
void IntVector2::operator=( const IntVector2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
const IntVector2 operator*( int uniformScale, const IntVector2& vecToScale )
{
	return IntVector2( static_cast<int>(vecToScale.x * uniformScale), static_cast<int>(vecToScale.y * uniformScale) );
}


//-----------------------------------------------------------------------------------------------
bool IntVector2::operator==( const IntVector2& compare ) const
{
	if ( x == compare.x && y == compare.y ) {
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------------------------
bool IntVector2::operator!=( const IntVector2& compare ) const
{
	if ( x != compare.x || y != compare.y ) {
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------------------------
// Calculates the magnitude (length) of the vector and returns it
//
float IntVector2::GetLength() const
{
	return sqrtf(static_cast<float>((x * x) + (y * y)));
}


//-----------------------------------------------------------------------------------------------
// Calculates the squared magnitude (length) of the vector and returns it
//
float IntVector2::GetLengthSquared() const
{
	return static_cast<float>((x * x) + (y * y));
}


//-----------------------------------------------------------------------------------------------
// Normalizes the vector and returns its original length
//
float IntVector2::NormalizeAndGetLength()
{
	float length = GetLength();

	x = static_cast<int>(x / length);
	y = static_cast<int>(y / length);

	return length;

}


//-----------------------------------------------------------------------------------------------
// Returns a normalized copy of the vector, the original vector is unchanged
//
IntVector2 IntVector2::GetNormalized() const
{
	float magnitude = GetLength();

	IntVector2 normalizedForm;

	normalizedForm.x = static_cast<int>(x / magnitude);
	normalizedForm.y = static_cast<int>(y / magnitude);

	return normalizedForm;
}


//-----------------------------------------------------------------------------------------------
// Returns the degree orientation of a vector, as if the angle was on a unit circle
//
float IntVector2::GetOrientationDegrees() const
{
	return Atan2Degrees(static_cast<float>(y), static_cast<float>(x));
}


//-----------------------------------------------------------------------------------------------
// Returns this IntVector2 as a Vector2 of float values, essentially just casting
//
Vector2 IntVector2::GetAsFloats() const
{
	Vector2 floatVector;

	floatVector.x = static_cast<float>(x);
	floatVector.y = static_cast<float>(y);

	return floatVector;
}


//-----------------------------------------------------------------------------------------------
// Returns the distance between points a and b on a 2D plane
//
float GetDistance(const IntVector2& a, const IntVector2& b)
{
	return sqrtf(static_cast<float>((a.x - b.x) * (a.x - b.x)) + ((a.y - b.y) * (a.y - b.y)));
}


//-----------------------------------------------------------------------------------------------
// Returns the squared distance between points a and b on a 2D plane
//
float GetDistanceSquared(const IntVector2& a, const IntVector2& b)
{
	return (static_cast<float>((a.x - b.x) * (a.x - b.x)) + ((a.y - b.y) * (a.y - b.y)));
}
