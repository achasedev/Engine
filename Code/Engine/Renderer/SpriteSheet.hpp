/************************************************************************/
/* File: SpriteSheet.hpp
/* Author: Andrew Chase
/* Date: March 14th, 2018
/* Description: Class to represent a texture atlas of sprites
/************************************************************************/
#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/XmlUtilities.hpp"

class Sprite;

class SpriteSheet
{
public:
	//-----Public Methods-----

	SpriteSheet(const std::string& name, const Texture& texture, const IntVector2& spriteLayout);		// Only constructor, uses a reference as a member
	~SpriteSheet();

	Sprite* GetSprite(const std::string& name) const;							// Returns the sprite corresponding to the given index

	AABB2 GetTexUVsFromSpriteCoords(const IntVector2& spriteCoords) const;		// Returns the texture coords given the 2D index in the atlas
	AABB2 GetTexUVsFromSpriteIndex(int spriteIndex) const;						// Returns the texture coords given the linear index into the atlas

	int GetNumSprites() const;													// Returns the number of individual sprites in the atlas
	const Texture& GetTexture() const;											// Returns a reference to the texture this sprite sheet comprises

	static SpriteSheet* LoadSpriteSheet(const std::string& filePath);
	static SpriteSheet* GetResource(const std::string& name);


private:
	//-----Private Methods-----

	void ParseSprite(const XMLElement& element);


private:
	//-----Private Data

	std::string m_name;
	const Texture& m_texture;													// The texture of this sprite sheet
	IntVector2 m_spriteLayout;													// The number of sprites in each row and column of this texture

	std::map<std::string, Sprite*> m_sprites;

	static std::map<std::string, SpriteSheet*> s_spriteSheets;
};
