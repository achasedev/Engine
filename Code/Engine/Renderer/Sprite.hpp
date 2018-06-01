/************************************************************************/
/* File: Sprite.hpp
/* Author: Andrew Chase
/* Date: February 25th, 2018
/* Description: Class to represent a single 2D sprite from a spritesheet
/************************************************************************/
#pragma once
#include <string>
#include "Engine/Math/AABB2.hpp"


class Texture;


class Sprite
{
	friend class SpriteSheet;

public:
	//-----Public Methods-----

	// Accessors
	const Texture&	GetTexture() const;
	AABB2			GetUVs() const;
	Vector2			GetPivot() const;
	Vector2			GetDimensions() const;


private:
	//-----Private Methods-----

	Sprite(const std::string& name, const Texture& texture, const AABB2& uvs, const Vector2& pivot, const Vector2& dimensions);


private:
	//-----Private Data-----

	std::string		m_name;
	const Texture&	m_texture;
	AABB2			m_uvs;
	Vector2			m_pivot;
	Vector2			m_dimensions;

};
