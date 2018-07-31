/************************************************************************/
/* File: Animator.hpp
/* Author: Andrew Chase
/* Date: July 16th, 2018
/* Description: Class to represent a skeletal animator
/************************************************************************/
#pragma once

class Pose;
class Stopwatch;
class AnimationClip;

class Animator
{
public:
	//-----Public Methods-----

	// Initialization
	Animator();
	~Animator();

	// Playback controls
	void	Play(AnimationClip* clip);
	void	TransitionToClip(AnimationClip* clip, float transitionTime);

	// Accessors
	Pose*	GetCurrentPose();


private:
	//-----Private Data-----

	AnimationClip*	m_currAnimation = nullptr;
	AnimationClip*	m_nextAnimation = nullptr;

	Stopwatch*		m_currStopwatch			= nullptr;
	Stopwatch*		m_nextStopwatch			= nullptr;
	Stopwatch*		m_transitionStopwatch	= nullptr;

	bool			m_isPaused				= false;
	bool			m_isTransitioning		= false;

};
