/************************************************************************/
/* File: SpriteAnimation.cpp
/* Author: Andrew Chase
/* Date: November 12th, 2017
/* Bugs: None
/* Description: Implementation of the SpriteAnimation class
/************************************************************************/
#include "Engine/Renderer/SpriteAnim.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Renderer/SpriteAnimDef.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

//-----------------------------------------------------------------------------------------------
// Constructs a SpriteAnimation given definition information, and sets all state information to the defaults
//
SpriteAnim::SpriteAnim(const SpriteAnimDef* animationDef, bool playImmediately)
	: m_spriteAnimDef(animationDef)
	, m_name(animationDef->GetName())
	, m_playMode(animationDef->GetPlayMode())
	, m_secondsElapsed(0.f)
	, m_isFinished(false)
	, m_isPlaying(playImmediately)
{
}


//-----------------------------------------------------------------------------------------------
// Increments seconds elapsed, and sets the isFinished flag to true for PLAY_MODE_ONCE animations
// if the animation has played completely through
//
void SpriteAnim::Update(float deltaTime)
{
	if (m_isPlaying)
	{
		m_secondsElapsed += deltaTime;

		float sequenceDuration = m_spriteAnimDef->GetSequenceDuration();

		if (m_secondsElapsed >= sequenceDuration && m_playMode == PLAY_MODE_ONCE)
		{
			m_isFinished = true;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Starts playing the animation
//
void SpriteAnim::Play()
{
	m_isPlaying = true;
}


//-----------------------------------------------------------------------------------------------
// Freezes the animation, maintaining it's position in the sequence
//
void SpriteAnim::Pause()
{
	m_isPlaying = false;
}


//-----------------------------------------------------------------------------------------------
// Sets the animation back to the beginning, does not begin playing it
//
void SpriteAnim::Reset()
{
	m_isPlaying = false;
	m_isFinished = false;
	m_secondsElapsed = 0.f;
}


//-----------------------------------------------------------------------------------------------
// Sets the animation back to the beginning, and begins playing it
//
void SpriteAnim::ResetAndPlay()
{
	m_isPlaying = true;
	m_isFinished = false;
	m_secondsElapsed = 0.f;
}


//-----------------------------------------------------------------------------------------------
// Returns the name of this animation
//
std::string SpriteAnim::GetName() const
{
	return m_name;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the animation is playing, and false otherwise
//
bool SpriteAnim::IsPlaying() const
{
	return m_isPlaying;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the animation has fully played through once (for PLAY_MODE_ONCE only), and false
// otherwise
//
bool SpriteAnim::IsFinished() const
{
	return m_isFinished;
}


//-----------------------------------------------------------------------------------------------
// Returns the texture associated with this animation
//
const Texture& SpriteAnim::GetTexture() const
{
	return m_spriteAnimDef->GetTexture();
}


//-----------------------------------------------------------------------------------------------
// Returns the UV coordinates for the current sprite frame of this animation
//
AABB2 SpriteAnim::GetCurrentUVCoords() const
{
	int currentSequenceIndex = CalculateCurrentAnimationFrameIndex();
	return m_spriteAnimDef->GetCurrentUVCoords(currentSequenceIndex);
}


//-----------------------------------------------------------------------------------------------
// Returns the amount of time necessary to play through the animation once completely
//
float SpriteAnim::GetDurationSeconds() const
{
	return m_spriteAnimDef->GetSequenceDuration();
}


//-----------------------------------------------------------------------------------------------
// Returns true if this animation should rotate with the entity's rotation
//
bool SpriteAnim::ShouldAutoOrient() const
{
	return m_spriteAnimDef->ShouldAutoOrient();
}


//-----------------------------------------------------------------------------------------------
// Returns the total amount of time played since the animation first started/was last reset
//
float SpriteAnim::GetTotalSecondsElapsed() const
{
	return m_secondsElapsed;
}


//-----------------------------------------------------------------------------------------------
// Returns the priority of this animation as specified by in it's definition 
//
int SpriteAnim::GetPriority() const
{
	return m_spriteAnimDef->GetPriority();
}


//-----------------------------------------------------------------------------------------------
// Returns the amount of time currently into the sequence
//
float SpriteAnim::GetSecondsIntoSequence() const
{
	float secondsIntoSequence = m_secondsElapsed;
	float sequenceDuration = m_spriteAnimDef->GetSequenceDuration();

	while (secondsIntoSequence >= sequenceDuration)
	{
		secondsIntoSequence -= sequenceDuration;
	}

	return secondsIntoSequence;
}


//-----------------------------------------------------------------------------------------------
// Returns the amount of time left necessary to complete the sequence
//
float SpriteAnim::GetSecondsRemainingInSequence() const
{
	float secondsIntoSequence = GetSecondsIntoSequence();
	float sequenceDuration = m_spriteAnimDef->GetSequenceDuration();

	return (sequenceDuration - secondsIntoSequence);
}


//-----------------------------------------------------------------------------------------------
// Returns the fraction into the sequence (total time into the sequence / length of the sequence)
//
float SpriteAnim::GetFractionElapsed() const
{
	float secondsIntoSequence = GetSecondsIntoSequence();
	float sequenceDuration = m_spriteAnimDef->GetSequenceDuration();

	return (secondsIntoSequence / sequenceDuration);
}


//-----------------------------------------------------------------------------------------------
// Returns the fraction remaining in the sequence (1 - fraction into the sequence)
//
float SpriteAnim::GetFractionRemaining() const
{
	float fractionElapsed = GetFractionElapsed();
	return (1.f - fractionElapsed);
}


//-----------------------------------------------------------------------------------------------
// Sets the amount of time passed thus far, for skipping to a certain segment into the sequence
//
void SpriteAnim::SetSecondsElapsed(float secondsElapsed)
{
	m_secondsElapsed = secondsElapsed;
}


//-----------------------------------------------------------------------------------------------
// Sets the time elapsed to be a certain fraction into the sequence, for skipping
//
void SpriteAnim::SetFractionElapsed(float fractionElapsed)
{
	float sequenceDuration = m_spriteAnimDef->GetSequenceDuration();
	float secondsElapsed = (fractionElapsed * sequenceDuration);
	SetSecondsElapsed(secondsElapsed);
}


//-----------------------------------------------------------------------------------------------
// Updates the current index of this sprite animation based on the animation parameters
//
int SpriteAnim::CalculateCurrentAnimationFrameIndex() const
{
	float secondsIntoSequence = GetSecondsIntoSequence();

	// Find the amount of time per frame
	float secondsPerFrame = (1.f / m_spriteAnimDef->GetFramesPerSecond());
	 
	// Determine the frame number by finding which offset into the sequence we're
	// in the middle of
	int sequenceOffsetIndex = 0;
	while (secondsIntoSequence > secondsPerFrame)
	{
		secondsIntoSequence -= secondsPerFrame;
		sequenceOffsetIndex++;
	}

	return sequenceOffsetIndex;
}


//-----------------------------------------------------------------------------------------------
// Converts the string representation of a PlayMode enumeration to the corresponding enumeration
//
PlayMode SpriteAnim::ConvertStringToPlayMode(const std::string& playModeString)
{
	if (playModeString.compare("Once") == 0) { return PLAY_MODE_ONCE; }
	else if (playModeString.compare("Loop") == 0) { return PLAY_MODE_LOOP; }
	else if (playModeString.compare("PingPong") == 0) { return PLAY_MODE_PINGPONG; }
	else { ERROR_AND_DIE(Stringf("Error: SpriteAnim::ConvertStringToPlayMode couldn't convert string \"%s\" to PlayMode enum.", playModeString.c_str()));}	
}
