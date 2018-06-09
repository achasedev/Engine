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
	Image(const IntVector2& dimensions, const Rgba& color = Rgba::WHITE);
	Image(const IntVector2& dimensions, const IntVector2& patternLayout, const Rgba& color1, const Rgba& color2);
	~Image();

	bool					LoadFromFile(const std::string& filepath);
	Rgba					GetTexelColor(int x, int y) const;
	float					GetTexelGrayScale(int x, int y) const;
	int						GetTexelCount() const;
	IntVector2				GetTexelDimensions() const;
	int						GetNumComponentsPerTexel() const;
	const unsigned char*	GetImageData() const;
	bool					IsFlippedForTextures() const;

	void SetTexel( int x, int y, const Rgba& color );
	void FlipVertical();


public:
	//-----Public Data-----

	const static Image IMAGE_FLAT;
	const static Image IMAGE_WHITE;
	const static Image IMAGE_BLACK;
	const static Image IMAGE_DEFAULT_TEXTURE;


private:
	//-----Private Data-----

	IntVector2 m_dimensions;		// Width and height of the image in texels
	int m_numComponentsPerTexel;	// The number of values for each texel (3 for RGB, 4 for RGBA, etc)
		
	unsigned char* m_imageData;		// Raw string data of the image from stbi
	bool m_isFlippedForTextures = false;
};
