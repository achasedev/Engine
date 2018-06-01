/************************************************************************/
/* File: BitmapFont.hpp
/* Author: Andrew Chase
/* Date: October 31st, 2017
/* Bugs: None
/* Description: Class to represent a font spritesheet
/************************************************************************/
#pragma once
#include "Engine/Renderer/SpriteSheet.hpp"


class BitmapFont
{
	// Constructor is private, so only the AssetDB can construct BitmapFonts
	friend class AssetDB;

public:
	//-----Public Methods-----

	AABB2 GetGlyphUVs(int glyphID) const;
	float GetGlyphAspect() const { return m_baseAspect; }	// Will replace later when we implement varied aspects
	float GetStringWidth(const std::string& asciiText, float cellHeight, float aspectScale);

	const SpriteSheet& GetSpriteSheet() const;


private:
	//-----Private Methods-----

	explicit BitmapFont(const SpriteSheet& glyphSheet, float baseAspect);
	BitmapFont(const BitmapFont& copy) = delete;


private:
	//-----Private Data-----

	const SpriteSheet m_spriteSheet;	// The spritesheet of the font, assumed to be 16x16
	float m_baseAspect;					// The base width:height ratio of the font

};