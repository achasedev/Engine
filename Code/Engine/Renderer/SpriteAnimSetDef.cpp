/************************************************************************/
/* File: SpriteAnimSetDef.cpp
/* Author: Andrew Chase
/* Date: November 13th, 2017
/* Bugs: None
/* Description: Implementation of the SpriteAnimSetDef class
/************************************************************************/
#include "Engine/Renderer/SpriteAnimSetDef.hpp"
#include "Engine/Renderer/SpriteAnimDef.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtilities.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor - using an XMLElement representing an Animation Set Definition
//
SpriteAnimSetDef::SpriteAnimSetDef(const XMLElement& animationSetElement)
{
	Renderer* renderer = Renderer::GetInstance();

	// Create the spritesheet for the animation child elements
	std::string		spriteSheetName		= ParseXmlAttribute(animationSetElement, "spriteSheet", nullptr);
	std::string		spriteSheetFilePath = Stringf("Data/Images/%s", spriteSheetName.c_str());
	IntVector2		spriteLayout		= ParseXmlAttribute(animationSetElement, "spriteLayout", spriteLayout);
	Texture*		spriteSheetTexture	= renderer->CreateOrGetTexture(spriteSheetFilePath);
	SpriteSheet		setSpriteSheet		= SpriteSheet(spriteSheetName, *spriteSheetTexture, spriteLayout);


	// Iterate across animation elements to create animation definitions
	const XMLElement* animationElement = animationSetElement.FirstChildElement();

	while (animationElement != nullptr)
	{
		// Make the animation definition
		SpriteAnimDef* animationDefinition = new SpriteAnimDef(setSpriteSheet, *animationElement);

		// Add it to the list of animation definitions by name
		std::string animationName = animationDefinition->GetName();
		AddAnimationDefinition(animationName, animationDefinition);

		// Iterate to the next animation element
		animationElement = animationElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the animation defintion given by the name animationName
//
SpriteAnimDef* SpriteAnimSetDef::GetAnimationDefinitionByName(const std::string& animationName) const
{
	// Ensure the animation exists
	bool animationExists = (m_animDefinitions.find(animationName) != m_animDefinitions.end());
	GUARANTEE_OR_DIE(animationExists, Stringf("Error: SpriteAnimSetDef::GetAnimationByName couldn't find animation \"%s\" in its map of animations", animationName.c_str()))

	return m_animDefinitions.at(animationName);
}


//-----------------------------------------------------------------------------------------------
// Adds the animation definition to this set, avoid duplicates
//
void SpriteAnimSetDef::AddAnimationDefinition(const std::string& animationName, SpriteAnimDef* animationToAdd)
{
	// Ensure the animation doesn't exist
	bool animationExists = (m_animDefinitions.find(animationName) != m_animDefinitions.end());
	GUARANTEE_OR_DIE(!animationExists, Stringf("Error: SpriteAnimSetDef::AddAnimationDefinition tried to add duplicate animation \"%s\"", animationName.c_str()))

	m_animDefinitions[animationName] = animationToAdd;
}
