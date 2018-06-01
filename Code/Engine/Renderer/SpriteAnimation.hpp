/************************************************************************/
/* Project: Incursion
/* File: SpriteAnimation.hpp
/* Author: Andrew Chase
/* Date: October 13th, 2017
/* Bugs: None
/* Description: Class to represent a sprite animation from a divide cell
				sprite sheet
/************************************************************************/
#pragma once
#include "Engine/Renderer/SpriteSheet.hpp"

enum PlayMode
{
	PLAY_MODE_ERROR = -1,
	PLAY_MODE_ONCE, 
	PLAY_MODE_LOOP,
	NUM_PLAY_MODES
};

class SpriteAnimation
{
public:
	//-----Public Methods-----

	// Constructor, given the definition data
	SpriteAnimation(const SpriteSheet& spriteSheet, int spriteStartIndex, int spriteEndIndex, float sequenceDurationSeconds, PlayMode playMode);


	void Update(float deltaTime);	// For incrementing time playing

	void Play();	// Starts playing the animation
	void Pause();	// Pauses the animation, but maintains it's place last played
	void Reset();	// Starts the animation over at the beginning

	//-----Accessors-----
	bool IsPlaying() const;
	bool IsFinished() const;
	const Texture& GetTexture() const;
	AABB2 GetCurrentTexCoords() const;
	float GetDurationSeconds() const;
	float GetTotalSecondsElapsed() const;
	float GetSecondsIntoSequence() const;
	float GetSecondsRemainingInSequence() const;
	float GetFractionElapsed() const;
	float GetFractionRemaining() const;

	//-----Mutators-----
	void SetSecondsElapsed(float secondsElapsed);
	void SetFractionElapsed(float fractionElapsed);


public:

private:

private:
	//-----Private Data-----

	// Data definition members
	const SpriteSheet& m_spriteSheet;		// SpriteSheet used for this animation
	PlayMode m_playMode;					// Way the animation is played (forward to back, or looping)
	int m_spriteStartIndex;					// The linear index of the first sprite in the sequence
	int m_spriteEndIndex;					// The linear index of the last sprite in the sequence (inclusive)
	float m_sequenceDurationSeconds;		// The amount of time necessary to play the animation once through (start index to finish index)
											
	// State tracking members				
	float m_secondsElapsed;					// Amount of time passed since the animation first started playing
	bool m_isPlaying;						// True if the animation is currently playing (seconds elapsed increasing)
	bool m_isFinished;						// True if the animation is done playing (for PLAY_MODE_ONCE only)
};