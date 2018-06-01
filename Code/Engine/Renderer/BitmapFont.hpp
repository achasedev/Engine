/************************************************************************/
/* File: BitmapFont.hpp
/* Author: Andrew Chase
/* Date: October 31st, 2017
/* Bugs: None
/* Description: Class to represent a font spritesheet
/************************************************************************/
#pragma once
#include "Engine/Renderer/SpriteSheet.hpp"

// Enum to be used later for varied-alignment texts
enum FontAlignment
{
	FONT_ALIGNMENT_ERROR = -1,
	FONT_ALIGNMENT_LEFT,
	FONT_ALIGNMENT_CENTERED,
	FONT_ALIGNMENT_RIGHT,
	NUM_FONT_ALIGNMENTS
};

class BitmapFont
{
public:
	//-----Public Methods-----

	// Constructor is private, so only the renderer can construct BitmapFonts
	friend class Renderer;

	AABB2 GetGlyphUVs(int glyphID) const;
	float GetGlyphAspect() const { return m_baseAspect; }	// Will replace later when we implement varied aspects
	float GetStringWidth(const std::string& asciiText, float cellHeight, float aspectScale);

private:
	//-----Private Methods-----

	explicit BitmapFont(const SpriteSheet& glyphSheet, float baseAspect);

private:
	//-----Private Data-----

	const SpriteSheet& m_spriteSheet;	// The spritesheet of the font, assumed to be 16x16
	float m_baseAspect;					// The base width:height ratio of the font
};