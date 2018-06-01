/************************************************************************/
/* Project: Game Engine
/* File: Texture.hpp
/* Author:
/* Date: September 28th, 2017
/* Bugs: None
/* Description: Class to represent a texture in game
/************************************************************************/
#pragma once
#include "Engine/Math/IntVector2.hpp"
#include <string>
#include <map>


//---------------------------------------------------------------------------
class Texture
{
	friend class Renderer; // Textures are managed by a Renderer instance

private:
	Texture( const std::string& imageFilePath ); // Use renderer->CreateOrGetTexture() instead!
	void PopulateFromData( unsigned char* imageData, const IntVector2& texelSize, int numComponents );

private:
	unsigned int								m_textureID;
	IntVector2									m_dimensions;
};


