/************************************************************************/
/* File: PropertyBlockDescription.hpp
/* Author: Andrew Chase
/* Date: April 23rd, 2018
/* Description: Class to represent the data for a single uniform block
/************************************************************************/
#pragma once
#include <vector>
#include <string>

class PropertyDescription;

class PropertyBlockDescription
{
public:
	//-----Public Methods-----

	PropertyBlockDescription() {}
	PropertyBlockDescription(const PropertyBlockDescription& copyDescription);
	~PropertyBlockDescription();

	// Accessors
	std::string			GetName() const				{ return m_name; }
	int					GetPropertyCount() const	{ return (int) m_propertyDescriptions.size(); }
	unsigned int		GetBlockSize() const		{ return m_blockSize; }
	unsigned int		GetBlockBinding() const		{ return m_shaderBinding; }

	const PropertyDescription* GetPropertyDescription(int propertyIndex) const;
	const PropertyDescription* GetPropertyDescription(const std::string& propertyName) const;

	// Mutators
	void SetName(const char* name);
	void SetShaderBinding(unsigned int binding);
	void SetBlockSize(unsigned int byteSize);
	void AddProperty(PropertyDescription* newProperty);


private:
	//-----Private Data-----

	std::string		m_name;
	unsigned int	m_shaderBinding;	// Binding of this block in the shader program
	unsigned int	m_blockSize;

	std::vector<PropertyDescription*> m_propertyDescriptions;

};
