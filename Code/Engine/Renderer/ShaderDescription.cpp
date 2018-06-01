/************************************************************************/
/* File: ShaderPropertyDescription.cpp
/* Author: Andrew Chase
/* Date: April 23rd, 2018
/* Description: Implementation of the ShaderPropertyInfo class
/************************************************************************/
#include "Engine/Renderer/PropertyDescription.hpp"
#include "Engine/Renderer/PropertyBlockDescription.hpp"
#include "Engine/Renderer/ShaderDescription.hpp"


//-----------------------------------------------------------------------------------------------
// Destructor
//
ShaderDescription::~ShaderDescription()
{
	int numBlocks = (int) m_blockDescriptions.size();
	for (int blockIndex = 0; blockIndex < numBlocks; ++blockIndex)
	{
		delete m_blockDescriptions[blockIndex];
	}

	m_blockDescriptions.clear();
}


//-----------------------------------------------------------------------------------------------
// Returns the information object for a given uniform block
//
const PropertyBlockDescription* ShaderDescription::GetBlockDescription(const char* blockName) const
{
	int numBlocks = (int) m_blockDescriptions.size();

	for (int blockIndex = 0; blockIndex < numBlocks; ++blockIndex)
	{
		if (m_blockDescriptions[blockIndex]->GetName() == blockName)
		{
			return m_blockDescriptions[blockIndex];
		}
	}

	// Not found, return null
	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Returns the block description at the given index
//
const PropertyBlockDescription* ShaderDescription::GetBlockDescription(int index) const
{
	return m_blockDescriptions[index];
}


//-----------------------------------------------------------------------------------------------
// Returns the info for a uniform member inside a uniform block
//
const PropertyDescription* ShaderDescription::GetPropertyDescription(const char* propertyName) const
{
	int numBlocks = (int) m_blockDescriptions.size();

	// Iterate across all blocks
	for (int blockIndex = 0; blockIndex < numBlocks; ++blockIndex)
	{
		PropertyBlockDescription* currBlock = m_blockDescriptions[blockIndex];
		int numProperties = currBlock->GetPropertyCount();

		// Iterate across all properties in the current block
		for (int propertyIndex = 0; propertyIndex < numProperties; ++propertyIndex)
		{
			const PropertyDescription* currProperty = currBlock->GetPropertyDescription(propertyIndex);

			if (currProperty->GetName() == propertyName)
			{
				return currProperty;
			}
		}
	}

	// Not found, so return null
	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Returns the number of blocks in this shader's description
//
int ShaderDescription::GetBlockCount() const
{
	return (int) m_blockDescriptions.size();
}


//-----------------------------------------------------------------------------------------------
// Adds the property info for a uniform block to the list
//
void ShaderDescription::AddPropertyBlock(PropertyBlockDescription* blockInfo)
{
	m_blockDescriptions.push_back(blockInfo);
}
