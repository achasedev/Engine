/************************************************************************/
/* File: Blackboard.cpp
/* Author: Andrew Chase
/* Date: October 28th, 2017
/* Bugs: None
/* Description: Implementation of the Blackboard class
/************************************************************************/
#include "Engine/Core/Utility/Blackboard.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/FloatRange.hpp"


//-----------------------------------------------------------------------------------------------
// Adds the attributes in 'element' to the key-value map
//
void Blackboard::PopulateFromXmlElementAttributes(const tinyxml2::XMLElement& element)
{
	const tinyxml2::XMLAttribute* currAttribute = element.FirstAttribute();
	
	while (currAttribute != nullptr)
	{
		SetValue(currAttribute->Name(), currAttribute->Value());
		currAttribute = currAttribute->Next();
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the element indicated by keyName in the map to have the value newValue
// Creates an element for keyName if keyName doesn't already exist in the map
//
void Blackboard::SetValue(const std::string& keyName, const std::string& newValue)
{
	m_keyValuePairs[keyName] = newValue;
}


//-----------------------------------------------------------------------------------------------
// Gets the value indicated by keyName as a bool
//
bool Blackboard::GetValue(const std::string& keyName, bool defaultValue) const
{
	if (m_keyValuePairs.find(keyName) != m_keyValuePairs.end())
	{
		std::string boolText = m_keyValuePairs.at(keyName);

		// Convert string to bool logic
		if (boolText.compare("true") == 0)
		{
			return true;
		}
		else if (boolText.compare("false") == 0)
		{
			return false;
		}
		else
		{
			ERROR_RECOVERABLE(Stringf("Blackboard has a key %s with bool value %s", keyName.c_str(), boolText.c_str()));
		}
	}

	return defaultValue;
}


//-----------------------------------------------------------------------------------------------
// Gets the value indicated by keyName as an int
//
int Blackboard::GetValue(const std::string& keyName, int defaultValue) const
{
	if (m_keyValuePairs.find(keyName) != m_keyValuePairs.end())
	{
		std::string intText = m_keyValuePairs.at(keyName);
		int intValue = atoi(intText.c_str());
		return intValue;
	}
	else
	{
		return defaultValue;
	}
}


//-----------------------------------------------------------------------------------------------
// Gets the value indicated by keyName as a float
//
float Blackboard::GetValue(const std::string& keyName, float defaultValue) const
{
	if (m_keyValuePairs.find(keyName) != m_keyValuePairs.end())
	{
		std::string floatText = m_keyValuePairs.at(keyName);
		float floatValue = static_cast<float>(atof(floatText.c_str()));
		return floatValue;
	}
	else
	{
		return defaultValue;
	}
}


//-----------------------------------------------------------------------------------------------
// Gets the value indicated by keyName as a string
//
std::string Blackboard::GetValue(const std::string& keyName, std::string defaultValue) const
{
	if (m_keyValuePairs.find(keyName) != m_keyValuePairs.end())
	{
		return m_keyValuePairs.at(keyName);
	}
	else
	{
		return defaultValue;
	}
}


//-----------------------------------------------------------------------------------------------
// Gets the value indicated by keyName as a string, with a char* default value
//
std::string Blackboard::GetValue(const std::string& keyName, const char* defaultValue) const
{
	if (m_keyValuePairs.find(keyName) != m_keyValuePairs.end())
	{
		return m_keyValuePairs.at(keyName);
	}
	else
	{
		return std::string(defaultValue);
	}
}


//-----------------------------------------------------------------------------------------------
// Gets the value indicated by keyName as an Rgba
//
Rgba Blackboard::GetValue(const std::string& keyName, const Rgba& defaultValue) const
{
	if (m_keyValuePairs.find(keyName) != m_keyValuePairs.end())
	{
		std::string rgbaText = m_keyValuePairs.at(keyName);
		Rgba colorValue;
		colorValue.SetFromText(rgbaText.c_str());
		return colorValue;
	}
	else
	{
		return defaultValue;
	}
}


//-----------------------------------------------------------------------------------------------
// Gets the value indicated by keyName as a Vector2
//
Vector2 Blackboard::GetValue(const std::string& keyName, const Vector2& defaultValue) const
{
	if (m_keyValuePairs.find(keyName) != m_keyValuePairs.end())
	{
		std::string vectorText = m_keyValuePairs.at(keyName);
		Vector2 vectorValue;
		vectorValue.SetFromText(vectorText.c_str());
		return vectorValue;
	}
	else
	{
		return defaultValue;
	}
}


//-----------------------------------------------------------------------------------------------
// Gets the value indicated by keyName as an IntVector2
//
IntVector2 Blackboard::GetValue(const std::string& keyName, const IntVector2& defaultValue) const
{
	if (m_keyValuePairs.find(keyName) != m_keyValuePairs.end())
	{
		std::string vectorText = m_keyValuePairs.at(keyName);
		IntVector2 vectorValue;
		vectorValue.SetFromText(vectorText.c_str());
		return vectorValue;
	}
	else
	{
		return defaultValue;
	}
}


//-----------------------------------------------------------------------------------------------
// Gets the value indicated by keyName as a FloatRange
//
FloatRange Blackboard::GetValue(const std::string& keyName, const FloatRange& defaultValue) const
{
	if (m_keyValuePairs.find(keyName) != m_keyValuePairs.end())
	{
		std::string rangeText = m_keyValuePairs.at(keyName);
		FloatRange rangeValue;
		rangeValue.SetFromText(rangeText.c_str());
		return rangeValue;
	}
	else
	{
		return defaultValue;
	}
}


//-----------------------------------------------------------------------------------------------
// Gets the value indicated by keyName as an IntRange
//
IntRange Blackboard::GetValue(const std::string& keyName, const IntRange& defaultValue) const
{
	if (m_keyValuePairs.find(keyName) != m_keyValuePairs.end())
	{
		std::string rangeText = m_keyValuePairs.at(keyName);
		IntRange rangeValue;
		rangeValue.SetFromText(rangeText.c_str());
		return rangeValue;
	}
	else
	{
		return defaultValue;
	}
}
