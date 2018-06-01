/************************************************************************/
/* File: SpriteAnimDef.hpp
/* Author: Andrew Chase
/* Date: November 12th, 2017
/* Bugs: None
/* Description: Class to represent Sprite Animation static data
/************************************************************************/
#pragma once
#include <vector>
#include "Engine/Renderer/SpriteAnim.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "ThirdParty/tinyxml2/tinyxml2.h"

class SpriteAnimDef
{
public:
	//-----Public Methods-----

	SpriteAnimDef(const SpriteSheet& defaultSpriteSheet, int framesPerSecond, const std::vector<int>& frameIndices, const std::string& name=std::string("default"), PlayMode playmode=PLAY_MODE_ONCE);			// Construct from code
	SpriteAnimDef(const SpriteSheet& defaultSpriteSheet, const XMLElement& spriteAnimDefElement);						// Construct from XML data

	// Accessors/Producers
	std::string		GetName() const;
	int				GetFirstSpriteIndex() const;
	int				GetNumFrames() const;
	int				GetFramesPerSecond() const;
	float			GetSequenceDuration() const;
	PlayMode		GetPlayMode() const;
	bool			ShouldAutoOrient() const;
	int				GetPriority() const;


	const Texture& GetTexture() const;
	AABB2 GetCurrentUVCoords(int currentSpriteIndex) const;


private:
	//-----Private Methods-----

	void ParseSpriteIndexData(const std::string& indexString);


private:
	//-----Private Data-----

	std::string m_name = "";					// The name of this animation
	SpriteSheet m_spriteSheet;					// The spritesheet the animation frames come from 
	const int m_framesPerSecond;				// The number of frames displayed in one second
	std::vector<int> m_frameIndexes;			// The spritesheet frame indices of the sequence
	bool m_autoOrient = false;					// Whether this sprite should rotate to correspond with the entity's orientation
	PlayMode m_playMode = PLAY_MODE_LOOP;		// The play mode of this animation type
	int m_priority = 0;							// The priority of this animation, useful for use in SpriteAnimSets
};
