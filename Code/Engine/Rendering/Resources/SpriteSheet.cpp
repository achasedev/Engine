/************************************************************************/
/* File: SpriteSheet.cpp
/* Author: Andrew Chase
/* Date: March 14th, 2018
/* Description: Implementation of the SpriteSheet class
/************************************************************************/
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Resources/Sprite.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include "Engine/Rendering/Resources/SpriteSheet.hpp"

// XML Format for a spritesheet
// <spritesheet name="archer" texture="archer.png" layout="5,5"> // <--ROOT,  Layout currently not used with individual sprites
//		<sprite name="archer_f.tr.idle">
//			<ppu count="16" />
//			<uv layout="pixel" or "normalized" uvs="10,14,25,40" or "0.1,0.2,0.4,0.5" flipX="true" flipY="false"/>
//			<pivot xy="0.5,0" />
//		</sprite>
//</spritesheet>


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
// Frees the map of sprites for this spritesheet
//
SpriteSheet::~SpriteSheet()
{
	std::map<std::string, Sprite*>::iterator itr = m_sprites.begin();

	for (itr; itr != m_sprites.end(); itr++)
	{
		delete itr->second;
	}

	m_sprites.clear();
}


//-----------------------------------------------------------------------------------------------
// Returns the sprite given by name in the spritesheet
//
Sprite* SpriteSheet::GetSprite(const std::string& name) const
{
	bool spriteExists = (m_sprites.find(name) != m_sprites.end());

	if (spriteExists)
	{
		return m_sprites.at(name);
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Returns the texture coordinate UV's (bottom left, top right) given the sprite coordinates
// Sprites are indexed from bottom left being (0,0)
//
AABB2 SpriteSheet::GetTexUVsFromSpriteCoords(const IntVector2& spriteCoords) const
{
	float spriteStepX = (1.f / static_cast<float>(m_spriteLayout.x));
	float spriteStepY = (1.f / static_cast<float>(m_spriteLayout.y));

	// Finding the bottom left UVs
	Vector2 bottomLeftUVs;
	bottomLeftUVs.x = (static_cast<float>(spriteCoords.x) * spriteStepX);
	bottomLeftUVs.y = (static_cast<float>(spriteCoords.y) * spriteStepY);

	// Finding the top right UVs
	Vector2 topRightUVs;
	topRightUVs.x = bottomLeftUVs.x + spriteStepX;
	topRightUVs.y = bottomLeftUVs.y + spriteStepY;

	return AABB2(bottomLeftUVs, topRightUVs);
}


//-----------------------------------------------------------------------------------------------
// Returns the texture coordinates for the mins and maxes of a bounding box, for the specified
// sprite linear index.
// Sprites are indexed from the bottom left being 0 (?)
//
AABB2 SpriteSheet::GetTexUVsFromSpriteIndex(int spriteIndex) const
{
	IntVector2 spriteCoords;
	spriteCoords.x = (spriteIndex % m_spriteLayout.x);
	spriteCoords.y = (spriteIndex / m_spriteLayout.x);
	spriteCoords.y = m_spriteLayout.y - spriteCoords.y - 1;
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


//-----------------------------------------------------------------------------------------------
// Loads an XML file defining the sprite sheet's sprite information, adds it to the SpriteSheet
// registry, and returns it
//
SpriteSheet* SpriteSheet::LoadSpriteSheet(const std::string& filePath)
{
	// First get the general spritesheet information
	XMLDocument document;
	XMLError error = document.LoadFile(filePath.c_str());
	ASSERT_OR_DIE(error == tinyxml2::XML_SUCCESS, Stringf("Error: SpriteSheet::LoadSpriteSheet() couldn't load file \"%s\"", filePath.c_str()));

	XMLElement* rootElement = document.RootElement();

	// Name of spritesheet
	std::string name = ParseXmlAttribute(*rootElement, "name");

	// Texture
	std::string textureName = ParseXmlAttribute(*rootElement, "texture");

	Texture* texture = AssetDB::CreateOrGetTexture(textureName.c_str());

	// Layout
	IntVector2 layout = ParseXmlAttribute(*rootElement, "layout", IntVector2(1,1));

	// Create the SpriteSheet
	SpriteSheet* spriteSheet = new SpriteSheet(*texture, layout);

	// Load the sprite information into the spritesheet
	XMLElement* spriteElement = rootElement->FirstChildElement();

	while (spriteElement != nullptr)
	{
		spriteSheet->ParseSprite(*spriteElement);
		spriteElement = spriteElement->NextSiblingElement();
	}

	// Return the sprite sheet
	return spriteSheet;
}


//-----------------------------------------------------------------------------------------------
// Parses the given sprite XML element and adds the sprite to the SpriteSheet
//
void SpriteSheet::ParseSprite(const XMLElement& element)
{
	// Parse the name
	std::string spriteName = ParseXmlAttribute(element, "name");

	// Parse PPU
	const XMLElement* ppuElement = element.FirstChildElement("ppu");
	int ppu = ParseXmlAttribute(*ppuElement, "count", 16);	// Default to 16 ppu

	// Parse the pivot
	const XMLElement* pivotElement = element.FirstChildElement("pivot");
	Vector2 pivot = ParseXmlAttribute(*pivotElement, "xy", Vector2(0.5f, 0.5f));

	// Parse UV's
	const XMLElement* uvElement = element.FirstChildElement("uv");
	AABB2 uvs = ParseXmlAttribute(*uvElement, "uvs", AABB2::UNIT_SQUARE_OFFCENTER);

	// Layout
	std::string layout = ParseXmlAttribute(*uvElement, "layout");
	Vector2 spriteDimensions;
	if (layout == "normalized")
	{
		// Calculate the pixel bounds of the sprite - to find the sprite world dimensions
		IntVector2 textureDimensions = m_texture.GetDimensions();
		AABB2 pixelDimensions;

		pixelDimensions.mins.x = uvs.mins.x * textureDimensions.x;
		pixelDimensions.maxs.x = uvs.maxs.x * textureDimensions.x;

		pixelDimensions.mins.y = uvs.mins.y * textureDimensions.y;
		pixelDimensions.maxs.y = uvs.maxs.y * textureDimensions.y;

		spriteDimensions = (pixelDimensions.maxs - pixelDimensions.mins) / (float) ppu;
	}
	else if (layout == "pixel")
	{
		// First calculate the sprite world dimensions
		spriteDimensions = (uvs.maxs - uvs.mins) / (float) ppu;

		// Then convert the pixel uvs to normalized uvs
		IntVector2 textureDimensions = m_texture.GetDimensions();

		float xScalar = 1.0f / (float) textureDimensions.x;
		float yScalar = 1.0f / (float) textureDimensions.y;

		uvs.mins.x *= xScalar;
		uvs.maxs.x *= xScalar;

		uvs.mins.y *= yScalar;
		uvs.maxs.y *= yScalar;
	}

	// Check if we need to flip the UV's
	bool flipX = ParseXmlAttribute(*uvElement, "flipX", false);
	if (flipX)
	{
		float temp = uvs.mins.x;
		uvs.mins.x = uvs.maxs.x;
		uvs.maxs.x = temp;
	}

	bool flipY = ParseXmlAttribute(*uvElement, "flipY", false);
	if (flipY)
	{
		float temp = uvs.mins.y;
		uvs.mins.y = uvs.maxs.y;
		uvs.maxs.y = temp;
	}

	// Create the sprite
	Sprite* sprite = new Sprite(spriteName, m_texture, uvs, pivot, spriteDimensions);
	m_sprites[spriteName] = sprite;
}
