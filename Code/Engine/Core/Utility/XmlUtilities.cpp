/************************************************************************/
/* File: XmlUtilities.cpp
/* Author: Andrew Chase
/* Date: October 28th, 2017
/* Bugs: None
/* Description: Collection of functions used to parse text to various data
/*				structures
/************************************************************************/
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/AABB2.hpp"

//-----------------------------------------------------------------------------------------------
// Gets an attribute value and returns it as an int
//
int ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, int defaultValue)
{
	int attributeValue = element.IntAttribute(attributeName, defaultValue);
	return attributeValue;
}


//-----------------------------------------------------------------------------------------------
// Gets an attribute value and returns it as a char
//
char ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, char defaultValue)
{
	const char* attributeText = element.Attribute(attributeName);

	if (attributeText == nullptr)
	{
		// Attribute not found
		return defaultValue;
	}
	else
	{
		return *attributeText;
	}
}


//-----------------------------------------------------------------------------------------------
// Gets an attribute value and returns it as a bool
//
bool ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, bool defaultValue)
{
	bool attributeValue = element.BoolAttribute(attributeName, defaultValue);

	return attributeValue;
}


//-----------------------------------------------------------------------------------------------
// Gets an attribute value and returns it as a float
//
float ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, float defaultValue)
{
	float attributeValue = element.FloatAttribute(attributeName, defaultValue);
	return attributeValue;
}


//-----------------------------------------------------------------------------------------------
// Gets an attribute value and returns it as an Rgba
//
Rgba ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, const Rgba& defaultValue)
{
	Rgba colorToReturn;

	const char* attributeText = element.Attribute(attributeName);

	if (attributeText == nullptr)
	{
		// Attribute not found
		return defaultValue;
	}
	else
	{
		colorToReturn.SetFromText(attributeText);
		return colorToReturn;
	}
}


//-----------------------------------------------------------------------------------------------
// Gets an attribute value and returns it as a Vector2
//
Vector2 ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, const Vector2& defaultValue)
{
	Vector2 vectorToReturn;

	const char* attributeText = element.Attribute(attributeName);

	if (attributeText == nullptr)
	{
		// Attribute not found
		return defaultValue;
	}
	else
	{
		vectorToReturn.SetFromText(attributeText);
		return vectorToReturn;
	}
}


//-----------------------------------------------------------------------------------------------
// Gets an attribute value and returns it as an IntRange
//
IntRange ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, const IntRange& defaultValue)
{
	IntRange intRangeToReturn;

	const char* attributeText = element.Attribute(attributeName);

	if (attributeText == nullptr)
	{
		// Attribute not found
		return defaultValue;
	}
	else
	{
		intRangeToReturn.SetFromText(attributeText);
		return intRangeToReturn;
	}
}


//-----------------------------------------------------------------------------------------------
// Gets an attribute value and returns it as a FloatRange
//
FloatRange ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, const FloatRange& defaultValue)
{
	FloatRange floatRangeToReturn;

	const char* attributeText = element.Attribute(attributeName);

	if (attributeText == nullptr)
	{
		// Attribute not found
		return defaultValue;
	}
	else
	{
		floatRangeToReturn.SetFromText(attributeText);
		return floatRangeToReturn;
	}
}


//-----------------------------------------------------------------------------------------------
// Gets an attribute value and returns it as a IntVector2
//
IntVector2 ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, const IntVector2& defaultValue)
{
	IntVector2 vectorToReturn;

	const char* attributeText = element.Attribute(attributeName);

	if (attributeText == nullptr)
	{
		// Attribute not found
		return defaultValue;
	}
	else
	{
		vectorToReturn.SetFromText(attributeText);
		return vectorToReturn;
	}
}


//-----------------------------------------------------------------------------------------------
// Gets an attribute value and returns it as an AABB2
//
AABB2 ParseXmlAttribute(const XMLElement& element, const char* attributeName, const AABB2& defaultValue)
{
	const char* attributeText = element.Attribute(attributeName);

	if (attributeText == nullptr)
	{
		return defaultValue;
	}
	else
	{	
		AABB2 parsedValues;
		parsedValues.SetFromText(attributeText);

		return parsedValues;
	}
}


//-----------------------------------------------------------------------------------------------
// Gets an attribute value and returns it as a string, with string default value
//
std::string ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, const std::string& defaultValue)
{
	const char* attributeText = element.Attribute(attributeName);

	if (attributeText == nullptr)
	{
		return defaultValue;
	}
	else
	{	
		return std::string(attributeText);
	}
}


//-----------------------------------------------------------------------------------------------
// Gets an attribute value and returns it as a string, with const char* default value
//
std::string ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, const char* defaultValue/*=nullptr */)
{
	const char* attributeText = element.Attribute(attributeName);

	if (attributeText == nullptr)
	{
		if (defaultValue != nullptr)
		{
			return std::string(defaultValue);
		}
		else
		{
			return std::string("");
		}
	}
	else
	{	
		return std::string(attributeText);
	}
}


//-----------------------------------------------------------------------------------------------
// Gets an attribute value and returns it as an IntVector3, with const char* default value
//
IntVector3 ParseXmlAttribute(const XMLElement& element, const char* attributeName, const IntVector3& defaultValue)
{
	IntVector3 vectorToReturn;

	const char* attributeText = element.Attribute(attributeName);

	if (attributeText == nullptr)
	{
		// Attribute not found
		return defaultValue;
	}
	else
	{
		vectorToReturn.SetFromText(attributeText);
		return vectorToReturn;
	}
}


//-----------------------------------------------------------------------------------------------
// Gets an attribute value and returns it as an Vector3, with default value
//
Vector3 ParseXmlAttribute(const XMLElement& element, const char* attributeName, const Vector3& defaultValue)
{
	Vector3 vectorToReturn;

	const char* attributeText = element.Attribute(attributeName);

	if (attributeText == nullptr)
	{
		// Attribute not found
		return defaultValue;
	}
	else
	{
		vectorToReturn.SetFromText(attributeText);
		return vectorToReturn;
	}
}


//-----------------------------------------------------------------------------------------------
// Gets an attribute value and returns it as an unsigned int, with default value
//
unsigned int ParseXmlAttribute(const XMLElement& element, const char* attributeName, unsigned int defaultValue)
{
	int attributeValue = element.UnsignedAttribute(attributeName, defaultValue);
	return attributeValue;
}
