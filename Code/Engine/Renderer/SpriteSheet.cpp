/************************************************************************/
/* Project: Incursion
/* File: SpriteSheet.cpp
/* Author: Andrew Chase
/* Date: October 3rd, 2017
/* Bugs: None
/* Description: Implementation of the SpriteSheet class
/************************************************************************/
#include "Engine/Renderer/SpriteSheet.hpp"


//-----------------------------------------------------------------------------------------------
// Only constructor for a SpriteSheet, no default (empty) constructor since the SpriteSheet
// owns a Texture reference
//
SpriteSheet::SpriteSheet(const Texture& texture, const IntVector2& spriteLayout)
	: m_texture(texture)
	, m_spriteLayout(spriteLayout)
{
}


//-----------------------------------------------------------------------------------------------
// Returns the texture coordinates for the mins and maxes of a bounding box, for the specified
// sprite coordinates (bottom left and top right)
//
AABB2 SpriteSheet::GetTexCoordFromSpriteCoords(const IntVector2& spriteCoords) const
{
	float spriteStepX = (1.f / static_cast<float>(m_spriteLayout.x));
	float spriteStepY = (1.f / static_cast<float>(m_spriteLayout.y));

	// Finding the min coordinates
	Vector2 texCoordsAtMins;
	texCoordsAtMins.x = (static_cast<float>(spriteCoords.x) * spriteStepX);
	texCoordsAtMins.y = (static_cast<float>(spriteCoords.y) * spriteStepY) + spriteStepY; // Add one extra step to get the bottom left corner, not the top left

	// Finding the max coordinates
	Vector2 texCoordsAtMaxs;
	texCoordsAtMaxs.x = (static_cast<float>(spriteCoords.x) * spriteStepX) + spriteStepX; // Add one extra step to get the top right corner, not the top left
	texCoordsAtMaxs.y = (static_cast<float>(spriteCoords.y) * spriteStepY);

	return AABB2(texCoordsAtMins, texCoordsAtMaxs);
}


//-----------------------------------------------------------------------------------------------
// Returns the texture coordinates for the mins and maxes of a bounding box, for the specified
// sprite linear index.
//
AABB2 SpriteSheet::GetTexCoordFromSpriteIndex(int spriteIndex) const
{
	IntVector2 spriteCoords;
	spriteCoords.x = (spriteIndex % m_spriteLayout.x);
	spriteCoords.y = (spriteIndex / m_spriteLayout.x);

	return GetTexCoordFromSpriteCoords(spriteCoords);
}


//-----------------------------------------------------------------------------------------------
// Returns the number of sprites in the sprite sheet
//
int SpriteSheet::GetNumSprites() const
{
	return (m_spriteLayout.x * m_spriteLayout.y);
}


//-----------------------------------------------------------------------------------------------
// Returns the texture associated with this spreadsheet
//
const Texture& SpriteSheet::GetTexture() const
{
	return m_texture;
}
