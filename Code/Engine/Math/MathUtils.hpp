/************************************************************************/
/* Project: MP1 C27 A1 - Vector2, Basics, MathUtils
/* File: MathUtils.hpp
/* Author: Andrew Chase
/* Date: August 30th, 2017
/* Bugs: None
/* Description: Math Utility Class for my personal game engine
/************************************************************************/
#pragma once
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/IntVector3.hpp"
#include <stdint.h>

// Constants
const float PI = 3.1415926535897932384626433832795f;

// Math functions
int		Ceiling(float value);
float 	Log2(float value);
float 	Log10(float value);
float 	ModFloat(float x, float y);
float	Sqrt(float value);
float   Pow(float base, float exponent);

// Coordinate system
Vector2 PolarToCartesian(float radius, float angleRadians);
void	CartesianToPolar(float x, float y, float& out_radius, float& out_angleRadians);
Vector3 SphericalToCartesian(float radius, float rotationDegrees, float azimuthDegrees);

// Angle manipulation
float	ConvertRadiansToDegrees(float radians);									// Converts an angle in radians to degrees
float	ConvertDegreesToRadians(float degrees);									// converts an angle in degrees to radians
float	CosDegrees(float degrees);												// Calculates the cosine of an angle in degrees
float   ACosDegrees(float ratio);												// Calculates the arccosine of the ratio given, in degrees
float	SinDegrees(float degrees);												// Calculates the sine of an angle in degrees
float   ASinDegrees(float ratio);												// Calculates the arcsine of the ratio given, in degrees
float	TanDegrees(float degrees);												// Calculates the tangent of the angle in degrees
float	Atan2Degrees(float y, float x);											// Calculates arctan(y, x) expressed in degrees
float	Atan2Degrees(float ratio);												// Calculates arctan of the side ratio, expressed in degrees
float	GetNearestCardinalAngle(float angle);									// Returns the nearest cardinal direction to the given angle
float	GetNearestInterCardinalAngle(float angle);								// Returns the nearest inter-cardinal direction to the given angle
Vector2	GetNearestCardinalDirection(const Vector2& direction);					// Returns the nearest cardinal direction to the one given
Vector2 GetNearestCardinalDirection(float angle);
float	GetAngleBetweenZeroThreeSixty(float angleDegrees);						// Returns an angle coterminal to angleDegrees between 0 and 360

// Random functions
float	GetRandomFloatInRange(float minInclusive, float maxInclusive);			// Generates a random float between minInclusive and maxInclusive
int		GetRandomIntInRange(int minInclusive, int maxInclusive);				// Generates a random int between minInclusive and maxInclusive
float	GetRandomFloatZeroToOne();												// Generates a random float between zero and one
int		GetRandomIntLessThan(int maxExclusive);									// Generates a random int between zero (inclusive) and maxExclusive
bool	GetRandomTrueOrFalse();													// Randomly returns true or false
bool	CheckRandomChance(float chanceForSuccess);								// Returns true if we generate a number less than chanceForSuccess,  between 0.f and 1.f
Vector2 GetRandomPointWithinCircle();
Vector3 GetRandomPointOnSphere();													// Returns a random unit vector
Vector3 GetRandomPointWithinSphere();

// Rounding, clamping, and range mapping
int		RoundToNearestInt(float inValue);										// 0.5 rounds up to 1, -0.5 rounds up to 0
int		ClampInt(int inValue, int minInclusive, int maxInclusive);				// Clamps inValue to be between min and max, inclusive
float	ClampFloat(float inValue, float minInclusive, float maxInclusive);		// Clamps inValue to be between min and max, inclusive
float	ClampFloatZeroToOne(float inValue);										// Clamps inValue to be between 0 and 1, inclusive
float	ClampFloatNegativeOneToOne(float inValue);								// Clamps inValue to be between -1 and 1, inclusive
Vector2	ClampVector2(const Vector2& inValue, const Vector2& minInclusive, const Vector2& maxInclusive);
Vector3 ClampVector3(const Vector3& inValue, float minInclusive, float maxInclusive);

float	GetFractionInRange(float inValue, float rangeStart, float rangeEnd);	// Finds the % (as fraction) of inValue in [rangeStart, rangeEnd]																

float	RangeMapFloat(float inValue, float inStart, float inEnd, float outStart, float outEnd);		// Maps inValue from an inRange to an outRange
Vector2 RangeMap(Vector2 inValue, Vector2 inStart, Vector2 inEnd, Vector2 outStart, Vector2 outEnd);

// Angle and vector utilities
float	GetAngularDisplacement(float startDegrees, float endDegrees);					// Returns the angular displacement from start to end, taking the shorter path
float	TurnToward(float currentDegrees, float goalDegrees, float maxTurnDegrees);		// Returns an angle that is at most maxTurn from current towards goal 
float	DotProduct(const Vector2& a, const Vector2& b);									// Returns the dot product between a and b
float	DotProduct(const Vector3& a, const Vector3& b);	
float	DotProduct(const Vector4& a, const Vector4& b);
Vector3 CrossProduct(const Vector3& a, const Vector3& b);								// Returns the cross product between a and b
Vector3 Reflect(const Vector3& incidentVector, const Vector3& normal);					// Reflects the incident vector about the normal
bool	Refract(const Vector3& incidentVector, const Vector3& normal, float niOverNt, Vector3& out_refractedVector); // Returns true if the given vector will refract across the surface, false otherwise

// Bitflag utilities
bool	AreBitsSet(unsigned char bitFlags8, unsigned char flagsToCheck);				// Checks to see if the bits are set in the 8 bit flag
bool	AreBitsSet(unsigned int bitFlags32, unsigned int flagsToCheck);					// Checks to see if the bits are set in the 32 bit flag
void	SetBits(unsigned char& bitFlags8, unsigned char flagsToSet);					// Sets the bits indicated by flagsToSet in the 8 bit flag
void	SetBits(unsigned int& bitFlags32, unsigned int flagsToSet);						// Sets the bits indicated by flagsToSet in the 32 bit flag
void	ClearBits(unsigned char& bitFlags8, unsigned char flagsToClear);				// Clears the bits indicated by flagsToSet in the 8 bit flag
void	ClearBits(unsigned int& bitFlags32, unsigned int flagsToClear);					// Clears the bits indicated by flagsToSet in the 32 bit flag
uint32_t GetBitsReversed(const uint32_t& bits);

// Easing Functions
float	SmoothStart2(float t);		// 2nd-degree smooth start (a.k.a "quadratic ease in")
float	SmoothStart3(float t);		// 3rd-degree smooth start (a.k.a "cubic ease in")
float	SmoothStart4(float t);		// 4th-degree smooth start (a.k.a "quartic ease in")

float	SmoothStop2(float t);		// 2nd-degree smooth start (a.k.a "quadratic ease out")
float	SmoothStop3(float t);		// 3rd-degree smooth start (a.k.a "cubic ease out")
float	SmoothStop4(float t);		// 4th-degree smooth start (a.k.a "quartic ease out")

float	SmoothStep3(float t);		// 3nd-degree smooth start/stop (a.k.a "smoothstep")


// Interpolation -- Finds the value at a certain percent (fraction) in [start, end]	
float				Interpolate(float start, float end, float fractionTowardEnd);							
int					Interpolate(int start, int end, float fractionTowardEnd);
unsigned char		Interpolate(unsigned char start, unsigned char end, float fractionTowardEnd);
const IntVector2	Interpolate(const IntVector2& start, const IntVector2& end, float fractionTowardEnd);
const IntVector3	Interpolate(const IntVector3& start, const IntVector3& end, float fractionTowardEnd);
const IntRange		Interpolate(const IntRange& start, const IntRange& end, float fractionTowardEnd);
const Rgba			Interpolate(const Rgba& start, const Rgba& end, float fractionTowardEnd);


// Polynomials
bool Quadratic(Vector2& out_solutions, float a, float b, float c);


// General functions
int			MinInt(int a, int b);
int			MaxInt(int a, int b);

unsigned int MinInt(unsigned int a, unsigned int b);
unsigned int MaxInt(unsigned int a, unsigned int b);
float	MaxFloat(float a, float b, float c, float d);
float	MinFloat(float a, float b, float c, float d);
float	MinFloat(float a, float b);
float	MaxFloat(float a, float b);
int		AbsoluteValue(int inValue);
float	AbsoluteValue(float inValue);
Vector3 AbsoluteValue(const Vector3& inValue);

// Float comparison function
bool AreMostlyEqual(float a, float b, float epsilon = 0.0001f);
bool AreMostlyEqual(const Vector3& a, const Vector3& b, float epsilon = 0.0001f);
bool AreMostlyEqual(const Matrix44& a, const Matrix44& b, float epsilon = 0.0001f);

// Misc
bool DoSpheresOverlap(const Vector3& posA, float radiusA, const Vector3& posB, float radiusB);
bool DoesBoxSphereOverlap(const AABB3& boxBounds, const Vector3& sphereCenter, float sphereRadius);