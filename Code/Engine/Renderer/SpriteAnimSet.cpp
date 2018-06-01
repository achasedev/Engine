/************************************************************************/
/* File: SpriteAnimSet.cpp
/* Author: Andrew Chase
/* Date: November 13th, 2017
/* Bugs: None
/* Description: Implementation of the SpriteAnimSet class
/************************************************************************/
#include "Engine/Renderer/SpriteAnim.hpp"
#include "Engine/Renderer/SpriteAnimSet.hpp"
#include "Engine/Renderer/SpriteAnimSetDef.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor - from a set definition
//
SpriteAnimSet::SpriteAnimSet(const SpriteAnimSetDef* setDefinition)
	: m_setDefinition(setDefinition)
{
	// Create an animation from each animation definition
	std::map<std::string, SpriteAnimDef*>::const_iterator itr = setDefinition->m_animDefinitions.begin();

	for (itr; itr != setDefinition->m_animDefinitions.end(); itr++)
	{
		SpriteAnim* newAnimation = new SpriteAnim(itr->second, true);
		m_animations[newAnimation->GetName()] = newAnimation;
	}
}


//-----------------------------------------------------------------------------------------------
// Destructor - Deletes the map of animation pointers
//
SpriteAnimSet::~SpriteAnimSet()
{
	// Clean up SpriteAnim pointers
	for (std::map<std::string, SpriteAnim*>::iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		delete it->second;
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the current animation, if it is playing
//
void SpriteAnimSet::Update(float deltaTime)
{
	m_currentAnimation->Update(deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Returns the animation that is currently playing
//
SpriteAnim* SpriteAnimSet::GetCurrentAnimation() const
{
	return m_currentAnimation;
}


//-----------------------------------------------------------------------------------------------
// Returns the animation given by the name animationName
//
SpriteAnim* SpriteAnimSet::GetAnimationByName(const std::string& animationName) const
{
	// Ensure the animation exists
	bool animationExists = (m_animations.find(animationName) != m_animations.end());
	GUARANTEE_OR_DIE(animationExists, Stringf("Error: SpriteAnimSet::GetAnimation() - Animation \"%s\" does not exist.", animationName.c_str()));

	return m_animations.at(animationName);
}


//-----------------------------------------------------------------------------------------------
// Sets the current animation to the one given by animationName, and starts playing it
//
void SpriteAnimSet::StartAnimation(const std::string& animationName)
{
	SetCurrentAnimation(animationName);
	m_currentAnimation->Play();
}


//-----------------------------------------------------------------------------------------------
// Sets the current animation to the one given by animationName
// Does not begin playing the animation
//
void SpriteAnimSet::SetCurrentAnimation(const std::string& animationName)
{
	bool animationExists = (m_animations.find(animationName) != m_animations.end());

	// If the animation doesn't exist, panic
	GUARANTEE_OR_DIE(animationExists, Stringf("Error: SpriteAnimSet::SetCurrentAnimation couldn't find animation named \"%s\"", animationName.c_str()));

	// Check the priority to see if we should update it
	int currPriority = (m_currentAnimation == nullptr ? -1 : m_currentAnimation->GetPriority());
	int newPriority = m_animations.at(animationName)->GetPriority();

	if (newPriority >= currPriority || m_currentAnimation->IsFinished())
	{
		m_currentAnimation = m_animations.at(animationName);

		// If the animation was finished reset it
		if (m_currentAnimation->IsFinished())
		{
			m_currentAnimation->Reset();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Adds the animation given by animationName to the map of animations, avoiding duplicates
//
void SpriteAnimSet::AddAnimation(const std::string& animationName, SpriteAnim* animationToAdd)
{
	// Ensure the animation doesn't exist
	bool animationExists = (m_animations.find(animationName) != m_animations.end());
	GUARANTEE_OR_DIE(!animationExists, Stringf("Error: SpriteAnimSet::AddAnimation() - Animation \"%s\" couldn't be added, it already exists in the set.", animationName.c_str()));

	m_animations[animationName] = animationToAdd;
}


//-----------------------------------------------------------------------------------------------
// Sets the current animation's time elapsed to the one specified
//
void SpriteAnimSet::SetCurrentTimeElapsed(float timeToSet)
{
	m_currentAnimation->SetSecondsElapsed(timeToSet);
}


//-----------------------------------------------------------------------------------------------
// Pauses the animation currently playing
//
void SpriteAnimSet::PauseCurrentAnimation()
{
	m_currentAnimation->Pause();
}


//-----------------------------------------------------------------------------------------------
// Plays the animation from where it was last playing at
//
void SpriteAnimSet::PlayCurrentAnimation()
{
	m_currentAnimation->Play();
}


//-----------------------------------------------------------------------------------------------
// Returns the texture used by the current animation
//
const Texture& SpriteAnimSet::GetTexture() const
{
	return m_currentAnimation->GetTexture();
}


//-----------------------------------------------------------------------------------------------
// Returns the UVs of the current animation's current frame
//
AABB2 SpriteAnimSet::GetCurrentUVs() const
{
	return m_currentAnimation->GetCurrentUVCoords();
}
