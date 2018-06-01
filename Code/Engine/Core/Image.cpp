/************************************************************************/
/* File: Image.cpp
/* Author: Andrew Chase
/* Date: November 1st, 2017
/* Bugs: None
/* Description: Implementation of the Image class, indexed as top left (0,0)
/************************************************************************/
#include "Engine/Core/Image.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "ThirdParty/stb/stb_image.h"


// 4 texel white image, used for solid color rendering
const Image Image::IMAGE_WHITE;
const Image Image::IMAGE_FLAT = Image(IntVector2(2, 2), Rgba(127, 127, 255, 255));

//-----------------------------------------------------------------------------------------------
// Explicit constructor - loads a file from disk and converts the character data to RGBA object data
//
Image::Image(const std::string& imageFilePath)
{
	m_numComponentsPerTexel = 0;		// Filled in for us to indicate how many color/alpha components the image had (e.g. 3=RGB, 4=RGBA)
	int numComponentsRequested = 0;		// don't care; we support 3 (RGB) or 4 (RGBA)

	// Load (and decompress) the image RGB(A) bytes from a file on disk
	m_imageData = stbi_load( imageFilePath.c_str(), &m_dimensions.x, &m_dimensions.y, &m_numComponentsPerTexel, numComponentsRequested );
	
	// Ensure the image could be loaded
	GUARANTEE_OR_DIE(m_imageData != nullptr, Stringf("Error: Image at path \"%s\" not found", imageFilePath.c_str()));

	// Use the data to fill the texel array
	bool hasAlpha = (m_numComponentsPerTexel == 4);
	int numPixels = (m_dimensions.x * m_dimensions.y);
	int totalNumComponents = (numPixels * m_numComponentsPerTexel);
	m_texels.reserve(numPixels);
	for (int componentIndex = 0; componentIndex < totalNumComponents; componentIndex+=m_numComponentsPerTexel)
	{
		unsigned char redValue		= m_imageData[componentIndex];
		unsigned char greenValue	= m_imageData[componentIndex + 1];
		unsigned char blueValue		= m_imageData[componentIndex + 2];
		unsigned char alphaValue	= static_cast<unsigned char>(255);
		if (hasAlpha)
		{
			alphaValue = m_imageData[componentIndex + 3];
		}

		m_texels.push_back(Rgba(redValue, greenValue, blueValue, alphaValue));	
	}
}


//-----------------------------------------------------------------------------------------------
// Default constructor, just makes a white 2x2 texel image
//
Image::Image()
	: m_dimensions(IntVector2(2,2))
	, m_numComponentsPerTexel(4)
{
	m_imageData = (unsigned char*)malloc(sizeof(unsigned char) * m_numComponentsPerTexel * 4);

	unsigned char* temp = m_imageData;
	for (int i = 0; i < 4; ++i)
	{
		m_texels.push_back(Rgba());	// Rgba default constructed to opaque white

		temp[0]		= m_texels[i].r;
		temp[1]		= m_texels[i].g;
		temp[2]		= m_texels[i].b;
		temp[3]		= m_texels[i].a;

		temp += 4;
	}
}


//-----------------------------------------------------------------------------------------------
// Constructs an image of the given dimensions and color
//
Image::Image(const IntVector2& dimensions, const Rgba& color /*= Rgba::WHITE*/)
	: m_dimensions(IntVector2(2,2))
	, m_numComponentsPerTexel(4)
{
		m_imageData = (unsigned char*)malloc(sizeof(unsigned char) * m_numComponentsPerTexel * 4);

		unsigned char* temp = m_imageData;
		for (int i = 0; i < 4; ++i)
		{
			m_texels.push_back(Rgba(127, 127, 255, 255));	// Rgba default constructed to opaque white

			temp[0]		= m_texels[i].r;
			temp[1]		= m_texels[i].g;
			temp[2]		= m_texels[i].b;
			temp[3]		= m_texels[i].a;

			temp += 4;
		}
}


//-----------------------------------------------------------------------------------------------
// Frees the image data used by this instance
//
Image::~Image()
{
	stbi_image_free((void*)m_imageData);
}


//-----------------------------------------------------------------------------------------------
// Returns the RGBA representation of the texel at (x, y)
//
Rgba Image::GetTexelColor(int x, int y) const
{
	int index = (m_dimensions.x * y) + x;

	// Safety check
	GUARANTEE_OR_DIE((index >= 0 && index < static_cast<int>(m_texels.size())), Stringf("Error - index into texels was out of range: Index was %d", index));
	return m_texels[index];
}


//-----------------------------------------------------------------------------------------------
// Returns the grayscale equivalent value of the texel color at (x, y)
//
unsigned char Image::GetTexelGrayScale(int x, int y) const
{
	Rgba color = GetTexelColor(x, y);

	unsigned char grayscale = static_cast<unsigned char>(((float) color.r + (float) color.g + (float) color.b) / 3.f);

	return grayscale;
}


//-----------------------------------------------------------------------------------------------
// Returns the width x height dimensions of this image
//
IntVector2 Image::GetDimensions() const
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
// Sets the texel at (x, y) to the color specified
//
void Image::SetTexel(int x, int y, const Rgba& color)
{
	int index = (m_dimensions.x * y) + x;

	// Safety check
	GUARANTEE_OR_DIE((index > 0 && index < static_cast<int>(m_texels.size())), Stringf("Error - index into texels was out of range: Index was %d", index));

	m_texels[index] = color;
}


//-----------------------------------------------------------------------------------------------
// Flips the image horizontally (over the X axis, making the top row the bottom row, and so on) 
// Flips both the RGBA and the raw image data formats
//
void Image::FlipVertical()
{
	// Create a temp vector for assembly
	int numTexels = m_dimensions.x * m_dimensions.y;
	std::vector<Rgba> result;
	result.reserve(numTexels);

	int numComponents = numTexels * m_numComponentsPerTexel;
	stbi_image_free((void*)m_imageData);
	m_imageData = (unsigned char*)malloc(sizeof(unsigned char) * numComponents);
	unsigned char* currentComponentLocation = m_imageData;
	bool hasAlpha = (m_numComponentsPerTexel > 3);

	for (int rowIndex = m_dimensions.y - 1; rowIndex >= 0; --rowIndex)	// Start from the last (bottom) row and work back up
	{
		for (int colIndex = 0; colIndex < m_dimensions.x; ++colIndex)
		{
			Rgba currTexel = GetTexelColor(colIndex, rowIndex);
			result.push_back(currTexel);

			*currentComponentLocation = currTexel.r;
			currentComponentLocation++;

			*currentComponentLocation = currTexel.g;
			currentComponentLocation++;

			*currentComponentLocation = currTexel.b;
			currentComponentLocation++;

			if (hasAlpha)
			{
				*currentComponentLocation = currTexel.a;
				currentComponentLocation++;
			}
		}
	}

	// Move the result into our image
	m_texels = std::move(result);
}
