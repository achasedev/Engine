/************************************************************************/
/* File: MaterialProperty_Vector2.hpp
/* Author: Andrew Chase
/* Date: April 23nd, 2018
/* Description: Class to represent a single material Vector2 property
/************************************************************************/
#pragma once
#include <vector>
#include "Engine/Rendering/Buffers/UniformBuffer.hpp"

class PropertyBlockDescription;

class MaterialPropertyBlock : public UniformBuffer
{
public:
	//-----Public Methods-----

	MaterialPropertyBlock(const PropertyBlockDescription* description);
	MaterialPropertyBlock(const MaterialPropertyBlock& copyBlock);

	// Accessors
	std::string						GetName() const;
	const PropertyBlockDescription* GetDescription() const { return m_description; }


private:
	//-----Private Data-----

	const PropertyBlockDescription* m_description; // Descriptor for this data block
	
};
