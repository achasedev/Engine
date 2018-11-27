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
#include <string>

// Defining the constant color values
const Rgba Rgba::WHITE			= Rgba();
const Rgba Rgba::CYAN			= Rgba(0, 255, 255, 255);
const Rgba Rgba::MAGENTA		= Rgba(255, 0, 255, 255);
const Rgba Rgba::YELLOW			= Rgba(255, 255, 0, 255);
const Rgba Rgba::RED			= Rgba(255, 0, 0, 255);
const Rgba Rgba::BLUE			= Rgba(0, 0, 255, 255);
const Rgba Rgba::ORANGE			= Rgba(255, 128, 0, 255);
const Rgba Rgba::PURPLE			= Rgba(128, 0, 255, 255);
const Rgba Rgba::GREEN			= Rgba(0, 255, 0, 255);
const Rgba Rgba::LIGHT_BLUE		= Rgba(0, 128, 255, 255);
const Rgba Rgba::BROWN			= Rgba(153, 76, 0, 255);
const Rgba Rgba::BLACK			= Rgba(0, 0, 0, 255);
const Rgba Rgba::GRAY			= Rgba(128, 128, 128, 255);
const Rgba Rgba::DARK_GREEN		= Rgba(100, 200, 0, 255);

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
// Constructs from normalized float values
//
Rgba::Rgba(float red, float green, float blue, float alpha)
{
	SetAsFloats(red, green, blue, alpha);
}


//-----------------------------------------------------------------------------------------------
// Sets the values of the rgba to the int values provided
//
Rgba::Rgba(int red, int green, int blue, int alpha)
{
	SetAsInts(red, green, blue, alpha);
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
	r = static_cast<unsigned char>(RoundToNearestInt(clampedRed));

	float clampedGreen = ClampFloat((normalizedGreen * 255.f), 0.f, 255.f);
	g = static_cast<unsigned char>(RoundToNearestInt(clampedGreen));

	float clampedBlue = ClampFloat((normalizedBlue * 255.f), 0.f, 255.f);
	b = static_cast<unsigned char>(RoundToNearestInt(clampedBlue));

	float clampedAlpha = ClampFloat((normalizedAlpha * 255.f), 0.f, 255.f);
	a = static_cast<unsigned char>(RoundToNearestInt(clampedAlpha));
}


//-----------------------------------------------------------------------------------------------
// Sets the values of the rgba to the int values corresponding to the ones given
//
void Rgba::SetAsInts(int red, int green, int blue, int alpha)
{
	r = (unsigned char)red;
	g = (unsigned char)green;
	b = (unsigned char)blue;
	a = (unsigned char)alpha;
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


//-----------------------------------------------------------------------------------------------
// Set from text, private function used to handle different cases
//
bool Rgba::SetFromText(const char* text, bool areInts, unsigned char delimiter)
{
	std::string stringText(text);

	if (areInts)
	{
		size_t firstDelim = stringText.find(delimiter);
		if (firstDelim == std::string::npos) { return false; }
		std::string redText = std::string(stringText, 0, firstDelim);
		int red = atoi(redText.c_str());

		// Green
		size_t secondDelim = stringText.find(delimiter, firstDelim + 1);
		if (secondDelim == std::string::npos) { return false; }
		std::string greenText = std::string(stringText, firstDelim + 1, secondDelim - firstDelim - 1);
		int green = atoi(greenText.c_str());

		// Blue and Alpha, if alpha is present
		int blue, alpha;
		size_t thirdDelim = stringText.find(delimiter, secondDelim + 1);
		if (thirdDelim == std::string::npos)
		{
			std::string blueText = std::string(stringText, secondDelim + 1);
			blue = atoi(blueText.c_str());
			alpha = 255;
		}
		else
		{
			std::string blueText = std::string(stringText, secondDelim + 1, thirdDelim - secondDelim - 1);
			blue = static_cast<unsigned char>(atoi(blueText.c_str()));

			std::string alphaText = std::string(stringText, thirdDelim + 1);
			alpha = static_cast<unsigned char>(atoi(alphaText.c_str()));
		}

		SetAsInts(red, green, blue, alpha);
	}
	else
	{
		// Red
		size_t firstDelim = stringText.find(delimiter);
		if (firstDelim == std::string::npos) { return false; }
		std::string redText = std::string(stringText, 0, firstDelim);
		float red = (float) atof(redText.c_str());

		// Green
		size_t secondDelim = stringText.find(delimiter, firstDelim + 1);
		if (secondDelim == std::string::npos) { return false; }
		std::string greenText = std::string(stringText, firstDelim + 1, secondDelim - firstDelim - 1);
		float green = (float) atof(greenText.c_str());

		// Blue and Alpha, if alpha is present
		float blue, alpha;
		size_t thirdDelim = stringText.find(delimiter, secondDelim + 1);
		if (thirdDelim == std::string::npos)
		{
			std::string blueText = std::string(stringText, secondDelim + 1);
			blue = (float) atof(blueText.c_str());
			alpha = 255;
		}
		else
		{
			std::string blueText = std::string(stringText, secondDelim + 1, thirdDelim - secondDelim - 1);
			blue = (float) atof(blueText.c_str());

			std::string alphaText = std::string(stringText, thirdDelim + 1);
			alpha = (float) atof(alphaText.c_str());
		}

		SetAsFloats(red, green, blue, alpha);
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
// Sets the rgba from the text given
//
bool Rgba::SetFromText(const char* text)
{
	std::string stringText(text);

	bool containsCommas = stringText.find(',') != std::string::npos;
	bool containsSpaces = stringText.find(' ') != std::string::npos;
	bool areInts = stringText.find('.') == std::string::npos;

	unsigned char delimiter;
	if (containsCommas)
	{
		delimiter = ',';
	}
	else if (containsSpaces)
	{
		delimiter = ' ';
	}
	else
	{
		// No delimiter specifed, so just return
		return false;
	}

	return SetFromText(text, areInts, delimiter);
}


//-----------------------------------------------------------------------------------------------
// Returns a random solid color
//
Rgba Rgba::GetRandomColor()
{
	unsigned char red,green,blue;

	red = (unsigned char)GetRandomIntInRange(0, 255);
	green = (unsigned char)GetRandomIntInRange(0, 255);
	blue = (unsigned char)GetRandomIntInRange(0, 255);

	return Rgba(red, green, blue, (unsigned char)255);
}


//-----------------------------------------------------------------------------------------------
// Comparison for Rgba's
//
bool Rgba::operator==(const Rgba& other) const
{
	bool rEquals = (r == other.r);
	bool gEquals = (g == other.g);
	bool bEquals = (b == other.b);
	bool aEquals = (a == other.a);

	return (rEquals && gEquals && bEquals && aEquals);
}

