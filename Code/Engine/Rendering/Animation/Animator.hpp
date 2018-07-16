#pragma once

class Pose;
class Stopwatch;
class AnimationClip;

class Animator
{
public:
	//-----Public Methods-----

	Animator();
	~Animator();

	void	Play(AnimationClip* clip);

	void	TransitionToClip(AnimationClip* clip, float transitionTime);

	Pose*	GetCurrentPose();


private:
	//-----Private Data-----

	AnimationClip*	m_currAnimation = nullptr;
	AnimationClip*	m_nextAnimation = nullptr;

	Stopwatch*		m_currStopwatch	= nullptr;
	Stopwatch*		m_nextStopwatch	= nullptr;
	Stopwatch*		m_transitionStopwatch = nullptr;

	bool			m_isPaused				= false;
	bool			m_isTransitioning		= false;

};
