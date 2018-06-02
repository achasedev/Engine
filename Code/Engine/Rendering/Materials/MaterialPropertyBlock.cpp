/************************************************************************/
/* File: MaterialPropertyBlock.cpp
/* Author: Andrew Chase
/* Date: April 23nd, 2018
/* Description: Implementation of the MaterialPropertyBlock class
/************************************************************************/
#include "Engine/Rendering/Materials/MaterialPropertyBlock.hpp"
#include "Engine/Rendering/Shaders/PropertyBlockDescription.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor from a Block Description
//
MaterialPropertyBlock::MaterialPropertyBlock(const PropertyBlockDescription* description)
	: m_description(description)
{
}


//-----------------------------------------------------------------------------------------------
// Copy constructor - used with Material Instancing
//
MaterialPropertyBlock::MaterialPropertyBlock(const MaterialPropertyBlock& copyBlock)
{
	const PropertyBlockDescription* description = copyBlock.GetDescription();
	m_description = new PropertyBlockDescription(*description);

	// need to copy this Uniform buffer's CPU data as well
	m_bufferSize = copyBlock.m_bufferSize;
	m_isCPUDirty = true;	// Always treated as dirty, since this GPU buffer has no data

	m_cpuBuffer = malloc(m_bufferSize);
	std::memcpy(m_cpuBuffer, copyBlock.m_cpuBuffer, m_bufferSize);	
}


//-----------------------------------------------------------------------------------------------
// Returns the name of this block
//
std::string MaterialPropertyBlock::GetName() const
{
	return m_description->GetName();
}
