/************************************************************************/
/* Project: Incursion
/* File: SpriteSheet.hpp
/* Author: Andrew Chase
/* Date: October 3rd, 2017
/* Bugs: None
/* Description: Class to represent a texture atlas of sprites
/************************************************************************/
#pragma once
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/AABB2.hpp"

class SpriteSheet
{
public:
	//-----Public Methods-----

	SpriteSheet(const Texture& texture, const IntVector2& spriteLayout);		// Only constructor, uses a reference as a member

	AABB2 GetTexCoordFromSpriteCoords(const IntVector2& spriteCoords) const;	// Returns the texture coords given the 2D index in the atlas
	AABB2 GetTexCoordFromSpriteIndex(int spriteIndex) const;					// Returns the texture coords given the linear index into the atlas

	int GetNumSprites() const;													// Returns the number of individual sprites in the atlas
	const Texture& GetTexture() const;											// Returns a reference to the texture this sprite sheet comprises


private:
	//-----Private Data

	const Texture& m_texture;													// The texture of this sprite sheet
	IntVector2 m_spriteLayout;													// The number of sprites in each row and column of this texture
};