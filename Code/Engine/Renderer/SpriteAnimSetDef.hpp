/************************************************************************/
/* File: SpriteAnimSetDef.hpp
/* Author: Andrew Chase
/* Date: November 13th, 2017
/* Bugs: None
/* Description: Class to represent a collection of Animation Definitions
/************************************************************************/
#pragma once
#include <map>
#include <string>
#include "ThirdParty/tinyxml2/tinyxml2.h"
#include "Engine/Renderer/SpriteSheet.hpp"

//typedef tinyxml2::XMLElement XMLElement;
class SpriteAnimDef;
class Renderer;

class SpriteAnimSetDef
{
public:

	friend class SpriteAnimSet;	// Friend the set that uses this definition to access the m_animDefinitions map without an accessor

	//-----Public Methods-----

	SpriteAnimSetDef(const XMLElement& animationSetElement);
	
	SpriteAnimDef*	GetAnimationDefinitionByName(const std::string& animationName) const;
	void			AddAnimationDefinition(const std::string& animationName, SpriteAnimDef* animationToAdd);


private:
	//-----Private Data-----

	std::map<std::string, SpriteAnimDef*> m_animDefinitions;
};
