/************************************************************************/
/* File: FloatRange.cpp
/* Author: Andrew Chase
/* Date: October 25th, 2017
/* Bugs: None
/* Description: Implementation of the FloatRange class
/************************************************************************/
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <string>

//-----------------------------------------------------------------------------------------------
// Float constructor, for min and max
//
FloatRange::FloatRange(float initialMin, float initialMax)
	: min(initialMin)
	, max(initialMax)
{
}


//-----------------------------------------------------------------------------------------------
// Single float constructor
//
FloatRange::FloatRange(float initialMinMax)
	: min(initialMinMax)
	, max(initialMinMax)
{
}


//-----------------------------------------------------------------------------------------------
// Default Constructor
//
FloatRange::FloatRange()
	: min(0.f)
	, max(0.f)
{
}


//-----------------------------------------------------------------------------------------------
// Returns a float within this FloatRange, inclusive of boundary
//
float FloatRange::GetRandomInRange() const
{
	return GetRandomFloatInRange(min, max);
}


//-----------------------------------------------------------------------------------------------
// Sets the float range based on the text representation
//
void FloatRange::SetFromText(const char* text)
{
	std::string stringText = std::string(text);

	size_t tildePosition = stringText.find("~");

	// No tilde present in text
	if (tildePosition == std::string::npos)
	{
		min = static_cast<float>(atof(std::string(stringText).c_str()));
		max = min;
	}
	else
	{
		min = static_cast<float>(atof(std::string(stringText, 0, tildePosition).c_str()));
		max = static_cast<float>(atof(std::string(stringText, tildePosition + 1).c_str()));
	}
}


//-----------------------------------------------------------------------------------------------
// Returns true if the two ranges overlap, including sharing an endpoint
//
bool FloatRange::DoRangesOverlap(const FloatRange& a, const FloatRange& b)
{
	bool doOverlap = false;

	// a's min falls inside b
	if (a.min >= b.min && a.min <= b.max)
	{
		doOverlap = true;
	}
	// a's max falls inside b
	else if (a.max >= b.min && a.max <= b.max)
	{
		doOverlap = true;
	}
	// a fully contains b
	else if (a.min <= b.min && a.max >= b.max)
	{
		doOverlap = true;
	}

	// b fully containing a is already handled in the first case

	return doOverlap;
}


//-----------------------------------------------------------------------------------------------
// Interpolates the mins and maxs
//
const FloatRange Interpolate(const FloatRange& start, const FloatRange& end, float fractionTowardEnd)
{
	float interpolatedMin = Interpolate(start.min, end.min, fractionTowardEnd);
	float interpolatedMax = Interpolate(start.max, end.max, fractionTowardEnd);

	return FloatRange(interpolatedMin, interpolatedMax);
}
