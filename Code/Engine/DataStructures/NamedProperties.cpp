/************************************************************************/
/* File: NamedProperties.cpp
/* Author: Andrew Chase
/* Date: May 2nd, 2019
/* Description: Implementation of non-templated NamedProperty class functions
/************************************************************************/
#include "Engine/DataStructures/NamedProperties.hpp"


//------------------------------------------------------------------------------
// Helper for const char* version of Set template, to ensure the string is saved off
// within this NamedProperties
//
void NamedProperties::Set(const std::string& name, const char* value)
{
	Set(name, std::string(value));
}


//------------------------------------------------------------------------------
// Helper for const char* version of Get template
//
std::string NamedProperties::Get(const std::string& name, const char* defaultValue)
{
	return Get(name, std::string(defaultValue));
}

