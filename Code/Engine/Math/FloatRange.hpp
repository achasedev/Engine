/************************************************************************/
/* File: FloatRange.hpp
/* Author: Andrew Chase
/* Date: October 25th, 2017
/* Bugs: None
/* Description: Class to represent a linear range of float values
				Inclusive of endpoints
/************************************************************************/
#pragma once


class FloatRange
{
public:
	//-----Public Methods-----
	
	FloatRange();
	FloatRange(float initialMin, float initialMax);
	FloatRange(float initialMinMax);

	float GetRandomInRange() const;
	void  SetFromText(const char* text);

	static bool DoRangesOverlap(const FloatRange& a, const FloatRange& b);

public:
	//-----Public Data-----

	float min;
	float max;
};

// Interpolates by interpolating the endpoints
const FloatRange Interpolate(const FloatRange& start, const FloatRange& end, float fractionTowardEnd);