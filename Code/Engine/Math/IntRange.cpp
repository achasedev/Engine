/************************************************************************/
/* File: IntRange.cpp
/* Author: Andrew Chase
/* Date: October 25th, 2017
/* Bugs: None
/* Description: Implementation of the IntRange class
/************************************************************************/
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <string>

//-----------------------------------------------------------------------------------------------
// Int constructor, for min and max
//
IntRange::IntRange(int initialMin, int initialMax)
	: min(initialMin)
	, max(initialMax)
{
}


//-----------------------------------------------------------------------------------------------
// Single int constructor
//
IntRange::IntRange(int initialMinMax)
	: min(initialMinMax)
	, max(initialMinMax)
{
}


//-----------------------------------------------------------------------------------------------
// Default constructor
//
IntRange::IntRange()
	: min(0)
	, max(0)
{
}

//-----------------------------------------------------------------------------------------------
// Returns a random int within the range, inclusive of endpoints
//
int IntRange::GetRandomInRange() const
{
	return GetRandomIntInRange(min, max);
}


//-----------------------------------------------------------------------------------------------
// Returns true if the value falls within this range, false otherwise
// Endpoint inclusive
//
bool IntRange::IsValueInRange(int value) const
{
	return (value >= min && value <= max);
}


//-----------------------------------------------------------------------------------------------
// Sets the min and max values from the text representation passed
//
void IntRange::SetFromText(const char* text)
{
	std::string stringText = std::string(text);

	int tildePosition = static_cast<int>(stringText.find("~"));

	// No tilde present in text
	if (tildePosition == static_cast<int>(std::string::npos))
	{
		min = atoi(std::string(stringText).c_str());
		max = min;
	}
	else
	{
		min = atoi(std::string(stringText, 0, tildePosition).c_str());
		max = atoi(std::string(stringText, tildePosition + 1).c_str());
	}
}


//-----------------------------------------------------------------------------------------------
// Returns true if the two ranges overlap, including if they share an endpoint
//
bool IntRange::DoRangesOverlap(const IntRange& a, const IntRange& b)
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
