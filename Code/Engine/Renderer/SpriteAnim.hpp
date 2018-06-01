/************************************************************************/
/* File: SpriteAnimation.hpp
/* Author: Andrew Chase
/* Date: November 12th, 2017
/* Bugs: None
/* Description: Class to represent a sprite animation from a divide cell
				sprite sheet
/************************************************************************/
#pragma once
#include <vector>
#include "Engine/Renderer/SpriteSheet.hpp"
#include "ThirdParty/tinyxml2/tinyxml2.h"

class SpriteAnimDef;

enum PlayMode
{
	PLAY_MODE_ERROR = -1,
	PLAY_MODE_ONCE, 
	PLAY_MODE_LOOP,
	PLAY_MODE_PINGPONG,
	NUM_PLAY_MODES
};

class SpriteAnim
{
public:
	//-----Public Methods-----

	// Constructor, given the definition data
	SpriteAnim(const SpriteAnimDef* animationDef, bool playImmediately);

	void			Update(float deltaTime);	// For incrementing time playing

	void			Play();			// Starts playing the animation
	void			Pause();		// Pauses the animation, but maintains it's place last played
	void			Reset();		// Sets the animation back to the beginning, and pauses it
	void			ResetAndPlay(); // Sets the animation back to the beginning, and resumes playing it

	//-----Accessors-----
	std::string		GetName() const;
	bool			IsPlaying() const;
	bool			IsFinished() const;
	float			GetTotalSecondsElapsed() const;
	int				GetPriority() const;

	//-----Producers-----
	const			Texture& GetTexture() const;	// From the SpriteAnimDef
	AABB2			GetCurrentUVCoords() const;		// From the SpriteAnimDef
	float			GetDurationSeconds() const;
	bool			ShouldAutoOrient() const;

	float			GetSecondsIntoSequence() const;
	float			GetSecondsRemainingInSequence() const;
	float			GetFractionElapsed() const;
	float			GetFractionRemaining() const;

	//-----Mutators-----
	void		SetSecondsElapsed(float secondsElapsed);
	void		SetFractionElapsed(float fractionElapsed);


	//-----Static Functions-----
	static PlayMode ConvertStringToPlayMode(const std::string& playModeString);

private:
	//-----Private Methods-----
	int	CalculateCurrentAnimationFrameIndex() const;


private:
	//-----Private Data-----

	// Data definition members
	std::string m_name;						// The name of this animation
	const SpriteAnimDef* m_spriteAnimDef;	// The data definition of this 
	PlayMode m_playMode;					// Way the animation is played (forward to back, or looping)

	// State tracking members				
	float m_secondsElapsed;					// Amount of time passed since the animation first started playing
	bool m_isPlaying;						// True if the animation is currently playing (seconds elapsed increasing)
	bool m_isFinished;						// True if the animation is done playing (for PLAY_MODE_ONCE only)
};