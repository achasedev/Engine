/************************************************************************/
/* Project: MP1 C27 A2 - More Vector2D, Box2D, Disc2D
/* File: AABB2.cpp
/* Author: Andrew Chase
/* Date: September 4th, 2017
/* Bugs: None
/* Description: Implementation of the Box2D class
/************************************************************************/
#include <string>
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"


// Constant square values
const AABB2 AABB2::UNIT_SQUARE_CENTERED = AABB2(Vector2(-1.f, -1.f), Vector2(1.f, 1.f));
const AABB2 AABB2::HALF_UNIT_SQUARE_CENTERED = AABB2(Vector2(-0.5f, -0.5f), Vector2(0.5f, 0.5f));
const AABB2 AABB2::UNIT_SQUARE_OFFCENTER = AABB2(Vector2(0.f, 0.f), Vector2(1.f, 1.f));


//-----------------------------------------------------------------------------------------------
// Constructs a Box2D using explicit float boundaries
//
AABB2::AABB2(float minX, float minY, float maxX, float maxY)
	: mins(Vector2(minX, minY))
	, maxs(Vector2(maxX, maxY))
{
}


//-----------------------------------------------------------------------------------------------
// Constructs a Box2D using two Vector2D objects to represent the bounds
//
AABB2::AABB2(const Vector2& referenceMins, const Vector2& referenceMaxs)
	: mins(referenceMins)
	, maxs(referenceMaxs)
{
}


//-----------------------------------------------------------------------------------------------
// Constructs a Box2D using a center point and XY-offsets for boundaries
//
AABB2::AABB2(const Vector2& center, float radiusX, float radiusY)
	: mins(Vector2((center.x - radiusX), (center.y - radiusY)))
	, maxs(Vector2((center.x + radiusX), (center.y + radiusY)))
{
}


//-----------------------------------------------------------------------------------------------
// Stretches the boundaries of the box to include a point given by (x,y)
//
void AABB2::StretchToIncludePoint(float x, float y)
{
	// X direction check
	if (x < mins.x) 
	{
		mins.x = x;
	} else if (x > maxs.x) 
	{
		maxs.x = x;
	}

	// Y direction check
	if (y < mins.y) 
	{
		mins.y = y;
	} else if (y > maxs.y) 
	{
		maxs.y = y;
	}
}


//-----------------------------------------------------------------------------------------------
// Stretches the boundaries of the box to include the given Vector2D point
//
void AABB2::StretchToIncludePoint(const Vector2& point)
{
	// X check
	if (point.x < mins.x) 
	{
		mins.x = point.x;
	} else if (point.x > maxs.x) 
	{
		maxs.x = point.x;
	}

	// Y check
	if (point.y < mins.y) 
	{
		mins.y = point.y;
	} else if (point.y > maxs.y) 
	{
		maxs.y = point.y;
	}
}


//-----------------------------------------------------------------------------------------------
// Offsets the boundaries of the box linearly given by xPaddingRadius and yPaddingRadius
//
void AABB2::AddPaddingToSides(float xPaddingRadius, float yPaddingRadius)
{
	mins.x -= xPaddingRadius;
	maxs.x += xPaddingRadius;
	mins.y -= yPaddingRadius;
	maxs.y += yPaddingRadius;
}


//-----------------------------------------------------------------------------------------------
// Moves the box by moving the boundaries additively in the XY direction indicated by translation
//
void AABB2::Translate(const Vector2& translation)
{
	mins.x += translation.x;
	mins.y += translation.y;

	maxs.x += translation.x;
	maxs.y += translation.y;
}


//-----------------------------------------------------------------------------------------------
// Moves the box by moving the boundaries additively in the XY direction indicated by
// translationX and translationY
//
void AABB2::Translate(float translationX, float translationY)
{
	mins.x += translationX;
	maxs.x += translationX;

	mins.y += translationY;
	maxs.y += translationY;
}


//-----------------------------------------------------------------------------------------------
// Sets the values based on the text representation passed
//
void AABB2::SetFromText(const char* text)
{
	std::string stringText = std::string(text);

	// min x value
	int firstComma = static_cast<int>(stringText.find(","));
	std::string minXText = std::string(stringText, 0, firstComma);
	mins.x = static_cast<float>(atof(minXText.c_str()));

	// min y value
	int secondComma = static_cast<int>(stringText.find(",", firstComma + 1));
	std::string minYText = std::string(stringText, firstComma + 1, secondComma - firstComma - 1);
	mins.y = static_cast<float>(atof(minYText.c_str()));

	// max x value
	int thirdComma = static_cast<int>(stringText.find(",", secondComma + 1));
	std::string maxXText = std::string(stringText, secondComma + 1, thirdComma - secondComma - 1);
	maxs.x = static_cast<float>(atof(maxXText.c_str()));

	// max y value
	std::string maxYText = std::string(stringText, thirdComma + 1);
	maxs.y = static_cast<float>(atof(maxYText.c_str()));
}


//-----------------------------------------------------------------------------------------------
// Checks whether or not the point at (x,y) is within the bounds of the box. Returns true if it
// is, false otherwise
//
bool AABB2::IsPointInside(float x, float y) const
{
	bool isInside = true;

	if (x < mins.x || x > maxs.x) 
	{
		isInside = false;
	}

	if (y < mins.y || y > maxs.y)
	{
		isInside = false;
	}

	return isInside;
}


//-----------------------------------------------------------------------------------------------
// Checks whether or not the point at 'point' is within the bounds of the box. Returns true if it
// is, false otherwise
//
bool AABB2::IsPointInside(const Vector2& point) const
{
	bool isInside = true;

	if (point.x < mins.x || point.x > maxs.x) 
	{
		isInside = false;
	}

	if (point.y < mins.y || point.y > maxs.y)
	{
		isInside = false;
	}

	return isInside;
}


//-----------------------------------------------------------------------------------------------
// Returns the dimensions of the box as a Vector2D in the form (width, height)
//
Vector2 AABB2::GetDimensions() const
{
	Vector2 dimensions;

	dimensions.x = (maxs.x - mins.x);
	dimensions.y = (maxs.y - mins.y);

	return dimensions;
}


//-----------------------------------------------------------------------------------------------
// Returns the center point position of the box as a Vector2D
//
Vector2 AABB2::GetCenter() const
{
	Vector2 centerPoint;

	centerPoint.x = (maxs.x + mins.x) / 2.f;
	centerPoint.y = (maxs.y + mins.y) / 2.f;

	return centerPoint;
}


//-----------------------------------------------------------------------------------------------
// Returns a random point inside the box as (x,y)
//
Vector2 AABB2::GetRandomPointInside() const
{
	Vector2 randomPoint;
	
	randomPoint.x = GetRandomFloatInRange(mins.x, maxs.x);
	randomPoint.y = GetRandomFloatInRange(mins.y, maxs.y);

	return randomPoint;
}


//-----------------------------------------------------------------------------------------------
// Returns the bottom left of the AABB2
//
Vector2 AABB2::GetBottomLeft() const
{
	return mins;
}


//-----------------------------------------------------------------------------------------------
// Returns the bottom right of the AABB2
//
Vector2 AABB2::GetBottomRight() const
{
	return Vector2(maxs.x, mins.y);
}


//-----------------------------------------------------------------------------------------------
// Returns the top right of the AABB2
//
Vector2 AABB2::GetTopRight() const
{
	return maxs;
}


//-----------------------------------------------------------------------------------------------
// Returns the top left of the AABB2
//
Vector2 AABB2::GetTopLeft() const
{
	return Vector2(mins.x, maxs.y);
}


//-----------------------------------------------------------------------------------------------
// Moves the box boundaries in the direction given by translation, works the same as Translate
//
void AABB2::operator+=(const Vector2& translation)
{
	mins.x += translation.x;
	mins.y += translation.y;
	maxs.x += translation.x;
	maxs.y += translation.y;
}


//-----------------------------------------------------------------------------------------------
// Moves the box boundaries in the direction opposite of antiTranslation
//
void AABB2::operator-=(const Vector2& antiTranslation)
{
	mins.x -= antiTranslation.x;
	mins.y -= antiTranslation.y;
	maxs.x -= antiTranslation.x;
	maxs.y -= antiTranslation.y;
}


//-----------------------------------------------------------------------------------------------
// Returns a copy of this box with all corners scaled by the given scalar
//
AABB2 AABB2::operator*(float scalar) const
{
	AABB2 scaledBox;

	scaledBox.mins = mins * scalar;
	scaledBox.maxs = maxs * scalar;

	return scaledBox;
}


//-----------------------------------------------------------------------------------------------
// Returns a copy of this box after being translated in the direction of translation
//
AABB2 AABB2::operator+(const Vector2& translation) const
{
	AABB2 translatedBox;
	
	translatedBox.mins.x = (this->mins.x + translation.x);
	translatedBox.mins.y = (this->mins.y + translation.y);

	translatedBox.maxs.x = (this->maxs.x + translation.x);
	translatedBox.maxs.y = (this->maxs.y + translation.y);

	return translatedBox;
}


//-----------------------------------------------------------------------------------------------
// Returns a copy of this box after being translated in the opposite direction of antiTranslation
//
AABB2 AABB2::operator-(const Vector2& antiTranslation) const
{
	AABB2 antiTranslatedBox;

	antiTranslatedBox.mins.x = (this->mins.x - antiTranslation.x);
	antiTranslatedBox.mins.y = (this->mins.y - antiTranslation.y);

	antiTranslatedBox.maxs.x = (this->maxs.x - antiTranslation.x);
	antiTranslatedBox.maxs.y = (this->maxs.y - antiTranslation.y);

	return antiTranslatedBox;
}


//-----------------------------------------------------------------------------------------------
// Checks if the given Box2Ds a and b overlap (either their boundaries intersect, or one is contained
// in another
//
bool DoAABB2sOverlap(const AABB2& a, const AABB2& b)
{
	bool doOverlap = true;

	// Check if a is completely to the left of b
	if (a.maxs.x <= b.mins.x)
	{
		doOverlap = false;
	// Check if a is completely to the right of b
	} else if (a.mins.x >= b.maxs.x)
	{
		doOverlap = false;
	// Check if a is completely above b
	} else if (a.mins.y >= b.maxs.y)
	{
		doOverlap = false;
	// Check if a is completely below b
	} else if (a.maxs.y <= b.mins.y)
	{
		doOverlap = false;
	}

	return doOverlap;
}


//-----------------------------------------------------------------------------------------------
// Interpolates the two boxes by interpolating their mins/maxes to find the box that is
// fractionTowardEnd in between
//
const AABB2 Interpolate(const AABB2& start, const AABB2& end, float fractionTowardEnd)
{
	Vector2 interpolatedMins = Interpolate(start.mins, end.mins, fractionTowardEnd);
	Vector2 interpolatedMaxs = Interpolate(start.maxs, end.maxs, fractionTowardEnd);

	return AABB2(interpolatedMins, interpolatedMaxs);
}
