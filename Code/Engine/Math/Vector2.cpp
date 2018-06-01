/************************************************************************/
/* Project: MP1 C27 A1 - Vector2, Basics, MathUtils
/* File: Vector2.cpp
/* Author: Andrew Chase
/* Date: August 30th, 2017
/* Bugs: None
/* Description: Implementation of the Vector2 class
/************************************************************************/
#include <math.h>
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/MathUtils.hpp"


// Initialize the static constants
const Vector2 Vector2::ZERO = Vector2(0.f, 0.f);
const Vector2 Vector2::ONES = Vector2(1.f, 1.f);


//-----------------------------------------------------------------------------------------------
// Copy constructor
Vector2::Vector2( const Vector2& copy )
	: x( copy.x )
	, y( copy.y )
{
}


//-----------------------------------------------------------------------------------------------
// Explicit constructor
Vector2::Vector2( float initialX, float initialY )
	: x( initialX )
	, y( initialY )
{
}


//------------------------------ Operator Overloads ---------------------------------------------

//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator + ( const Vector2& vecToAdd ) const
{
	return Vector2( (x + vecToAdd.x), (y + vecToAdd.y) );
}


//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator-( const Vector2& vecToSubtract ) const
{
	return Vector2( (x - vecToSubtract.x), (y - vecToSubtract.y) );
}


//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator*( float uniformScale ) const
{
	return Vector2( (x * uniformScale), (y * uniformScale) );
}


//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator/( float inverseScale ) const
{
	float multScaler = (1.f / inverseScale);
	return Vector2( (x * multScaler), (y * multScaler) );
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator+=( const Vector2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator-=( const Vector2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator/=( const float uniformDivisor )
{
	float multScaler = (1.f / uniformDivisor);

	x *= multScaler;
	y *= multScaler;
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator=( const Vector2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
const Vector2 operator*( float uniformScale, const Vector2& vecToScale )
{
	return Vector2( (vecToScale.x * uniformScale), (vecToScale.y * uniformScale) );
}


//-----------------------------------------------------------------------------------------------
bool Vector2::operator==( const Vector2& compare ) const
{
	if ( x == compare.x && y == compare.y ) {
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------------------------
bool Vector2::operator!=( const Vector2& compare ) const
{
	if ( x != compare.x || y != compare.y ) {
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------------------------
// Calculates the magnitude (length) of the vector and returns it
//
float Vector2::GetLength() const
{
	return sqrtf((x * x) + (y * y));
}


//-----------------------------------------------------------------------------------------------
// Calculates the squared magnitude (length) of the vector and returns it
//
float Vector2::GetLengthSquared() const
{
	return (x * x) + (y * y);
}


//-----------------------------------------------------------------------------------------------
// Normalizes the vector and returns its original length
//
float Vector2::NormalizeAndGetLength()
{
	float length = this->GetLength();

	x = (x / length);
	y = (y / length);

	return length;

}


//-----------------------------------------------------------------------------------------------
// Returns a normalized copy of the vector, the original vector is unchanged
//
Vector2 Vector2::GetNormalized() const
{
	float magnitude = GetLength();
	
	Vector2 normalizedForm;

	normalizedForm.x = (this->x / magnitude);
	normalizedForm.y = (this->y / magnitude);

	return normalizedForm;
}


//-----------------------------------------------------------------------------------------------
// Returns the degree orientation of a vector, as if the angle was on a unit circle
//
float Vector2::GetOrientationDegrees() const
{
	return Atan2Degrees(y, x);
}


//-----------------------------------------------------------------------------------------------
// Returns the unit vector that represents the angle degrees on a unit circle
//
Vector2 Vector2::MakeDirectionAtDegrees(float degrees)
{
	Vector2 direction;
	
	direction.x = CosDegrees(degrees);
	direction.y = SinDegrees(degrees);

	return direction;
}


//-----------------------------------------------------------------------------------------------
// Returns a randomly oriented vector given the desired magnitude
//
Vector2 Vector2::GetRandomVector(float desiredMagnitude)
{
	float randomDegrees = GetRandomFloatInRange(0.f, 360.f);
	Vector2 randomUnitVector = MakeDirectionAtDegrees(randomDegrees);

	return (desiredMagnitude * randomUnitVector);
}


//-----------------------------------------------------------------------------------------------
// Returns the distance between points a and b on a 2D plane
//
float GetDistance(const Vector2& a, const Vector2& b)
{
	return sqrtf(((a.x - b.x) * (a.x - b.x)) + ((a.y - b.y) * (a.y - b.y)));
}


//-----------------------------------------------------------------------------------------------
// Returns the squared distance between points a and b on a 2D plane
//
float GetDistanceSquared(const Vector2& a, const Vector2& b)
{
	return ((a.x - b.x) * (a.x - b.x)) + ((a.y - b.y) * (a.y - b.y));
}
