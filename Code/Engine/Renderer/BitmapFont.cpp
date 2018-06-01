/************************************************************************/
/* File: BitmapFont.cpp
/* Author: Andrew Chase
/* Date: October 31st, 2017
/* Bugs: None
/* Description: Implementation of the BitmapFont class
/************************************************************************/
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"


//-----------------------------------------------------------------------------------------------
// Base constructor
//
BitmapFont::BitmapFont(const SpriteSheet& glyphSheet, float baseAspect)
	: m_spriteSheet(glyphSheet)
	, m_baseAspect(baseAspect)
{
}


//-----------------------------------------------------------------------------------------------
// Returns the UVs of the given glyph in the glyph spritesheet (top-left, bottom-right corners)
//
AABB2 BitmapFont::GetGlyphUVs(int glyphID) const
{
	return m_spriteSheet.GetTexUVsFromSpriteIndex(glyphID);
}


//-----------------------------------------------------------------------------------------------
// Returns the width of the given string, taking into consideration the base aspect, aspect scale,
// and cell height of the font
//
float BitmapFont::GetStringWidth(const std::string& asciiText, float cellHeight, float aspectScale)
{
	float totalWidth = 0.f;

	for (int charIndex = 0; charIndex < static_cast<int>(asciiText.length()); charIndex++)
	{
		float glyphAspect = GetGlyphAspect();
		float glyphWidth = (glyphAspect * cellHeight) * aspectScale;
		totalWidth += glyphWidth;
	}

	return totalWidth;
}


//-----------------------------------------------------------------------------------------------
// Returns the spritesheet of this font
//
const SpriteSheet& BitmapFont::GetSpriteSheet() const
{
	return m_spriteSheet;
}
