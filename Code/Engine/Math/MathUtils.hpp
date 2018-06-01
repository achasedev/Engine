/************************************************************************/
/* Project: MP1 C27 A1 - Vector2, Basics, MathUtils
/* File: MathUtils.hpp
/* Author: Andrew Chase
/* Date: August 30th, 2017
/* Bugs: None
/* Description: Math Utility Class for my personal game engine
/************************************************************************/
#pragma once
#include "Engine/Math/Vector2.hpp"

// Angle manipulation
float ConvertRadiansToDegrees(float radians);							// Converts an angle in radians to degrees
float ConvertDegreesToRadians(float degrees);							// converts an angle in degrees to radians
float CosDegrees(float degrees);										// Calculates the cosine of an angle in degrees
float SinDegrees(float degrees);										// Calculates the sine of an angle in degrees
float Atan2Degrees(float y, float x);									// Calculates arctan(y, x) expressed in degrees

// Random functions
float GetRandomFloatInRange(float minInclusive, float maxInclusive);	// Generates a random float between minInclusive and maxInclusive
int GetRandomIntInRange(int minInclusive, int maxInclusive);			// Generates a random int between minInclusive and maxInclusive
float GetRandomFloatZeroToOne();										// Generates a random float between zero and one
int GetRandomIntLessThan(int maxExclusive);								// Generates a random int between zero (inclusive) and maxExclusive
bool GetRandomTrueOrFalse();											// Randomly returns true or false
bool CheckRandomChance(float chanceForSuccess);							// Returns true if we generate a number less than chanceForSuccess,  between 0.f and 1.f

// Rounding, clamping, and range mapping
int RoundToNearestInt(float inValue);															// 0.5 rounds up to 1, -0.5 rounds up to 0
int ClampInt(int inValue, int minInclusive, int maxInclusive);									// Clamps inValue to be between min and max, inclusive
float ClampFloat(float inValue, float minInclusive, float maxInclusive);						// Clamps inValue to be between min and max, inclusive
float ClampFloatZeroToOne(float inValue);														// Clamps inValue to be between 0 and 1, inclusive
float ClampFloatNegativeOneToOne(float inValue);												// Clamps inValue to be between -1 and 1, inclusive
float GetFractionInRange(float inValue, float rangeStart, float rangeEnd);						// Finds the % (as fraction) of inValue in [rangeStart, rangeEnd]
float Interpolate(float start, float end, float fractionTowardEnd);								// Finds the value at a certain percent (fraction) in [rangeStart, rangeEnd]																	
float RangeMapFloat(float inValue, float inStart, float inEnd, float outStart, float outEnd);	// Maps inValue from an inRange to an outRange

// Angle and vector utilities
float GetAngularDisplacement(float startDegrees, float endDegrees);					// Returns the angular displacement from start to end, taking the shorter path
float TurnToward(float currentDegrees, float goalDegrees, float maxTurnDegrees);	// Returns an angle that is at most maxTurn from current towards goal 
float DotProduct(const Vector2& a, const Vector2& b);								// Returns the dot product between a and b

// Bitflag utilities
bool AreBitsSet(unsigned char bitFlags8, unsigned char flagsToCheck);				// Checks to see if the bits are set in the 8 bit flag
bool AreBitsSet(unsigned int bitFlags32, unsigned int flagsToCheck);				// Checks to see if the bits are set in the 32 bit flag
void SetBits(unsigned char& bitFlags8, unsigned char flagsToSet);					// Sets the bits indicated by flagsToSet in the 8 bit flag
void SetBits(unsigned int& bitFlags32, unsigned int flagsToSet);					// Sets the bits indicated by flagsToSet in the 32 bit flag
void ClearBits(unsigned char& bitFlags8, unsigned char flagsToClear);				// Clears the bits indicated by flagsToSet in the 8 bit flag
void ClearBits(unsigned int& bitFlags32, unsigned int flagsToClear);				// Clears the bits indicated by flagsToSet in the 32 bit flag
