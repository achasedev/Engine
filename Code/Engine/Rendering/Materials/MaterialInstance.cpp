/************************************************************************/
/* File: MaterialInstance.cpp
/* Author: Andrew Chase
/* Date: April 28th, 2018
/* Description: Implementation of the MaterialInstance class
/************************************************************************/
#include "Engine/Rendering/Shaders/Shader.hpp"
#include "Engine/Rendering/Materials/MaterialInstance.hpp"
#include "Engine/Rendering/Materials/MaterialPropertyBlock.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
MaterialInstance::MaterialInstance(const Material* baseMaterial)
	: m_baseMaterial(baseMaterial)
{
	ResetToBaseMaterial();
}


//-----------------------------------------------------------------------------------------------
// Set all material values on this instance to match that of the base material
//
void MaterialInstance::ResetToBaseMaterial()
{
	if (m_isInstancedShader)
	{
		delete m_shader;
	}

	// Copy all the Material data
	m_isInstancedShader = m_baseMaterial->m_isInstancedShader;

	// Prevent memory leak, don't clone shared shaders
	if (m_isInstancedShader)
	{
		m_shader = m_baseMaterial->m_shader->Clone();
	}
	else
	{
		m_shader = m_baseMaterial->m_shader;
	}

	// Textures and samplers
	for (int textureIndex = 0; textureIndex < MAX_TEXTURES_SAMPLERS; ++textureIndex)
	{
		m_textures[textureIndex] = m_baseMaterial->m_textures[textureIndex];
		m_samplers[textureIndex] = m_baseMaterial->m_samplers[textureIndex];
	}

	// Deep copy uniform blocks
	int numBlocks = m_baseMaterial->GetPropertyBlockCount();
	for (int blockIndex = 0; blockIndex < numBlocks; ++blockIndex)
	{
		MaterialPropertyBlock* copyBlock = m_baseMaterial->m_propertyBlocks[blockIndex];
		m_propertyBlocks.push_back(new MaterialPropertyBlock(*copyBlock));
	}
}
