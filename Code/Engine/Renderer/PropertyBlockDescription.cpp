/************************************************************************/
/* File: PropertyBlockDescription.cpp
/* Author: Andrew Chase
/* Date: April 23rd, 2018
/* Description: Implementation of the PropertyBlockDescription class
/************************************************************************/
#include "Engine/Renderer/PropertyDescription.hpp"
#include "Engine/Renderer/PropertyBlockDescription.hpp"


//-----------------------------------------------------------------------------------------------
// Copy constructor - for Material instancing
//
PropertyBlockDescription::PropertyBlockDescription(const PropertyBlockDescription& copyDescription)
	: m_name(copyDescription.m_name)
	, m_blockSize(copyDescription.m_blockSize)
	, m_shaderBinding(copyDescription.m_shaderBinding)
{
	// Deep copy all descriptions
	int numProperties = copyDescription.GetPropertyCount();

	for (int propertyIndex = 0; propertyIndex < numProperties; ++propertyIndex)
	{
		const PropertyDescription* currProperty = copyDescription.GetPropertyDescription(propertyIndex);
		m_propertyDescriptions.push_back(new PropertyDescription(*currProperty));
	}
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
PropertyBlockDescription::~PropertyBlockDescription()
{
	int numProperties = GetPropertyCount();

	for (int propertyIndex = 0; propertyIndex < numProperties; ++propertyIndex)
	{
		delete m_propertyDescriptions[propertyIndex];
	}

	m_propertyDescriptions.clear();
}


//-----------------------------------------------------------------------------------------------
// Returns the property given by propertyName if it exists, nullptr otherwise
//
const PropertyDescription* PropertyBlockDescription::GetPropertyDescription(const std::string& propertyName) const
{
	int numProperties = (int) m_propertyDescriptions.size();

	for (int index = 0; index < numProperties; ++index)
	{
		if (m_propertyDescriptions[index]->GetName() == propertyName)
		{
			return m_propertyDescriptions[index];
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Returns the property located at index, nullptr if index out of range
//
const PropertyDescription* PropertyBlockDescription::GetPropertyDescription(int propertyIndex) const
{
	int numProperties = (int) m_propertyDescriptions.size();

	if (propertyIndex < 0 || propertyIndex >= numProperties)
	{
		return nullptr;
	}

	return m_propertyDescriptions[propertyIndex];
}


//-----------------------------------------------------------------------------------------------
// Sets the name of the block info to the one specified
//
void PropertyBlockDescription::SetName(const char* name)
{
	m_name = name;
}


//-----------------------------------------------------------------------------------------------
// Sets this block's index in the shader to the one specified
//
void PropertyBlockDescription::SetShaderBinding(unsigned int binding)
{
	m_shaderBinding = binding;
}


//-----------------------------------------------------------------------------------------------
// Sets this block's recorded size value
//
void PropertyBlockDescription::SetBlockSize(unsigned int byteSize)
{
	m_blockSize = byteSize;
}


//-----------------------------------------------------------------------------------------------
// Adds the given property to the block info
//
void PropertyBlockDescription::AddProperty(PropertyDescription* newProperty)
{
	m_propertyDescriptions.push_back(newProperty);
}
