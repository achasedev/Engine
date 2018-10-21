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

typedef tinyxml2::XMLDocument XMLDocument;
typedef tinyxml2::XMLElement XMLElement;
typedef tinyxml2::XMLAttribute XMLAttribute;
typedef tinyxml2::XMLError XMLError;

class Rgba;
class Vector2;
class Vector3;
class IntRange;
class FloatRange;
class IntVector2;
class IntVector3;
class AABB2;

int				ParseXmlAttribute( const XMLElement& element, const char* attributeName, int defaultValue );
unsigned int	ParseXmlAttribute(const XMLElement& element, const char* attributeName, unsigned int defaultValue);
char			ParseXmlAttribute( const XMLElement& element, const char* attributeName, char defaultValue );
bool			ParseXmlAttribute( const XMLElement& element, const char* attributeName, bool defaultValue );
float			ParseXmlAttribute( const XMLElement& element, const char* attributeName, float defaultValue );
Rgba			ParseXmlAttribute( const XMLElement& element, const char* attributeName, const Rgba& defaultValue );
Vector2			ParseXmlAttribute( const XMLElement& element, const char* attributeName, const Vector2& defaultValue );
Vector3			ParseXmlAttribute(const XMLElement& element, const char* attributeName, const Vector3& defaultValue);
IntRange		ParseXmlAttribute( const XMLElement& element, const char* attributeName, const IntRange& defaultValue );
FloatRange		ParseXmlAttribute( const XMLElement& element, const char* attributeName, const FloatRange& defaultValue );
IntVector2		ParseXmlAttribute( const XMLElement& element, const char* attributeName, const IntVector2& defaultValue );
IntVector3		ParseXmlAttribute( const XMLElement& element, const char* attributeName, const IntVector3& defaultValue);
AABB2			ParseXmlAttribute( const XMLElement& element, const char* attributeName, const AABB2& defaultValue );
std::string		ParseXmlAttribute( const XMLElement& element, const char* attributeName, const std::string& defaultValue );
std::string		ParseXmlAttribute( const XMLElement& element, const char* attributeName, const char* defaultValue=nullptr );
