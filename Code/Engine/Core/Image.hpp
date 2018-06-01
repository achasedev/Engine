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

	explicit Image(const std::string& imageFilePath);

	Rgba GetTexel(int x, int y) const;
	IntVector2 GetDimensions() const;

	void SetTexel( int x, int y, const Rgba& color );

private:
	//-----Private Data-----

	IntVector2 m_dimensions;	// Width and height of the image in texels
	std::vector<Rgba> m_texels;	// Ordered left to right, then down, so (0,0) is the texel at m_texels[0]

};