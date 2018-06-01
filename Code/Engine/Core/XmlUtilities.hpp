/************************************************************************/
/* File: XmlUtilities.hpp
/* Author: Andrew Chase
/* Date: October 28th, 2017
/* Bugs: None
/* Description: Collection of functions used to parse text to various data
				structures
/************************************************************************/
#pragma once
#include "ThirdParty/tinyxml2/tinyxml2.h"
#include <string>

typedef tinyxml2::XMLElement XMLElement;
class Rgba;
class Vector2;
class IntRange;
class FloatRange;
class IntVector2;
class AABB2;

int				ParseXmlAttribute( const XMLElement& element, const char* attributeName, int defaultValue );
char			ParseXmlAttribute( const XMLElement& element, const char* attributeName, char defaultValue );
bool			ParseXmlAttribute( const XMLElement& element, const char* attributeName, bool defaultValue );
float			ParseXmlAttribute( const XMLElement& element, const char* attributeName, float defaultValue );
Rgba			ParseXmlAttribute( const XMLElement& element, const char* attributeName, const Rgba& defaultValue );
Vector2			ParseXmlAttribute( const XMLElement& element, const char* attributeName, const Vector2& defaultValue );
IntRange		ParseXmlAttribute( const XMLElement& element, const char* attributeName, const IntRange& defaultValue );
FloatRange		ParseXmlAttribute( const XMLElement& element, const char* attributeName, const FloatRange& defaultValue );
IntVector2		ParseXmlAttribute( const XMLElement& element, const char* attributeName, const IntVector2& defaultValue );
AABB2			ParseXmlAttribute( const XMLElement& element, const char* attributeName, const AABB2& defaultValue );
std::string		ParseXmlAttribute( const XMLElement& element, const char* attributeName, const std::string& defaultValue );
std::string		ParseXmlAttribute( const XMLElement& element, const char* attributeName, const char* defaultValue=nullptr );
