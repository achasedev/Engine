/************************************************************************/
/* Project: MP1 C27 A1 - Vector2, Basics, MathUtils
/* File: Vector2.cpp
/* Author: Andrew Chase
/* Date: August 30th, 2017
/* Bugs: None
/* Description: Implementation of the Vector2 class
/************************************************************************/
#include <math.h>
#include <string>
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntVector2.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"


// Initialize the static constants
const Vector2 Vector2::ZERO = Vector2(0.f, 0.f);
const Vector2 Vector2::ONES = Vector2(1.f, 1.f);
const Vector2 Vector2::DIRECTION_UP = Vector2(0.f, 1.0f);
const Vector2 Vector2::DIRECTION_DOWN = Vector2(0.f, -1.0f);
const Vector2 Vector2::DIRECTION_LEFT = Vector2(-1.0f, 0.f);
const Vector2 Vector2::DIRECTION_RIGHT = Vector2(1.0f, 0.f);

//-----------------------------------------------------------------------------------------------
// Copy constructor
Vector2::Vector2( const Vector2& copy )
	: x( copy.x )
	, y( copy.y )
{
}


//-----------------------------------------------------------------------------------------------
// Explicit constructor - floats
Vector2::Vector2( float initialX, float initialY )
	: x( initialX )
	, y( initialY )
{
}


//-----------------------------------------------------------------------------------------------
// IntVector2 constructor
Vector2::Vector2(const IntVector2& copyFrom)
{
	x = static_cast<float>(copyFrom.x);
	y = static_cast<float>(copyFrom.y);
}


//-----------------------------------------------------------------------------------------------
// Explicit constructor - ints
Vector2::Vector2( int initialX, int initialY )
	: x( static_cast<float>(initialX))
	, y( static_cast<float>(initialY))
{
}


//-----------------------------------------------------------------------------------------------
// Single float constructor
Vector2::Vector2(float initialValue)
	: x(initialValue)
	, y(initialValue)
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
	float length = GetLength();

	if (length > 0.f)
	{
		x = (x / length);
		y = (y / length);
	}

	return length;
}


//-----------------------------------------------------------------------------------------------
// Returns a normalized copy of the vector, the original vector is unchanged
//
Vector2 Vector2::GetNormalized() const
{
	// Ensure we have a valid vector to normalize
	ASSERT_OR_DIE((x != 0.f || y != 0.f), Stringf("Error: Vector2::GetNormalized called on a (0,0) Vector2."));

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
	// Ensure we have a valid vector to calculate on
	ASSERT_OR_DIE((x != 0.f || y != 0.f), Stringf("Error: Vector2::GetOrientationDegrees called on a (0,0) Vector2."));
	return Atan2Degrees(y, x);
}


//-----------------------------------------------------------------------------------------------
// Sets the Vector2 to the values represented in the text passed, expects the form "5,3"
//
bool Vector2::SetFromText(const char* text)
{
	std::string stringText = std::string(text);

	size_t commaPosition = stringText.find(",");

	// No comma present in text
	if (commaPosition == std::string::npos)
	{
		return false;
	}

	x = static_cast<float>(atof(std::string(stringText, 0, commaPosition).c_str()));
	y = static_cast<float>(atof(std::string(stringText, commaPosition + 1).c_str()));

	return true;
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


//-----------------------------------------------------------------------------------------------
// Returns the projected vector in the projectOnto direction with magnitude is the projected length
// of vectorToProject in that direction
//
const Vector2 GetProjectedVector(const Vector2& vectorToProject, const Vector2& projectOnto)
{
	// Optimized for efficiency - using distance squared instead of distance
	float projectOntoMagnitudeSquared = projectOnto.GetLengthSquared();

	float dotProdcut = DotProduct(vectorToProject, projectOnto);

	return (dotProdcut / projectOntoMagnitudeSquared) * projectOnto;
}


//-----------------------------------------------------------------------------------------------
// Returns the originalVector's representation in I,J space (from X, Y space)
// ASSUMES I AND J ARE ORTHONORMAL
//
const Vector2 GetTransformedIntoBasis(const Vector2& originalVector, const Vector2& newBasisI, const Vector2& newBasisJ)
{	
	float iMagnitude = DotProduct(originalVector, newBasisI);
	float jMagnitude = DotProduct(originalVector, newBasisJ);

	return Vector2(iMagnitude, jMagnitude);
}


//-----------------------------------------------------------------------------------------------
// Returns the originalVector's representation in X, Y space
//
const Vector2 GetTransformedOutOfBasis(const Vector2& vectorInBasis, const Vector2& oldBasisI, const Vector2& oldBasisJ)
{
	// Ensure the basis vectors are normalized
	Vector2 iDirection = oldBasisI.GetNormalized();
	Vector2 jDirection = oldBasisJ.GetNormalized();

	Vector2 iComponent = iDirection * vectorInBasis.x;
	Vector2 jComponent = jDirection * vectorInBasis.y;


	// Sum up the corresponding components to get the result
	float xValue = iComponent.x + jComponent.x;
	float yValue = iComponent.y + jComponent.y;

	return Vector2(xValue, yValue);
}


//-----------------------------------------------------------------------------------------------
// Returns the components of originalVector in the I and J direction
//
void DecomposeVectorIntoBasis(const Vector2& originalVector, const Vector2& newBasisI, const Vector2& newBasisJ, Vector2& out_vectorAlongI, Vector2& out_vectorAlongJ)
{
	out_vectorAlongI = GetProjectedVector(originalVector, newBasisI);
	out_vectorAlongJ = GetProjectedVector(originalVector, newBasisJ);
}


//-----------------------------------------------------------------------------------------------
// Finds the "bounce" vector of vectorToReflect after hitting a surface with normal 'normal'
//
const Vector2 Reflect(const Vector2& vectorToReflect, const Vector2& normal)
{
	// Ensure normal is normalized
	Vector2 normalDirection = normal.GetNormalized();

	float magnitudeInNormalDirection = DotProduct(vectorToReflect, normalDirection);
	Vector2 componentInNormalDirection = magnitudeInNormalDirection * normalDirection;

	// Remove the normal, then add its inverse, essentially removing it twice
	Vector2 reflectedResult = vectorToReflect - (2.f * componentInNormalDirection);

	return reflectedResult;
}


//-----------------------------------------------------------------------------------------------
// Returns the vector that is fractionTowardEnd interpolated between start and end
//
const Vector2 Interpolate(const Vector2& start, const Vector2& end, float fractionTowardEnd)
{
	float interpolatedX = Interpolate(start.x, end.x, fractionTowardEnd);
	float interpolatedY = Interpolate(start.y, end.y, fractionTowardEnd);

	return Vector2(interpolatedX, interpolatedY);
}
