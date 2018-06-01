/************************************************************************/
/* File: IntRange.hpp
/* Author: Andrew Chase
/* Date: October 25th, 2017
/* Bugs: None
/* Description: Class to represent a linear range of int values
				Inclusive of endpoints
/************************************************************************/
#pragma once

class IntRange
{
public:
	//-----Public Methods-----
	IntRange();
	IntRange(int initialMin, int initialMax);
	IntRange(int initialMinMax);

	int GetRandomInRange() const;
	bool IsValueInRange(int value) const;
	void SetFromText(const char* text);

	static bool DoRangesOverlap(const IntRange& a, const IntRange& b);

public:
	//-----Public Data-----

	int min;
	int max;
};
