/************************************************************************/
/* File: Vector4.cpp
/* Author: Andrew Chase
/* Date: February 15th, 2017
/* Bugs: None
/* Description: Implementation of the Vector4 class
/************************************************************************/
#include <math.h>
#include <string>
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/MathUtils.hpp"


// Initialize the static constants
const Vector4 Vector4::ZERO	= Vector4(0.f, 0.f, 0.f, 1.0f);
const Vector4 Vector4::ONES	= Vector4(1.f, 1.f, 1.f, 1.0f);
					
const Vector4 Vector4::DIRECTION_UP			= Vector4(0.f,	 1.0f, 0.f, 1.0f);
const Vector4 Vector4::DIRECTION_DOWN		= Vector4(0.f,	-1.0f, 0.f, 1.0f);
const Vector4 Vector4::DIRECTION_LEFT		= Vector4(-1.0f, 0.f,  0.f, 1.0f);
const Vector4 Vector4::DIRECTION_RIGHT		= Vector4(1.0f,	 0.f,  0.f, 1.0f);
const Vector4 Vector4::DIRECTION_FORWARD	= Vector4(0.f,	 0.f,  1.f, 1.0f);
const Vector4 Vector4::DIRECTION_BACK		= Vector4(0.f,	 0.f, -1.f, 1.0f);


//-----------------------------------------------------------------------------------------------
// Copy constructor
Vector4::Vector4( const Vector4& copy )
	: x( copy.x )
	, y( copy.y )
	, z(copy.z)
	, w(copy.w)
{
}


//-----------------------------------------------------------------------------------------------
// Explicit constructor
Vector4::Vector4( float initialX, float initialY, float initialZ, float initialW)
	: x( initialX )
	, y( initialY )
	, z(initialZ)
	, w(initialW)
{
}


//------------------------------ Operator Overloads ---------------------------------------------

//-----------------------------------------------------------------------------------------------
const Vector4 Vector4::operator + ( const Vector4& vecToAdd ) const
{
	return Vector4( (x + vecToAdd.x), (y + vecToAdd.y), (z + vecToAdd.z), (w + vecToAdd.w));
}


//-----------------------------------------------------------------------------------------------
const Vector4 Vector4::operator-( const Vector4& vecToSubtract ) const
{
	return Vector4( (x - vecToSubtract.x), (y - vecToSubtract.y) , (z - vecToSubtract.z), (w - vecToSubtract.w));
}


//-----------------------------------------------------------------------------------------------
const Vector4 Vector4::operator*( float uniformScale ) const
{
	return Vector4( (x * uniformScale), (y * uniformScale), (z * uniformScale), (w * uniformScale));
}


//-----------------------------------------------------------------------------------------------
const Vector4 Vector4::operator/( float inverseScale ) const
{
	float multScaler = (1.f / inverseScale);
	return Vector4( (x * multScaler), (y * multScaler), (z * multScaler), (w * multScaler) );
}


//-----------------------------------------------------------------------------------------------
void Vector4::operator+=( const Vector4& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
	w += vecToAdd.w;
}


//-----------------------------------------------------------------------------------------------
void Vector4::operator-=( const Vector4& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
	w -= vecToSubtract.w;
}


//-----------------------------------------------------------------------------------------------
void Vector4::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
	w *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vector4::operator/=( const float uniformDivisor )
{
	float multScaler = (1.f / uniformDivisor);

	x *= multScaler;
	y *= multScaler;
	z *= multScaler;
	w *= multScaler;
}


//-----------------------------------------------------------------------------------------------
void Vector4::operator=( const Vector4& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
	w = copyFrom.w;
}


//-----------------------------------------------------------------------------------------------
const Vector4 operator*( float uniformScale, const Vector4& vecToScale )
{
	return Vector4( (vecToScale.x * uniformScale), (vecToScale.y * uniformScale), (vecToScale.z * uniformScale), (vecToScale.w * uniformScale) );
}


//-----------------------------------------------------------------------------------------------
bool Vector4::operator==( const Vector4& compare ) const
{
	if (x == compare.x && y == compare.y && z == compare.z && w == compare.w) {
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------------------------
bool Vector4::operator!=( const Vector4& compare ) const
{
	if ( x != compare.x || y != compare.y || z != compare.z || w != compare.w) {
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------------------------
// Calculates the magnitude (length) of the vector and returns it
//
float Vector4::GetLength() const
{
	return sqrtf((x * x) + (y * y) + (z * z) + (w * w));
}


//-----------------------------------------------------------------------------------------------
// Calculates the squared magnitude (length) of the vector and returns it
//
float Vector4::GetLengthSquared() const
{
	return (x * x) + (y * y) + (z * z) + (w * w);
}


//-----------------------------------------------------------------------------------------------
// Normalizes the vector and returns its original length
//
float Vector4::NormalizeAndGetLength()
{
	float length = this->GetLength();
	float oneOverLength = (1.f / length);

	x = (x * oneOverLength);
	y = (y * oneOverLength);
	z = (z * oneOverLength);
	w = (w * oneOverLength);

	return length;
}


//-----------------------------------------------------------------------------------------------
// Returns a normalized copy of the vector, the original vector is unchanged
//
Vector4 Vector4::GetNormalized() const
{
	float magnitude = GetLength();
	float oneOverMagnitude = (1.f / magnitude);

	Vector4 normalizedForm;

	normalizedForm.x = (x * oneOverMagnitude);
	normalizedForm.y = (y * oneOverMagnitude);
	normalizedForm.z = (z * oneOverMagnitude);
	normalizedForm.w = (w * oneOverMagnitude);

	return normalizedForm;
}


//-----------------------------------------------------------------------------------------------
// Returns a randomly oriented vector given the desired magnitude
// NOT EFFICIENT
//
Vector4 Vector4::GetRandomVector(float desiredMagnitude)
{
	Vector4 randomVector;
	randomVector.x = GetRandomFloatInRange(0.f, 1.f);
	randomVector.y = GetRandomFloatInRange(0.f, 1.f);
	randomVector.z = GetRandomFloatInRange(0.f, 1.f);
	randomVector.w = GetRandomFloatInRange(0.f, 1.f);

	Vector4 randomUnitVector = randomVector.GetNormalized();

	return (desiredMagnitude * randomUnitVector);
}


//-----------------------------------------------------------------------------------------------
// Returns the vector that is fractionTowardEnd interpolated between start and end
//
const Vector4 Interpolate(const Vector4& start, const Vector4& end, float fractionTowardEnd)
{
	float interpolatedX = Interpolate(start.x, end.x, fractionTowardEnd);
	float interpolatedY = Interpolate(start.y, end.y, fractionTowardEnd);
	float interpolatedZ = Interpolate(start.z, end.z, fractionTowardEnd);
	float interpolatedW = Interpolate(start.w, end.w, fractionTowardEnd);

	return Vector4(interpolatedX, interpolatedY, interpolatedZ, interpolatedW);
}
