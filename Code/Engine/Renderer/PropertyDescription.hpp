/************************************************************************/
/* File: PropertyDescription.hpp
/* Author: Andrew Chase
/* Date: April 23rd, 2018
/* Description: Class to represent the data for a single uniform
/************************************************************************/
#pragma once
#include <string>


class PropertyDescription
{
public:
	//-----Public Methods-----

	PropertyDescription(const std::string& name, size_t offset, size_t byteSize);
	PropertyDescription(const PropertyDescription& copyDescription);

	inline std::string	GetName() const		{ return m_name; }
	inline size_t		GetOffset() const	{ return m_offset; }
	inline size_t		GetSize() const		{ return m_byteSize; }


private:
	//-----Private Data-----

	std::string m_name;
	size_t		m_offset;
	size_t		m_byteSize;

};
