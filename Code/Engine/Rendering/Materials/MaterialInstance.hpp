/************************************************************************/
/* File: MaterialInstance.hpp
/* Author: Andrew Chase
/* Date: April 28th, 2018
/* Description: Class to represent a single mutable material instance
/************************************************************************/
#pragma once
#include "Engine/Rendering/Materials/Material.hpp"

class MaterialInstance : public Material
{
public:
	//-----Public Methods-----

	MaterialInstance(const Material* copyMaterial);

	// Mutators
	void ResetToBaseMaterial();


private:
	//-----Private Data-----

	const Material* m_baseMaterial;

};
