/************************************************************************/
/* Project: Game Engine
/* File: Texture.hpp
/* Author:
/* Date: September 28th, 2017
/* Bugs: None
/* Description: Class to represent a texture in game, UV bottom left is (0,0)
/************************************************************************/
#pragma once
#include <map>
#include <string>
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Rendering/OpenGL/glTypes.hpp"

class Image;

//---------------------------------------------------------------------------
class Texture
{
public:
	//-----Public Methods-----

	Texture(); // Texture constructor is now public, initializes everything to 0, needed for render targets
	~Texture();

	// Only the AssetDatabase can create textures for use other than render targets
	virtual bool CreateFromFile(const std::string& filename, bool useMipMaps = false);
	virtual void CreateFromImage(const Image* image, bool useMipMaps = false);
	virtual void CreateFromRawData(const IntVector2& dimensions, unsigned int numComponents, const unsigned char* imageData, bool useMipMaps);
	
	IntVector2		GetDimensions() const;
	unsigned int	GetHandle() const;
	TextureType		GetTextureType() const;

	// Render target related
	bool CreateRenderTarget(unsigned int width, unsigned int height, TextureFormat format);

	// Copying from one texture to another on the gpu
	static bool CopyTexture(Texture* source, Texture* destination);


protected:
	//-----Protected Data-----

	unsigned int		m_textureHandle;
	IntVector2			m_dimensions;
	TextureFormat		m_textureFormat;
	TextureType			m_textureType;

	bool				m_isUsingMipMaps;

};
