/************************************************************************/
/* File: ShaderPropertyDescription.hpp
/* Author: Andrew Chase
/* Date: April 23rd, 2018
/* Description: Class to represent the uniform data for a given shader program
/************************************************************************/
#pragma once
#include <vector>

class PropertyDescription;
class PropertyBlockDescription;

class ShaderDescription
{
public:
	//-----Public Methods-----
	
	~ShaderDescription();

	// Accessors
	const PropertyBlockDescription* GetBlockDescription(int index) const;
	const PropertyBlockDescription*	GetBlockDescription(const char* blockName) const;
	const PropertyDescription*		GetPropertyDescription(const char* propertyName) const;
	int								GetBlockCount() const;

	// Mutators
	void AddPropertyBlock(PropertyBlockDescription* blockInfo);


private:
	//-----Private Data-----

	std::vector<PropertyBlockDescription*> m_blockDescriptions;

};
