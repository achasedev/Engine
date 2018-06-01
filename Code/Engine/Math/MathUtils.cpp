/************************************************************************/
/* Project: MP1 C27 A1 - Vector2, Basics, MathUtils
/* File: MathUtils.cpp
/* Author: Andrew Chase
/* Date: August 30th, 2017
/* Bugs: None
/* Description: Implementation of the MathUtils class
/************************************************************************/
#include "Engine/Math/MathUtils.hpp"
#include <math.h>
#include <cstdlib>


//-----------------------------------------------------------------------------------------------
// Converts an angle in radians to its degree measure and returns it
//
float ConvertRadiansToDegrees(float radians) 
{
	float pi = 3.1415926535897932384626433832795f;
	return radians * (180.f / pi);
}


//-----------------------------------------------------------------------------------------------
// Converts an angle in degrees to its radian measure and returns it
//
float ConvertDegreesToRadians(float degrees)
{
	float pi = 3.1415926535897932384626433832795f;
	return degrees * (pi / 180.f);
}


//-----------------------------------------------------------------------------------------------
// Calculates the cosine of an angle in degrees and returns it
//
float CosDegrees(float degrees)
{
	float radians = ConvertDegreesToRadians(degrees);

	return cosf(radians);
}


//-----------------------------------------------------------------------------------------------
// Calculates the sine of an angle in degrees and returns it
//
float SinDegrees(float degrees) 
{
	float radians = ConvertDegreesToRadians(degrees);

	return sinf(radians);
}


//-----------------------------------------------------------------------------------------------
// Calculates the arctan(y,x), expressed in degrees
//
float Atan2Degrees(float y, float x) 
{
	float radians = atan2f(y, x);

	return ConvertRadiansToDegrees(radians);
}


//-----------------------------------------------------------------------------------------------
// Generates a random float between zero and one (inclusively) and returns it
//
float GetRandomFloatZeroToOne()
{
	return static_cast<float>(rand()) * (1.f / static_cast<float>(RAND_MAX));
}


//-----------------------------------------------------------------------------------------------
// Generates a random float between minInclusive and maxInclusive and returns it
//
float GetRandomFloatInRange(float minInclusive, float maxInclusive)
{
	float ratio = GetRandomFloatZeroToOne();

	return ((maxInclusive - minInclusive) * ratio) + minInclusive;
}


//-----------------------------------------------------------------------------------------------
// Generates a random integer between zero (inclusive) and maxNotInclusive
//
int GetRandomIntLessThan(int maxNotInclusive)
{
	return rand() % maxNotInclusive;
}


//-----------------------------------------------------------------------------------------------
// Generates a random integer between minInclusive and maxInclusive and returns it
//
int GetRandomIntInRange(int minInclusive, int maxInclusive)
{
	return (rand() % (maxInclusive - minInclusive + 1)) + minInclusive; 
}


//-----------------------------------------------------------------------------------------------
// Randomly returns true or false (essentially a coin flip)
//
bool GetRandomTrueOrFalse()
{
	return ((rand() % 2) == 0);
}


//-----------------------------------------------------------------------------------------------
// Returns true based on the percent chance of chanceForSuccess
//
bool CheckRandomChance(float chanceForSuccess)
{
	if (chanceForSuccess >= 1.f)
	{
		return true;
	}
	else if (chanceForSuccess <= 0.f)
	{
		return false;
	}
	else
	{
		float outcome = GetRandomFloatZeroToOne();
		return (outcome <= chanceForSuccess);
	}
}


//-----------------------------------------------------------------------------------------------
// Rounds inValue to the nearest integer, with 0.5 rounding to 1, -0.5 rounding to 0
//
int RoundToNearestInt(float inValue)
{
	int castedInt = static_cast<int>(inValue);
	float fraction = (inValue - castedInt);

	int toReturn = castedInt;

	if (fraction >= 0.5)
	{
		toReturn = castedInt + 1;
	}
	

	if (fraction < -0.5f)
	{
		toReturn = castedInt - 1;
	}
	
	return toReturn;
	
}


//-----------------------------------------------------------------------------------------------
// Clamps inValue to be between minInclusive and maxInclusive
//
int ClampInt(int inValue, int minInclusive, int maxInclusive)
{
	if (inValue > maxInclusive)
	{
		return maxInclusive;
	}
	else if (inValue < minInclusive)
	{
		return minInclusive;
	}
	else
	{
		return inValue;
	}
}


//-----------------------------------------------------------------------------------------------
// Clamps inValue to be between minInclusive and maxInclusive
//
float ClampFloat(float inValue, float minInclusive, float maxInclusive)
{
	if (inValue > maxInclusive)
	{
		return maxInclusive;
	}
	else if (inValue < minInclusive)
	{
		return minInclusive;
	}
	else
	{
		return inValue;
	}
}


//-----------------------------------------------------------------------------------------------
// Clamps inValue to be between 0.f and 1.f
//
float ClampFloatZeroToOne(float inValue)
{
	return ClampFloat(inValue, 0.f, 1.f);
}


//-----------------------------------------------------------------------------------------------
// Clamps inValue to be between -1.f and 1.f
//
float ClampFloatNegativeOneToOne(float inValue)
{
	return ClampFloat(inValue, -1.f, 1.f);
}


//-----------------------------------------------------------------------------------------------
// Finds the % (as fraction) of inValue in [rangeStart, rangeEnd]
//
float GetFractionInRange(float inValue, float rangeStart, float rangeEnd)
{
	float offsetIntoRange = (inValue - rangeStart);
	float rangeSize = (rangeEnd - rangeStart);
	return (offsetIntoRange / rangeSize);
}


//-----------------------------------------------------------------------------------------------
// Finds the value at a certain percent (fraction) in [rangeStart, rangeEnd]
//
float Interpolate(float start, float end, float fractionTowardEnd)
{
	float rangeSize = (end - start);
	float offsetIntoRange = (fractionTowardEnd * rangeSize);
	return (offsetIntoRange + start);
}


//-----------------------------------------------------------------------------------------------
// Maps inValue from an inRange to an outRange
//
float RangeMapFloat(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	// If in range has size 0, just return the average of the out range
	if (inStart == inEnd)
	{
		return (outStart + outEnd) * 0.5f;
	}

	float inRange = inEnd - inStart;
	float outRange = outEnd - outStart;

	float inRelativeToStart = inValue - inStart;

	float fractionIntoRange = inRelativeToStart / inRange;
	float outRelativeToStart = fractionIntoRange * outRange;

	return outRelativeToStart + outStart;
}


//-----------------------------------------------------------------------------------------------
// Returns the angular displacement from start to end, taking the shorter path
//
float GetAngularDisplacement(float startDegrees, float endDegrees)
{
	float angularDisp = (endDegrees - startDegrees);

	// Increment/decrement the displacement to represent the shorter turn direction
	while (angularDisp > 180)
	{
		angularDisp -= 360;
	}

	while (angularDisp < -180)
	{
		angularDisp += 360;
	}

	return angularDisp;
}


//-----------------------------------------------------------------------------------------------
// Returns an angle that is at most maxTurnDegrees from currentDegrees towards goalDegrees, in
// the direction of the shortest path. If current and goal are within maxTurn from eachother,
// goal is returned.
//
float TurnToward(float currentDegrees, float goalDegrees, float maxTurnDegrees)
{
	float angularDisp = GetAngularDisplacement(currentDegrees, goalDegrees);

	if (abs(angularDisp) <= maxTurnDegrees)
	{
		return goalDegrees;
	}
	
	float directionToTurn = (angularDisp / abs(angularDisp));
	float result = ((directionToTurn * maxTurnDegrees) + currentDegrees);

	return result;
}


//-----------------------------------------------------------------------------------------------
// Calculates the dot product between a and b
//
float DotProduct(const Vector2& a, const Vector2& b)
{
	return ((a.x * b.x) + (a.y * b.y));
}


//-----------------------------------------------------------------------------------------------
// Checks to see if the bits are set in the 8 bit flag
//
bool AreBitsSet(unsigned char bitFlags8, unsigned char flagsToCheck)
{
	return ((bitFlags8 & flagsToCheck) == flagsToCheck);
}


//-----------------------------------------------------------------------------------------------
// Checks to see if the bits are set in the 32 bit flag
//
bool AreBitsSet(unsigned int bitFlags32, unsigned int flagsToCheck)
{
	return ((bitFlags32 & flagsToCheck) == flagsToCheck);
}


//-----------------------------------------------------------------------------------------------
// Sets the bits indicated by flagsToSet in the 8 bit flag
//
void SetBits(unsigned char& bitFlags8, unsigned char flagsToSet)
{
	bitFlags8 |= flagsToSet;
}


//-----------------------------------------------------------------------------------------------
// Sets the bits indicated by flagsToSet in the 32 bit flag
//
void SetBits(unsigned int& bitFlags32, unsigned int flagsToSet)
{
	bitFlags32 |= flagsToSet;
}


//-----------------------------------------------------------------------------------------------
// Clears the bits indicated by flagsToSet in the 8 bit flag
//
void ClearBits(unsigned char& bitFlags8, unsigned char flagsToClear)
{
	unsigned char bitMask = ~flagsToClear;
	bitFlags8 &= bitMask;
}


//-----------------------------------------------------------------------------------------------
// Clears the bits indicated by flagsToSet in the 32 bit flag
//
void ClearBits(unsigned int& bitFlags32, unsigned int flagsToClear)
{
	unsigned int bitMask = ~flagsToClear;
	bitFlags32 &= bitMask;
}