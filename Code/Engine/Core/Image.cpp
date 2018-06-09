/************************************************************************/
/* File: Image.cpp
/* Author: Andrew Chase
/* Date: November 1st, 2017
/* Bugs: None
/* Description: Implementation of the Image class, indexed as top left (0,0)
/************************************************************************/
#include "Engine/Core/Image.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
#include "ThirdParty/stb/stb_image.h"

// 4 texel white image, used for solid color rendering
const Image Image::IMAGE_WHITE;
const Image Image::IMAGE_FLAT = Image(IntVector2(2, 2), Rgba(127, 127, 255, 255));
const Image Image::IMAGE_BLACK = Image(IntVector2(2, 2), Rgba::BLACK);
const Image Image::IMAGE_DEFAULT_TEXTURE = Image(IntVector2(64, 64), IntVector2(8, 8), Rgba::BLUE, Rgba::GRAY);


//-----------------------------------------------------------------------------------------------
// Default constructor, just makes a white 2x2 texel image
//
//
Image::Image()
	: m_dimensions(IntVector2(2,2))
	, m_numComponentsPerTexel(4)
{
	m_imageData = (unsigned char*)malloc(sizeof(unsigned char) * m_numComponentsPerTexel * GetTexelCount());

	for (int index = 0; index < GetTexelCount() * m_numComponentsPerTexel; ++index)
	{
		m_imageData[index] = 255;
	}
}


//-----------------------------------------------------------------------------------------------
// Constructs an image of the given dimensions and color
//
Image::Image(const IntVector2& dimensions, const Rgba& color /*= Rgba::WHITE*/)
	: m_dimensions(dimensions)
	, m_numComponentsPerTexel(4)
{
		m_imageData = (unsigned char*)malloc(sizeof(unsigned char) * m_numComponentsPerTexel * GetTexelCount());

		for (int texelIndex = 0; texelIndex < GetTexelCount(); ++texelIndex)
		{
			int offset = texelIndex * m_numComponentsPerTexel;

			m_imageData[offset + 0] = color.r;
			m_imageData[offset + 1] = color.g;
			m_imageData[offset + 2] = color.b;
			m_imageData[offset + 3] = color.a;
		}
}


//-----------------------------------------------------------------------------------------------
// Constructs a checkerboard pattern image of the given two colors
//
Image::Image(const IntVector2& dimensions, const IntVector2& patternLayout, const Rgba& color1, const Rgba& color2)
	: m_dimensions(dimensions)
	, m_numComponentsPerTexel(4)
{
	int cellWidth	= dimensions.x / patternLayout.x;
	int cellHeight	= dimensions.y / patternLayout.y;
	IntVector2 cellDimensions = IntVector2(cellWidth, cellHeight);

	m_imageData = (unsigned char*)malloc(sizeof(unsigned char) * m_numComponentsPerTexel * GetTexelCount());

	for (int xIndex = 0; xIndex < dimensions.x; ++xIndex)
	{
		for (int yIndex = 0; yIndex < dimensions.y; ++yIndex)
		{
			int dataOffset = (dimensions.x * yIndex + xIndex) * m_numComponentsPerTexel;

			int xCellIndex = (xIndex / cellWidth);
			int yCellIndex = (yIndex / cellHeight);

			int sum = xCellIndex + yCellIndex;

			Rgba colorToUse = color2;
			if (sum % 2 == 0)
			{
				colorToUse = color1;
			}

			m_imageData[dataOffset + 0] = colorToUse.r;
			m_imageData[dataOffset + 1] = colorToUse.g;
			m_imageData[dataOffset + 2] = colorToUse.b;
			m_imageData[dataOffset + 3] = colorToUse.a;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Frees the image data used by this instance
//
Image::~Image()
{
	free((void*)m_imageData);
	m_imageData = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Loads the image from the given filepath, returning true on success and false otherwise
//
bool Image::LoadFromFile(const std::string& filepath)
{
	m_numComponentsPerTexel = 0;		// Filled in for us to indicate how many color/alpha components the image had (e.g. 3=RGB, 4=RGBA)
	int numComponentsRequested = 0;		// don't care; we support 3 (RGB) or 4 (RGBA)

	// Load (and decompress) the image RGB(A) bytes from a file on disk
	m_imageData = stbi_load(filepath.c_str(), &m_dimensions.x, &m_dimensions.y, &m_numComponentsPerTexel, numComponentsRequested);

	if (m_imageData == nullptr)
	{
		ConsoleErrorf("Couldn't load image file \"%s\"", filepath.c_str());
	}
	else if (DevConsole::GetInstance() != nullptr)
	{
		ConsolePrintf(Rgba::GREEN, "Loaded image \"%s\"", filepath.c_str());
	}

	return (m_imageData != nullptr);
}


//-----------------------------------------------------------------------------------------------
// Returns the RGBA representation of the texel at (x, y)
//
Rgba Image::GetTexelColor(int x, int y) const
{
	// Safety check
	GUARANTEE_OR_DIE(x >= 0 && y >= 0 && x < m_dimensions.x && y < m_dimensions.y, Stringf("Error: Image::SetTexel coords were out of bounds, coords were (%i, %i)", x, y));

	int index = ((m_dimensions.x * y) + x) * m_numComponentsPerTexel;

	Rgba color;
	if (m_numComponentsPerTexel >= 1) {color.r = m_imageData[index + 0]; }
	if (m_numComponentsPerTexel >= 2) {color.g = m_imageData[index + 1]; }
	if (m_numComponentsPerTexel >= 3) {color.b = m_imageData[index + 2]; }
	if (m_numComponentsPerTexel == 4) {color.a = m_imageData[index + 3]; }

	return color;
}


//-----------------------------------------------------------------------------------------------
// Returns the grayscale equivalent value of the texel color at (x, y)
//
float Image::GetTexelGrayScale(int x, int y) const
{
	Rgba color = GetTexelColor(x, y);

	float red, green, blue, alpha;
	color.GetAsFloats(red, green, blue, alpha);

	float grayscale = 0.2126f * red + 0.7152f * green + 0.0722f * blue;

	return grayscale;
}


//-----------------------------------------------------------------------------------------------
// Returns the number of texels in this image
//
int Image::GetTexelCount() const
{
	return m_dimensions.x * m_dimensions.y;
}


//-----------------------------------------------------------------------------------------------
// Returns the width x height dimensions of this image
//
IntVector2 Image::GetTexelDimensions() const
{
	return m_dimensions;
}


//-----------------------------------------------------------------------------------------------
// Returns the number of values used for each texel (3 for RGB, 4 for RGBA, etc)
//
int Image::GetNumComponentsPerTexel() const
{
	return m_numComponentsPerTexel;
}


//-----------------------------------------------------------------------------------------------
// Returns the raw string image data 
//
const unsigned char* Image::GetImageData() const
{
	return m_imageData;
}


//-----------------------------------------------------------------------------------------------
// Returns the flag indicating whether or not this image has been flipped to be used for textures
//
bool Image::IsFlippedForTextures() const
{
	return m_isFlippedForTextures;
}


//-----------------------------------------------------------------------------------------------
// Sets the texel at (x, y) to the color specified
//
void Image::SetTexel(int x, int y, const Rgba& color)
{
	// Safety check
	GUARANTEE_OR_DIE(x >= 0 && y >= 0 && x < m_dimensions.x && y < m_dimensions.y, Stringf("Error: Image::SetTexel coords were out of bounds, coords were (%i, %i)", x, y));

	int index = ((m_dimensions.x * y) + x) * m_numComponentsPerTexel;

	if (m_numComponentsPerTexel >= 1) { m_imageData[index + 0] = color.r; }
	if (m_numComponentsPerTexel >= 2) { m_imageData[index + 1] = color.g; }
	if (m_numComponentsPerTexel >= 3) { m_imageData[index + 2] = color.b; }
	if (m_numComponentsPerTexel == 4) { m_imageData[index + 3] = color.a; }
}


//-----------------------------------------------------------------------------------------------
// Flips the image horizontally (over the X axis, making the top row the bottom row, and so on) 
//
void Image::FlipVertical()
{
	// Create a temp vector for assembly
	int numTexels = GetTexelCount();

	int numComponents = numTexels * m_numComponentsPerTexel;
	unsigned char* newBuffer = (unsigned char*)malloc(sizeof(unsigned char) * numComponents);

	for (int rowIndex = m_dimensions.y - 1; rowIndex >= 0; --rowIndex)	// Start from the last (bottom) row and work back up
	{
		for (int colIndex = 0; colIndex < m_dimensions.x; ++colIndex)
		{
			Rgba currTexel = GetTexelColor(colIndex, rowIndex);

			// Temporarily swap the buffers to make use of the SetTexel function
			unsigned char* temp = m_imageData;
			m_imageData = newBuffer;
			SetTexel(colIndex, m_dimensions.y - rowIndex - 1, currTexel);
			m_imageData = temp;
		}
	}

	free(m_imageData);
	m_imageData = newBuffer;

	m_isFlippedForTextures = !m_isFlippedForTextures;
}
