/************************************************************************/
/* Project: Game Engine
/* File: Rgba.hpp
/* Author: Andrew Chase
/* Date: September 17th, 2017
/* Bugs: None
/* Description: Implementation of the Rgba class
/************************************************************************/
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/MathUtils.hpp"

// Defining the constant color values
const Rgba Rgba::WHITE			= Rgba();
const Rgba Rgba::CYAN			= Rgba(0, 255, 255, 255);
const Rgba Rgba::MAGENTA		= Rgba(255, 0, 255, 255);
const Rgba Rgba::YELLOW			= Rgba(255, 255, 0, 255);
const Rgba Rgba::RED			= Rgba(255, 0, 0, 255);
const Rgba Rgba::BLUE			= Rgba(0, 0, 255, 255);
const Rgba Rgba::ORANGE			= Rgba(255, 128, 0, 255);
const Rgba Rgba::PURPLE			= Rgba(128, 0, 255, 255);
const Rgba Rgba::LIGHT_GREEN	= Rgba(0, 255, 0, 255);
const Rgba Rgba::LIGHT_BLUE		= Rgba(0, 128, 255, 255);
const Rgba Rgba::BROWN			= Rgba(153, 76, 0, 255);
const Rgba Rgba::BLACK			= Rgba(0, 0, 0, 255);
const Rgba Rgba::GRAY			= Rgba(128, 128, 128, 255);
const Rgba Rgba::GREEN			= Rgba(100, 200, 0, 255);

//-----------------------------------------------------------------------------------------------
// Default constructor - initializes it to opaque white
//
Rgba::Rgba()
	: r(static_cast<unsigned char>(255))
	, g(static_cast<unsigned char>(255))
	, b(static_cast<unsigned char>(255))
	, a(static_cast<unsigned char>(255))
{
}


//-----------------------------------------------------------------------------------------------
// Constructor for explicit byte values
//
Rgba::Rgba(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphabyte)
	: r(redByte)
	, g(greenByte)
	, b(blueByte)
	, a(alphabyte)
{
}


//-----------------------------------------------------------------------------------------------
// Sets the values of the rgba to the byte values provided
//
void Rgba::SetAsBytes(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphabyte)
{
	r = redByte;
	g = greenByte; 
	b = blueByte;
	a = alphabyte;
}


//-----------------------------------------------------------------------------------------------
// Sets the values of the rgba to the byte values corresponding to the given float values (between 0.f and 1.f)
//
void Rgba::SetAsFloats(float normalizedRed, float normalizedGreen, float normalizedBlue, float normalizedAlpha)
{
	float clampedRed = ClampFloat((normalizedRed * 255.f), 0.f, 255.f);
	r = static_cast<unsigned char>(clampedRed);

	float clampedGreen = ClampFloat((normalizedGreen * 255.f), 0.f, 255.f);
	g = static_cast<unsigned char>(clampedGreen);

	float clampedBlue = ClampFloat((normalizedBlue * 255.f), 0.f, 255.f);
	b = static_cast<unsigned char>(clampedBlue);

	float clampedAlpha = ClampFloat((normalizedAlpha * 255.f), 0.f, 255.f);
	a = static_cast<unsigned char>(clampedAlpha);
}


//-----------------------------------------------------------------------------------------------
// Getter for the rgba values as floats; parameters provided are set to the corresponding values
//
void Rgba::GetAsFloats(float& out_normalizedRed, float& out_normalizedGreen, float& out_normalizedBlue, float& out_normalizedAlpha) const
{
	out_normalizedRed = (static_cast<float>(r) * (1.f / 255.f));
	out_normalizedGreen = (static_cast<float>(g) * (1.f / 255.f));
	out_normalizedBlue = (static_cast<float>(b) * (1.f / 255.f));
	out_normalizedAlpha = (static_cast<float>(a) * (1.f / 255.f));
}


//-----------------------------------------------------------------------------------------------
// Scales (and clamps) the rgb values, but leaves alpha untouched
//
void Rgba::ScaleRGB(float rgbScale)
{
	float redFloat = static_cast<float>(r) * rgbScale;
	redFloat = ClampFloat(redFloat, 0.f, 255.f);
	r = static_cast<unsigned char>(redFloat);

	float greenFloat = static_cast<float>(g) * rgbScale;
	greenFloat = ClampFloat(greenFloat, 0.f, 255.f);
	g = static_cast<unsigned char>(greenFloat);

	float blueFloat = static_cast<float>(b) * rgbScale;
	blueFloat = ClampFloat(blueFloat, 0.f, 255.f);
	b = static_cast<unsigned char>(blueFloat);
}


//-----------------------------------------------------------------------------------------------
// Scales (and clamps) alpha by alphaScale, and leaves rgb untouched
//
void Rgba::ScaleAlpha(float alphaScale)
{
	float alphaFloat = static_cast<float>(a) * alphaScale;
	alphaFloat = ClampFloat(alphaFloat, 0.f, 255.f);
	a = static_cast<unsigned char>(alphaFloat);
}


