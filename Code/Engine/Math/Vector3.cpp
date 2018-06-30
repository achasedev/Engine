/************************************************************************/
/* File: Vector3.cpp
/* Author: Andrew Chase
/* Date: November 7th, 2017
/* Bugs: None
/* Description: Implementation of the Vector3 class
/************************************************************************/
#include <math.h>
#include <string>
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/MathUtils.hpp"


// Initialize the static constants
const Vector3 Vector3::ZERO	= Vector3(0.f, 0.f, 0.f);
const Vector3 Vector3::ONES	= Vector3(1.f, 1.f, 1.f);

const Vector3 Vector3::DIRECTION_UP			= Vector3(0.f, 1.0f, 0.f);
const Vector3 Vector3::DIRECTION_DOWN		= Vector3(0.f, -1.0f, 0.f);
const Vector3 Vector3::DIRECTION_LEFT		= Vector3(-1.0f, 0.f, 0.f);
const Vector3 Vector3::DIRECTION_RIGHT		= Vector3(1.0f, 0.f, 0.f);
const Vector3 Vector3::DIRECTION_FORWARD	= Vector3(0.f, 0.f, 1.f);
const Vector3 Vector3::DIRECTION_BACK		= Vector3(0.f, 0.f, -1.f);


//-----------------------------------------------------------------------------------------------
// Copy constructor
Vector3::Vector3( const Vector3& copy )
	: x( copy.x )
	, y( copy.y )
	, z(copy.z)
{
}


//-----------------------------------------------------------------------------------------------
// Explicit constructor
Vector3::Vector3( float initialX, float initialY, float initialZ)
	: x( initialX )
	, y( initialY )
	, z(initialZ)
{
}


//-----------------------------------------------------------------------------------------------
// Constructor - from an IntVector3
//
Vector3::Vector3(const IntVector3& intVector)
{
	x = static_cast<float>(intVector.x);
	y = static_cast<float>(intVector.y);
	z = static_cast<float>(intVector.z);
}


//-----------------------------------------------------------------------------------------------
// Constructor - from a single float value
//
Vector3::Vector3(float value)
	: x(value), y(value), z(value)
{
}

//------------------------------ Operator Overloads ---------------------------------------------

//-----------------------------------------------------------------------------------------------
const Vector3 Vector3::operator + ( const Vector3& vecToAdd ) const
{
	return Vector3( (x + vecToAdd.x), (y + vecToAdd.y), (z + vecToAdd.z));
}


//-----------------------------------------------------------------------------------------------
const Vector3 Vector3::operator-( const Vector3& vecToSubtract ) const
{
	return Vector3( (x - vecToSubtract.x), (y - vecToSubtract.y) , (z - vecToSubtract.z));
}


//-----------------------------------------------------------------------------------------------
const Vector3 Vector3::operator*( float uniformScale ) const
{
	return Vector3( (x * uniformScale), (y * uniformScale), (z * uniformScale) );
}


//-----------------------------------------------------------------------------------------------
const Vector3 Vector3::operator/( float inverseScale ) const
{
	float multScaler = (1.f / inverseScale);
	return Vector3( (x * multScaler), (y * multScaler), (z * multScaler) );
}


//-----------------------------------------------------------------------------------------------
void Vector3::operator+=( const Vector3& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


//-----------------------------------------------------------------------------------------------
void Vector3::operator-=( const Vector3& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}


//-----------------------------------------------------------------------------------------------
void Vector3::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vector3::operator/=( const float uniformDivisor )
{
	float multScaler = (1.f / uniformDivisor);

	x *= multScaler;
	y *= multScaler;
	z *= multScaler;
}


//-----------------------------------------------------------------------------------------------
void Vector3::operator=( const Vector3& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}


//-----------------------------------------------------------------------------------------------
const Vector3 operator*( float uniformScale, const Vector3& vecToScale )
{
	return Vector3( (vecToScale.x * uniformScale), (vecToScale.y * uniformScale), (vecToScale.z * uniformScale) );
}


//-----------------------------------------------------------------------------------------------
bool Vector3::operator==( const Vector3& compare ) const
{
	if ( x == compare.x && y == compare.y && z == compare.z ) {
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------------------------
bool Vector3::operator!=( const Vector3& compare ) const
{
	if ( x != compare.x || y != compare.y || z != compare.z ) {
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------------------------
// Calculates the magnitude (length) of the vector and returns it
//
float Vector3::GetLength() const
{
	return sqrtf((x * x) + (y * y) + (z * z));
}


//-----------------------------------------------------------------------------------------------
// Calculates the squared magnitude (length) of the vector and returns it
//
float Vector3::GetLengthSquared() const
{
	return (x * x) + (y * y) + (z * z);
}


//-----------------------------------------------------------------------------------------------
// Normalizes the vector and returns its original length
//
float Vector3::NormalizeAndGetLength()
{
	float length = this->GetLength();
	float oneOverLength = (1.f / length);

	x = (x * oneOverLength);
	y = (y * oneOverLength);
	z = (z * oneOverLength);

	return length;
}


//-----------------------------------------------------------------------------------------------
// Returns a normalized copy of the vector, the original vector is unchanged
//
Vector3 Vector3::GetNormalized() const
{
	float magnitude = GetLength();

	if (AreMostlyEqual(magnitude, 0.f))
	{
		return Vector3::ZERO;
	}

	float oneOverMagnitude = (1.f / magnitude);

	Vector3 normalizedForm;

	normalizedForm.x = (x * oneOverMagnitude);
	normalizedForm.y = (y * oneOverMagnitude);
	normalizedForm.z = (z * oneOverMagnitude);

	return normalizedForm;
}


//-----------------------------------------------------------------------------------------------
// Sets the Vector2 to the values represented in the text passed
//
bool Vector3::SetFromText(const char* text)
{
	bool usingCommas = true;
	std::string stringText = std::string(text);

	int firstComma = static_cast<int>(stringText.find(","));

	// No comma present in text, so instead look for spaces
	if (firstComma == static_cast<int>(std::string::npos))
	{
		usingCommas = false;
		firstComma = static_cast<int>(stringText.find(" "));

		// No spaces either, so return false
		if (firstComma == static_cast<int>(std::string::npos))
		{
			return false;
		}
	}

	// Look for a comma or space, depending on the first comma/space
	int secondComma = (usingCommas ? static_cast<int>(stringText.find(",", firstComma + 1)) : static_cast<int>(stringText.find(" ", firstComma + 1)));

	// No second comma/space present in text
	if (secondComma == static_cast<int>(std::string::npos))
	{
		return false;
	}

	// Set the values
	x = static_cast<float>(atof(std::string(stringText, 0, firstComma).c_str()));
	y = static_cast<float>(atof(std::string(stringText, firstComma + 1, secondComma - firstComma - 1).c_str()));
	z = static_cast<float>(atof(std::string(stringText, secondComma + 1).c_str()));

	return true;
}


//-----------------------------------------------------------------------------------------------
// Returns the x and z values of this vector as a Vector2
//
Vector2 Vector3::xz() const
{
	return Vector2(x, z);
}


//-----------------------------------------------------------------------------------------------
// Returns a randomly oriented vector given the desired magnitude
// NOT EFFICIENT
//
Vector3 Vector3::GetRandomVector(float desiredMagnitude)
{
	Vector3 randomVector;
	randomVector.x = GetRandomFloatInRange(0.f, 1.f);
	randomVector.y = GetRandomFloatInRange(0.f, 1.f);
	randomVector.z = GetRandomFloatInRange(0.f, 1.f);

	Vector3 randomUnitVector = randomVector.GetNormalized();

	return (desiredMagnitude * randomUnitVector);
}


//-----------------------------------------------------------------------------------------------
// Returns the vector spherically lerped between start and end by percent
//
Vector3 Vector3::Slerp(const Vector3& start, const Vector3& end, float percent)
{
	float dot = DotProduct(start, end);
	dot = ClampFloat(dot, -1.0f, 1.0f); // Clamp for safety

	float theta = acosf(dot) * percent;	// Angle between start and the result we want
	
	Vector3 relative = (end - start * dot).GetNormalized(); // Direction we need to move towards result

	Vector3 result = start * cosf(theta) + relative * sinf(theta); // Get the result

	return result;
}


//-----------------------------------------------------------------------------------------------
// Returns the vector that is fractionTowardEnd interpolated between start and end
//
const Vector3 Interpolate(const Vector3& start, const Vector3& end, float fractionTowardEnd)
{
	float interpolatedX = Interpolate(start.x, end.x, fractionTowardEnd);
	float interpolatedY = Interpolate(start.y, end.y, fractionTowardEnd);
	float interpolatedZ = Interpolate(start.z, end.z, fractionTowardEnd);

	return Vector3(interpolatedX, interpolatedY, interpolatedZ);
}
