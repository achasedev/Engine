/************************************************************************/
/* File: SpriteAimDef.cpp
/* Author: Andrew Chase
/* Date: November 12th, 2017
/* Bugs: None
/* Description: Implementation of the SpriteAnimDef class
/************************************************************************/
#include "Engine/Renderer/SpriteAnimDef.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Core/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
// Constructs a SpriteAnimDef from explicit values
//
SpriteAnimDef::SpriteAnimDef(const SpriteSheet& spriteSheet, int framesPerSecond, const std::vector<int>& frameIndices, const std::string& name/*=std::string("default")*/)
	: m_name(name)
	, m_spriteSheet(spriteSheet)
	, m_framesPerSecond(framesPerSecond)
	, m_frameIndexes(frameIndices)
	, m_autoOrient(false)
	, m_priority(0)
{
}


//-----------------------------------------------------------------------------------------------
// Constructs a SpriteAnimDef by parsing XML data
//
SpriteAnimDef::SpriteAnimDef(const SpriteSheet& defaultSpriteSheet, const XMLElement& spriteAnimDefElement)
	: m_name(ParseXmlAttribute(spriteAnimDefElement, "name", m_name))
	, m_spriteSheet(defaultSpriteSheet)
	, m_framesPerSecond(ParseXmlAttribute(spriteAnimDefElement, "framesPerSecond", m_framesPerSecond))
	, m_autoOrient(ParseXmlAttribute(spriteAnimDefElement, "autoOrient", m_autoOrient))
{
	// Parse the string data for the sprite sequence indices
	std::string spriteIndexString = ParseXmlAttribute(spriteAnimDefElement, "spriteIndexes", spriteIndexString);
	ParseSpriteIndexData(spriteIndexString);

	// Parse the PlayMode
	std::string playModeString = ParseXmlAttribute(spriteAnimDefElement, "playMode", playModeString);
	if (playModeString.compare("") != 0)
	{
		m_playMode = SpriteAnim::ConvertStringToPlayMode(playModeString);
	}

	// Parse the priority
	m_priority = ParseXmlAttribute(spriteAnimDefElement, "priority", m_priority);
}


//-----------------------------------------------------------------------------------------------
// Returns the name of this animation
//
std::string SpriteAnimDef::GetName() const
{
	return m_name;
}


//-----------------------------------------------------------------------------------------------
// Returns the index of the first sprite of this sequence
//
int SpriteAnimDef::GetFirstSpriteIndex() const
{
	return m_frameIndexes[0];
}


//-----------------------------------------------------------------------------------------------
// Returns the number of sprite frames in this sequence
//
int SpriteAnimDef::GetNumFrames() const
{
	return static_cast<int>(m_frameIndexes.size());
}


//-----------------------------------------------------------------------------------------------
// Returns the number of animation frames played each second
//
int SpriteAnimDef::GetFramesPerSecond() const
{
	return m_framesPerSecond; 
}


//-----------------------------------------------------------------------------------------------
// Returns the duration of one play of this sequence, in seconds
//
float SpriteAnimDef::GetSequenceDuration() const
{
	int numFramesInSequence = GetNumFrames();

	return static_cast<float>(numFramesInSequence) / m_framesPerSecond;
}


//-----------------------------------------------------------------------------------------------
// Returns the PlayMode enumeration of this definition
//
PlayMode SpriteAnimDef::GetPlayMode() const
{
	return m_playMode;
}


//-----------------------------------------------------------------------------------------------
// Returns true if this animation should rotate with the entity's rotation
//
bool SpriteAnimDef::ShouldAutoOrient() const
{
	return m_autoOrient;
}


//-----------------------------------------------------------------------------------------------
// Returns the priority of this animation definition
//
int SpriteAnimDef::GetPriority() const
{
	return m_priority;
}


//-----------------------------------------------------------------------------------------------
// Returns the texture of the spritesheet used by this definition
// Used for rendering the animation without including g_theRenderer in engine code
//
const Texture& SpriteAnimDef::GetTexture() const
{
	return m_spriteSheet.GetTexture();
}


//-----------------------------------------------------------------------------------------------
// Returns the UV coordinates of the current frame of the animation sequence
//
AABB2 SpriteAnimDef::GetCurrentUVCoords(int sequenceIndex) const
{
	return m_spriteSheet.GetTexUVsFromSpriteIndex(m_frameIndexes[sequenceIndex]);
}


//-----------------------------------------------------------------------------------------------
// Parses a string representation of the animation frame sequence and constructs the 
// vector m_spriteIndices out of it
//
void SpriteAnimDef::ParseSpriteIndexData(const std::string& indexString)
{
	// TODO: Parse the list of indices here
	std::vector<std::string> indexTokens = Tokenize(indexString, ',');

	// Convert them to ints
	for (int i = 0; i < static_cast<int>(indexTokens.size()); i++)
	{
		m_frameIndexes.push_back(atoi(indexTokens[i].c_str()));
	}
}
