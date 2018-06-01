/************************************************************************/
/* File: Sprite.cpp
/* Author: Andrew Chase
/* Date: February 25th, 2018
/* Description: Implementation of the Sprite class
/************************************************************************/
#include "Engine/Renderer/Sprite.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
Sprite::Sprite(const std::string& name, const Texture& texture, const AABB2& uvs, const Vector2& pivot, const Vector2& dimensions)
	: m_name(name)
	, m_texture(texture)
	, m_uvs(uvs)
	, m_pivot(pivot)
	, m_dimensions(dimensions)
{
}


//-----------------------------------------------------------------------------------------------
// Returns the Texture reference for this Sprite
//
const Texture& Sprite::GetTexture() const
{
	return m_texture;
}


//-----------------------------------------------------------------------------------------------
// Returns the texture UVs for this sprite
//
AABB2 Sprite::GetUVs() const
{
	return m_uvs;
}


//-----------------------------------------------------------------------------------------------
// Returns the pivot position for this Sprite
//
Vector2 Sprite::GetPivot() const
{
	return m_pivot;
}


//-----------------------------------------------------------------------------------------------
// Returns the width and height of this sprite in world coordinates
//
Vector2 Sprite::GetDimensions() const
{
	return m_dimensions;
}
