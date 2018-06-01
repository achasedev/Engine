/************************************************************************/
/* Project: Incursion
/* File: SpriteAnimation.cpp
/* Author: Andrew Chase
/* Date: October 13th, 2017
/* Bugs: None
/* Description: Implementation of the SpriteAnimation class
/************************************************************************/
#include "Engine/Renderer/SpriteAnimation.hpp"


//-----------------------------------------------------------------------------------------------
// Constructs a SpriteAnimation given definition information, and sets all state information to the defaults
//
SpriteAnimation::SpriteAnimation(const SpriteSheet& spriteSheet, int spriteStartIndex, int spriteEndIndex, float sequenceDurationSeconds, PlayMode playMode)
	: m_spriteSheet(spriteSheet)
	, m_spriteStartIndex(spriteStartIndex)
	, m_spriteEndIndex(spriteEndIndex)
	, m_sequenceDurationSeconds(sequenceDurationSeconds)
	, m_playMode(playMode)
	, m_secondsElapsed(0.f)
	, m_isFinished(false)
	, m_isPlaying(true)
{
}


//-----------------------------------------------------------------------------------------------
// Increments seconds elapsed, and sets the isFinished flag to true for PLAY_MODE_ONCE animations
// if the animation has played completely through
//
void SpriteAnimation::Update(float deltaTime)
{
	if (m_isPlaying)
	{
		m_secondsElapsed += deltaTime;

		if (m_secondsElapsed >= m_sequenceDurationSeconds && m_playMode == PLAY_MODE_ONCE)
		{
			m_isFinished = true;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Starts playing the animation
//
void SpriteAnimation::Play()
{
	m_isPlaying = true;
}


//-----------------------------------------------------------------------------------------------
// Freezes the animation, maintaining it's position in the sequence
//
void SpriteAnimation::Pause()
{
	m_isPlaying = false;
}


//-----------------------------------------------------------------------------------------------
// Sets the animation back to the beginning, and automatically begins playing again
//
void SpriteAnimation::Reset()
{
	m_isPlaying = true;
	m_isFinished = false;
	m_secondsElapsed = 0.f;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the animation is playing, and false otherwise
//
bool SpriteAnimation::IsPlaying() const
{
	return m_isPlaying;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the animation has fully played through once (for PLAY_MODE_ONCE only), and false
// otherwise
//
bool SpriteAnimation::IsFinished() const
{
	return m_isFinished;
}


//-----------------------------------------------------------------------------------------------
// Returns the texture associated with this SpriteAnimation (the entire SpriteSheet)
//
const Texture& SpriteAnimation::GetTexture() const
{
	return m_spriteSheet.GetTexture();
}


//-----------------------------------------------------------------------------------------------
// REturns the current texture quad that this animation is on, given the amount of time passed
//
AABB2 SpriteAnimation::GetCurrentTexCoords() const
{
	// If we play once and the sequence is done, continue to return the last sprite
	if (m_playMode == PLAY_MODE_ONCE && m_isFinished)
	{
		return m_spriteSheet.GetTexCoordFromSpriteIndex(m_spriteEndIndex);
	}

	float secondsIntoSequence = GetSecondsIntoSequence();

	// Find the number of frames and time for each frame
	int numFrames = (m_spriteEndIndex - m_spriteStartIndex + 1);
	float secondsPerFrame = (m_sequenceDurationSeconds / numFrames);

	// Determine the frame number by finding which offset into the sequence we're
	// in the middle of
	int sequenceOffsetIndex = 0;
	while (secondsIntoSequence > secondsPerFrame)
	{
		secondsIntoSequence -= secondsPerFrame;
		sequenceOffsetIndex++;
	}

	// Use the offset plus the start index to find the current sprite
	return m_spriteSheet.GetTexCoordFromSpriteIndex(m_spriteStartIndex + sequenceOffsetIndex);
}


//-----------------------------------------------------------------------------------------------
// Returns the amount of time necessary to play through the animation once completely
//
float SpriteAnimation::GetDurationSeconds() const
{
	return m_sequenceDurationSeconds;
}


//-----------------------------------------------------------------------------------------------
// Returns the total amount of time played since the animation first started/was last reset
//
float SpriteAnimation::GetTotalSecondsElapsed() const
{
	return m_secondsElapsed;
}


//-----------------------------------------------------------------------------------------------
// Returns the amount of time currently into the sequence
//
float SpriteAnimation::GetSecondsIntoSequence() const
{
	float secondsIntoSequence = m_secondsElapsed;
	while (secondsIntoSequence >= m_sequenceDurationSeconds)
	{
		secondsIntoSequence -= m_sequenceDurationSeconds;
	}

	return secondsIntoSequence;
}


//-----------------------------------------------------------------------------------------------
// Returns the amount of time left necessary to complete the sequence
//
float SpriteAnimation::GetSecondsRemainingInSequence() const
{
	float secondsIntoSequence = GetSecondsIntoSequence();
	return (m_sequenceDurationSeconds - secondsIntoSequence);
}


//-----------------------------------------------------------------------------------------------
// Returns the fraction into the sequence (total time into the sequence / length of the sequence)
//
float SpriteAnimation::GetFractionElapsed() const
{
	float secondsIntoSequence = GetSecondsIntoSequence();
	return (secondsIntoSequence / m_sequenceDurationSeconds);
}


//-----------------------------------------------------------------------------------------------
// Returns the fraction remaining in the sequence (1 - fraction into the sequence)
//
float SpriteAnimation::GetFractionRemaining() const
{
	float fractionElapsed = GetFractionElapsed();
	return (1.f - fractionElapsed);
}


//-----------------------------------------------------------------------------------------------
// Sets the amount of time passed thus far, for skipping to a certain segment into the sequence
//
void SpriteAnimation::SetSecondsElapsed(float secondsElapsed)
{
	m_secondsElapsed = secondsElapsed;
}


//-----------------------------------------------------------------------------------------------
// Sets the time elapsed to be a certain fraction into the sequence, for skipping
//
void SpriteAnimation::SetFractionElapsed(float fractionElapsed)
{
	float secondsElapsed = (fractionElapsed * m_sequenceDurationSeconds);
	SetSecondsElapsed(secondsElapsed);
}
