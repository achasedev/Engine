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
// Returns the texture coordinate UV's (top left, bottom right) given the sprite coordinates
//
AABB2 SpriteSheet::GetTexUVsFromSpriteCoords(const IntVector2& spriteCoords) const
{
	float spriteStepX = (1.f / static_cast<float>(m_spriteLayout.x));
	float spriteStepY = (1.f / static_cast<float>(m_spriteLayout.y));

	// Finding the top left UVs
	Vector2 topLeftUVs;
	topLeftUVs.x = (static_cast<float>(spriteCoords.x) * spriteStepX);
	topLeftUVs.y = (static_cast<float>(spriteCoords.y) * spriteStepY);

	// Finding the bottom right UVs
	Vector2 bottomRightUVs;
	bottomRightUVs.x = topLeftUVs.x + spriteStepX;
	bottomRightUVs.y = topLeftUVs.y + spriteStepY;

	return AABB2(topLeftUVs, bottomRightUVs);
}


//-----------------------------------------------------------------------------------------------
// Returns the texture coordinates for the mins and maxes of a bounding box, for the specified
// sprite linear index.
//
AABB2 SpriteSheet::GetTexUVsFromSpriteIndex(int spriteIndex) const
{
	IntVector2 spriteCoords;
	spriteCoords.x = (spriteIndex % m_spriteLayout.x);
	spriteCoords.y = (spriteIndex / m_spriteLayout.x);

	return GetTexUVsFromSpriteCoords(spriteCoords);
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
