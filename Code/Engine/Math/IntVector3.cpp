/************************************************************************/
/* File: IntVector3.cpp
/* Author: Andrew Chase
/* Date: February 15th, 2017
/* Description: Implementation of the IntVector3 class
/************************************************************************/
#include <math.h>
#include <string>
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/MathUtils.hpp"


// Initialize the static constants
 const IntVector3 IntVector3::ZERO				= IntVector3(0, 0, 0);
 const IntVector3 IntVector3::ONES				= IntVector3(1, 1, 1);

//-----------------------------------------------------------------------------------------------
// default constructor
IntVector3::IntVector3()
	: x(0), y(0), z(0)
{
}


//-----------------------------------------------------------------------------------------------
// Copy constructor
IntVector3::IntVector3( const IntVector3& copy )
	: x( copy.x )
	, y( copy.y )
	, z( copy.z )
{
}


//-----------------------------------------------------------------------------------------------
// Explicit constructor
IntVector3::IntVector3( int initialX, int initialY, int initialZ )
	: x( initialX )
	, y( initialY )
	, z( initialZ )
{
}


//-----------------------------------------------------------------------------------------------
// Explicit float constructor
IntVector3::IntVector3(float initialX, float initialY, float initialZ)
	: x(static_cast<int>(initialX))
	, y(static_cast<int>(initialY))
	, z(static_cast<int>(initialZ))
{
}


//-----------------------------------------------------------------------------------------------
// Explicit constructor from float vector3
IntVector3::IntVector3(const Vector3& copyFrom)
	: IntVector3(copyFrom.x, copyFrom.y, copyFrom.z)
{
}


//-----------------------------------------------------------------------------------------------
// Explicit constructor from single int
IntVector3::IntVector3(int initialValue)
	: x(initialValue), y(initialValue), z(initialValue)
{
}

//------------------------------ Operator Overloads ---------------------------------------------

//-----------------------------------------------------------------------------------------------
const IntVector3 IntVector3::operator + ( const IntVector3& vecToAdd ) const
{
	return IntVector3( (x + vecToAdd.x), (y + vecToAdd.y), (z + vecToAdd.z));
}


//-----------------------------------------------------------------------------------------------
const IntVector3 IntVector3::operator-( const IntVector3& vecToSubtract ) const
{
	return IntVector3( (x - vecToSubtract.x), (y - vecToSubtract.y), (z - vecToSubtract.z) );
}


//-----------------------------------------------------------------------------------------------
const IntVector3 IntVector3::operator*( int uniformScale ) const
{
	return IntVector3( (x * uniformScale), (y * uniformScale), (z * uniformScale) );
}


//-----------------------------------------------------------------------------------------------
const IntVector3 IntVector3::operator/(int divisor) const
{
	return IntVector3(x / divisor, y / divisor, z / divisor);
}


//-----------------------------------------------------------------------------------------------
void IntVector3::operator+=( const IntVector3& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


//-----------------------------------------------------------------------------------------------
void IntVector3::operator-=( const IntVector3& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}


//-----------------------------------------------------------------------------------------------
void IntVector3::operator*=( const int uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void IntVector3::operator=( const IntVector3& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}


//-----------------------------------------------------------------------------------------------
const IntVector3 operator*( int uniformScale, const IntVector3& vecToScale )
{
	return IntVector3( (vecToScale.x * uniformScale), (vecToScale.y * uniformScale), (vecToScale.z * uniformScale) );
}


//-----------------------------------------------------------------------------------------------
bool IntVector3::operator==( const IntVector3& compare ) const
{
	if ( x == compare.x && y == compare.y && z == compare.z ) 
	{
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------------------------
bool IntVector3::operator!=( const IntVector3& compare ) const
{
	if ( x != compare.x || y != compare.y || z != compare.z ) 
	{
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------------------------
// Calculates the magnitude (length) of the vector and returns it
//
float IntVector3::GetLength() const
{
	return sqrtf(static_cast<float>((x * x) + (y * y) + (z * z)));
}


//-----------------------------------------------------------------------------------------------
// Calculates the squared magnitude (length) of the vector and returns it
//
float IntVector3::GetLengthSquared() const
{
	return static_cast<float>((x * x) + (y * y) * (z * z));
}


//-----------------------------------------------------------------------------------------------
// Returns this vector as a float vector
//
Vector3 IntVector3::GetAsFloats() const
{
	return Vector3(*this);
}


//-----------------------------------------------------------------------------------------------
// Sets the vector values from the text provided
//
void IntVector3::SetFromText(const char* text)
{
	std::string stringText = text;

	size_t firstCommaPosition = stringText.find(",");

	// No comma present in text
	if (firstCommaPosition == std::string::npos)
	{
		return;
	}

	int xValue = atoi(std::string(stringText, 0, firstCommaPosition).c_str());

	size_t secondCommaPosition = stringText.find(",", firstCommaPosition + 1);

	// Not enough commas/nothing between commas
	if (secondCommaPosition == std::string::npos || secondCommaPosition == firstCommaPosition + 1)
	{
		return;
	}

	int yValue = atoi(std::string(text, firstCommaPosition + 1, secondCommaPosition).c_str());
	int zValue = atoi(std::string(text, secondCommaPosition + 1).c_str());

	x = xValue;
	y = yValue;
	z = zValue;
}


//-----------------------------------------------------------------------------------------------
// Returns a random 3D IntVector with values within the maxDeviation of 0
//
IntVector3 IntVector3::GetRandomVector(int maxDeviation)
{
	IntVector3 randomVector;

	randomVector.x = GetRandomIntInRange(-maxDeviation, maxDeviation);
	randomVector.y = GetRandomIntInRange(-maxDeviation, maxDeviation);
	randomVector.z = GetRandomIntInRange(-maxDeviation, maxDeviation);

	return randomVector;
}


//-----------------------------------------------------------------------------------------------
// Returns the x and y components of this IntVector3
//
IntVector2 IntVector3::xy() const
{
	return IntVector2(x, y);
}


//-----------------------------------------------------------------------------------------------
// Returns the x and z components of this IntVector3
//
IntVector2 IntVector3::xz() const
{
	return IntVector2(x, z);
}


//-----------------------------------------------------------------------------------------------
// Returns the distance between points a and b
//
float GetDistance(const IntVector3& a, const IntVector3& b)
{
	return sqrtf(static_cast<float>((a.x - b.x) * (a.x - b.x)) + ((a.y - b.y) * (a.y - b.y)) + ((a.z - b.z) * (a.z - b.z)));
}


//-----------------------------------------------------------------------------------------------
// Returns the squared distance between points a and b
//
float GetDistanceSquared(const IntVector3& a, const IntVector3& b)
{
	return (static_cast<float>((a.x - b.x) * (a.x - b.x)) + ((a.y - b.y) * (a.y - b.y)) + ((a.z - b.z) * (a.z - b.z)));
}
