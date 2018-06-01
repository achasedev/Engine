/************************************************************************/
/* Project: Game Engine
/* File: Texture.hpp
/* Author:
/* Date: September 28th, 2017
/* Bugs: None
/* Description: Class to represent a texture in game, UV bottom left is (0,0)
/************************************************************************/
#pragma once
#include "Engine/Math/IntVector2.hpp"
#include <string>
#include <map>

class Image;

enum TextureFormat
{
	TEXTURE_FORMAT_R8,
	TEXTURE_FORMAT_RG8,
	TEXTURE_FORMAT_RGB8,
	TEXTURE_FORMAT_RGBA8,
	TEXTURE_FORMAT_D24S8,
	NUM_TEXTURE_FORMATS
};


//---------------------------------------------------------------------------
class Texture
{
public:
	//-----Public Methods-----

	Texture(); // Texture constructor is now public, initializes everything to 0

	void CreateFromFile(const std::string& filename);
	void CreateFromImage(const Image* image);

	IntVector2		GetDimensions() const;
	unsigned int	GetHandle() const;

	// Render target related
	bool CreateRenderTarget(unsigned int width, unsigned int height, TextureFormat format);

	// Copying from one texture to another on the gpu
	static bool CopyTexture(Texture* source, Texture* destination);

private:
	//-----Private Data-----

	unsigned int		m_textureHandle;
	IntVector2			m_dimensions;
	TextureFormat		m_textureFormat;
};
