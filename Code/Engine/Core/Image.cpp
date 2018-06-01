/************************************************************************/
/* File: Image.cpp
/* Author: Andrew Chase
/* Date: November 1st, 2017
/* Bugs: None
/* Description: Implementation of the Image class
/************************************************************************/
#include "Engine/Core/Image.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "ThirdParty/stb/stb_image.h"


//-----------------------------------------------------------------------------------------------
// Explicit constructor - loads a file from disk and converts the character data to RGBA object data
//
Image::Image(const std::string& imageFilePath)
{
	int numComponentsPerPixel = 0;	// Filled in for us to indicate how many color/alpha components the image had (e.g. 3=RGB, 4=RGBA)
	int numComponentsRequested = 0; // don't care; we support 3 (RGB) or 4 (RGBA)

	// Load (and decompress) the image RGB(A) bytes from a file on disk
	unsigned char* imageData = stbi_load( imageFilePath.c_str(), &m_dimensions.x, &m_dimensions.y, &numComponentsPerPixel, numComponentsRequested );

	// Ensure the image could be loaded
	GUARANTEE_OR_DIE(imageData != nullptr, Stringf("Error: Image at path \"%s\" not found", imageFilePath.c_str()));

	// Use the data to fill the texel array
	bool hasAlpha = (numComponentsPerPixel == 4);
	int numPixels = (m_dimensions.x * m_dimensions.y);
	int totalNumComponents = (numPixels * numComponentsPerPixel);

	for (int componentIndex = 0; componentIndex < totalNumComponents; componentIndex+=numComponentsPerPixel)
	{
		unsigned char redValue		= imageData[componentIndex];
		unsigned char greenValue	= imageData[componentIndex + 1];
		unsigned char blueValue		= imageData[componentIndex + 2];
		unsigned char alphaValue	= static_cast<unsigned char>(255);
		if (hasAlpha)
		{
			hasAlpha = imageData[componentIndex + 3];
		}

		m_texels.push_back(Rgba(redValue, greenValue, blueValue, alphaValue));	
	}
	stbi_image_free( imageData );
}


//-----------------------------------------------------------------------------------------------
// Returns the RGBA representation of the texel at (x, y)
//
Rgba Image::GetTexel(int x, int y) const
{
	int index = (m_dimensions.x * y) + x;

	// Safety check
	GUARANTEE_OR_DIE((index >= 0 && index < static_cast<int>(m_texels.size())), Stringf("Error - index into texels was out of range: Index was %d", index));
	return m_texels[index];
}


//-----------------------------------------------------------------------------------------------
// Returns the width x height dimensions of this image
//
IntVector2 Image::GetDimensions() const
{
	return m_dimensions;
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
