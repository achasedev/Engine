/************************************************************************/
/* File: TextureCube.hpp
/* Author:
/* Date: May 7th, 2017
/* Bugs: None
/* Description: Class to represent a 3D Texture Cube Map on GPU
/************************************************************************/
#pragma once
#include <string>
#include "Engine/Rendering/OpenGL/glTypes.hpp"
#include "Engine/Rendering/Resources/Texture.hpp"

class Image;

class TextureCube : public Texture // Extend to get a texture handle, and be able to make skybox materials
{
public:
	//-----Public Methods-----

	TextureCube();
	~TextureCube();

	// Constructs differently
	virtual bool CreateFromFile(const std::string& filename, bool useMipMaps = false) override;
	virtual void CreateFromImage(const Image* image, bool useMipMaps = false) override;


private:
	//-----Private Methods-----

	void BindImageToSide(TexCubeSide side, const Image& image, unsigned int xOffset, unsigned int yOffset);

};
