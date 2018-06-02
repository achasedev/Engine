/************************************************************************/
/* File: PropertyDescription.cpp
/* Author: Andrew Chase
/* Date: April 23rd, 2018
/* Description: Implementation of the PropertyDescription class
/************************************************************************/
#include "Engine/Rendering/Shaders/PropertyDescription.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
PropertyDescription::PropertyDescription(const std::string& name, size_t offset, size_t byteSize)
	: m_name(name), m_offset(offset), m_byteSize(byteSize)
{
}


//-----------------------------------------------------------------------------------------------
// Copy constructor
//
PropertyDescription::PropertyDescription(const PropertyDescription& copyDescription)
	: m_byteSize(copyDescription.m_byteSize)
	, m_name(copyDescription.m_name)
	, m_offset(copyDescription.m_offset)
{
}
