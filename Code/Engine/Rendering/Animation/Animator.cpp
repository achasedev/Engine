/************************************************************************/
/* File: Animator.cpp
/* Author: Andrew Chase
/* Date: July 16th, 2018
/* Description: Implementation of the Animator class
/************************************************************************/
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Rendering/Animation/Animator.hpp"
#include "Engine/Rendering/Animation/AnimationClip.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
Animator::Animator()
{
	m_currStopwatch = new Stopwatch(nullptr);
	m_nextStopwatch = new Stopwatch(nullptr);
	m_transitionStopwatch = new Stopwatch(nullptr);
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Animator::~Animator()
{
	delete m_currStopwatch;
	m_currStopwatch = nullptr;

	delete m_nextStopwatch; 
	m_nextStopwatch = nullptr;

	delete m_transitionStopwatch;
	m_transitionStopwatch = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Plays the given animation from the beginning, ignoring any initial state of the animator
//
void Animator::Play(AnimationClip* clip)
{
	m_currAnimation = clip;
	m_currStopwatch->SetInterval(clip->GetTotalDurationSeconds());

	m_isPaused = false;
	m_isTransitioning = false;
	m_nextAnimation = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Transitions from the currenly playing animation to the given one, over the course of time specified
//
void Animator::TransitionToClip(AnimationClip* clip, float transitionTime)
{
	// If we're currently transitioning then don't do anything
	if (m_isTransitioning)
	{
		return;
	}

	// Clamp the transition to be at most the clip duration
	float clipDuration = clip->GetTotalDurationSeconds();
	if (clipDuration < transitionTime)
	{
		transitionTime = clipDuration;
	}

	// Set up for transition
	m_nextAnimation = clip;
	m_nextStopwatch->SetInterval(clipDuration);
	m_transitionStopwatch->SetInterval(transitionTime);
	m_isTransitioning = true;
}


//-----------------------------------------------------------------------------------------------
// Returns the pose to render given the animator's current state
//
Pose* Animator::GetCurrentPose()
{
	if (m_currAnimation == nullptr)
	{
		return nullptr;
	}

	if (m_isTransitioning)
	{
		// Get the blend between the two

		// Get times
		float currTimeElapsed = m_currStopwatch->GetElapsedTimeNormalized();
		float nextTimeElapsed = m_nextStopwatch->GetElapsedTimeNormalized();

		// Get each respective pose
		Pose* currentPose	= m_currAnimation->CalculatePoseAtNormalizedTime(currTimeElapsed);
		Pose* nextPose		= m_nextAnimation->CalculatePoseAtNormalizedTime(nextTimeElapsed);

		// Interpolate the poses based on time into transition
		float transitionTimeNormalized = m_transitionStopwatch->GetElapsedTimeNormalized();
		int boneCount = currentPose->GetBoneCount();

		for (int boneIndex = 0; boneIndex < boneCount; ++boneIndex)
		{
			currentPose->SetBoneTransform(boneIndex, Interpolate(currentPose->GetBoneTransform(boneIndex), nextPose->GetBoneTransform(boneIndex), transitionTimeNormalized));
		}

		// Check if we're done transitioning
		if (m_transitionStopwatch->HasIntervalElapsed())
		{
			m_currAnimation = m_nextAnimation;
			m_nextAnimation = nullptr;

			// Need to swap stopwatches
			Stopwatch* temp = m_currStopwatch; 
			m_currStopwatch = m_nextStopwatch; 
			m_nextStopwatch = temp;

			m_isTransitioning = false;
		}

		// Return the result, cleaning up
		delete nextPose; 
		return currentPose;
	}
	else
	{
		// Just return the pose at time
		float timeElapsed = m_currStopwatch->GetElapsedTimeNormalized();
		return m_currAnimation->CalculatePoseAtNormalizedTime(timeElapsed);
	}
}
