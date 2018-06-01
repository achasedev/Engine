/************************************************************************/
/* File: Blackboard.hpp
/* Author: Andrew Chase
/* Date: October 28th, 2017
/* Bugs: None
/* Description: Class to represent a set of game/engine configuration
				information
/************************************************************************/
#pragma once
#include <map>
#include "ThirdParty/tinyxml2/tinyxml2.h"

class Rgba;
class Vector2;
class IntVector2;
class FloatRange;
class IntRange;

class Blackboard
{
public:
	//-----Public Methods-----

	void			PopulateFromXmlElementAttributes( const tinyxml2::XMLElement& element );
	void			SetValue( const std::string& keyName, const std::string& newValue );

	bool			GetValue( const std::string& keyName, bool defaultValue ) const;
	int				GetValue( const std::string& keyName, int defaultValue ) const;
	float			GetValue( const std::string& keyName, float defaultValue ) const;
	std::string		GetValue( const std::string& keyName, std::string defaultValue ) const;
	std::string		GetValue( const std::string& keyName, const char* defaultValue ) const;
	Rgba			GetValue( const std::string& keyName, const Rgba& defaultValue ) const;
	Vector2			GetValue( const std::string& keyName, const Vector2& defaultValue ) const;
	IntVector2		GetValue( const std::string& keyName, const IntVector2& defaultValue ) const;
	FloatRange		GetValue( const std::string& keyName, const FloatRange& defaultValue ) const;
	IntRange		GetValue( const std::string& keyName, const IntRange& defaultValue ) const;


private:
	//-----Private Data-----

	std::map<std::string, std::string> m_keyValuePairs;

};