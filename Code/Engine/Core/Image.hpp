/************************************************************************/
/* File: Image.hpp
/* Author: Andrew Chase
/* Date: November 1st, 2017
/* Bugs: None
/* Description: Class to represent an RGB/RGBA image in memory
/************************************************************************/
#pragma once
#include <string>
#include <vector>
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/IntVector2.hpp"

class Image
{
public:
	//-----Public Methods-----

	Image();
	explicit Image(const std::string& imageFilePath);
	~Image();

	Rgba GetTexelColor(int x, int y) const;
	unsigned char GetTexelGrayScale(int x, int y) const;
	IntVector2 GetDimensions() const;
	int GetNumComponentsPerTexel() const;
	const unsigned char* GetImageData() const;

	void SetTexel( int x, int y, const Rgba& color );
	void FlipVertical();

public:
	//-----Public Data-----

	const static Image IMAGE_WHITE;

private:
	//-----Private Data-----

	IntVector2 m_dimensions;		// Width and height of the image in texels
	int m_numComponentsPerTexel;	// The number of values for each texel (3 for RGB, 4 for RGBA, etc)
		
	std::vector<Rgba> m_texels;		// Ordered left to right, then down, so (0,0) is the texel at m_texels[0]
	unsigned char* m_imageData;		// Raw string data of the image from stbi

};
