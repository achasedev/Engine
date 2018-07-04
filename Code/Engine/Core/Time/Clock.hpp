/************************************************************************/
/* File: Clock.hpp
/* Author: Andrew Chase
/* Date: March 5th, 2018
/* Description: Class to represent time information across frames
/************************************************************************/
#pragma once
#include <stdint.h>
#include <vector>


// Struct for a set of time data, in hpc and seconds
struct TimeData_t
{
	TimeData_t()
		: m_hpc(0)
		, m_seconds(0)
	{}

	uint64_t	m_hpc;
	double		m_seconds;

};


class Clock
{
public:
	//-----Public Methods-----

	// Constructors
	Clock();
	Clock(Clock* parent);

	static void Initialize();
	void		BeginFrame();
	void		FrameStep(uint64_t elapsedHPC);

	// Mutators
	void ResetTimeData();
	void AddChild(Clock* child);

	void SetScale(float newScale);
	void SetPaused(bool pauseState);


	// Accessors
	float		GetDeltaTime() const;
	uint64_t	GetFrameHPC() const;

	float		GetTotalSeconds() const;
	uint64_t	GetTotalHPC() const;

	static Clock*	GetMasterClock();
	static uint64_t GetMasterTotalTime();
	static float	GetMasterDeltaTime();
	static float	GetMasterFPS();


private:
	//-----Private Methods

	Clock(const Clock& copy) = delete;	// No copying allowed


private:
	//-----Private Data-----

	Clock* m_parent;	// This clock's hierarchical parent
	std::vector<Clock*> m_childClocks;

	unsigned int	m_frameCount;
	double			m_scale;
	bool			m_isPaused;

	TimeData_t		m_frameData;
	TimeData_t		m_totalData;

	uint64_t		m_lastFrameHPC;

	// Not a pointer - is constructed before main
	static Clock s_masterClock;

};
