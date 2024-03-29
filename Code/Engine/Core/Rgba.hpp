/************************************************************************/
/* Project: Game Engine
/* File: Rgba.hpp
/* Author: Andrew Chase
/* Date: September 17th, 2017
/* Bugs: None
/* Description: Class to represent an rgb color with alpha value
/************************************************************************/
#pragma once

class Rgba
{

public:

	// Default constructor - initializes it to opaque white
	Rgba();
	// Construct from individual byte values
	explicit Rgba(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphabyte=255);
	
	// Construct from normalized float values
	explicit Rgba(float red, float green, float blue, float alpha);

	// Construct from int values
	explicit Rgba(int red, int green, int blue, int alpha);

	// Sets all values by byte
	void SetAsBytes(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphabyte=255);

	// Sets all values by float
	void SetAsFloats(float normalizedRed, float normalizedGreen, float normalizedBlue, float normalizedAlpha=1.0f);

	// Sets all values by int
	void SetAsInts(int red, int green, int blue, int alpha);

	// Returns the values of the rgba as four floats
	void GetAsFloats(float& out_normalizedRed, float& out_normalizedGreen, float& out_normalizedBlue, float& out_normalizedAlpha) const;

	// Multiplies a scalar against the rgb values (with clamping), ignoring alpha
	void ScaleRGB(float rgbScale);

	// Multiplies a scalar against the alpha value (with clamping, ignoring red, green, and blue
	void ScaleAlpha(float alphaScale);

	// Sets the values for RGB(and optionally A) to the ones specified in the passed text
	bool SetFromText(const char* text);
	
	// Operators
	bool	operator==(const Rgba& other) const;

	static Rgba GetRandomColor();


private:

	bool SetFromText(const char* text, bool areInts, unsigned char delimiter);


public:

	unsigned char r;	// Red
	unsigned char g;	// Green
	unsigned char b;	// Blue
	unsigned char a;	// Alpha

	// Default color values
	static const Rgba WHITE;
	static const Rgba CYAN;
	static const Rgba MAGENTA;
	static const Rgba YELLOW;
	static const Rgba BLUE;
	static const Rgba LIGHT_BLUE;
	static const Rgba RED;
	static const Rgba PURPLE;
	static const Rgba ORANGE;
	static const Rgba GREEN;
	static const Rgba BROWN;
	static const Rgba BLACK;
	static const Rgba GRAY;
	static const Rgba DARK_GREEN;
};
