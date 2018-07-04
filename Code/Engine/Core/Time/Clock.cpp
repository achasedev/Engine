/************************************************************************/
/* File: Clock.cpp
/* Author: Andrew Chase
/* Date: March 5th, 2018
/* Description: Implementation of the Clock class
/************************************************************************/
#include <cstring>
#include "Engine/Core/Time/Time.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"


// Master clock of the system, constructed before main()
Clock Clock::s_masterClock;


//-----------------------------------------------------------------------------------------------
// Default constructor - no parent
//
Clock::Clock()
	: m_parent(nullptr)
	, m_scale(1.0f)
	, m_isPaused(false)
{
	ResetTimeData();
}


//-----------------------------------------------------------------------------------------------
// Constructor for a specified parent
//
Clock::Clock(Clock* parent)
	: m_parent(parent)
	, m_scale(1.0f)
	, m_isPaused(false)
{
	ResetTimeData();

	if (m_parent != nullptr)
	{
		m_parent->AddChild(this);
	}
}


//-----------------------------------------------------------------------------------------------
// Static function = resets the time data on the master clock
//
void Clock::Initialize()
{
	s_masterClock.ResetTimeData();
}


//-----------------------------------------------------------------------------------------------
// Calculates the actual delta time of the frame and updates all clocks in the hierarchy
// Should only be called on the master clock for the master hierarchy
//
void Clock::BeginFrame()
{
	// For now, ensure we only call BeginFrame on the master clock
	if (this != &s_masterClock)
	{
		return;
	}

	uint64_t currentHPC = GetPerformanceCounter();
	uint64_t elapsed	= currentHPC - m_lastFrameHPC;
	m_lastFrameHPC		= currentHPC;

	// Update based on elapsed
	FrameStep(elapsed);
}


//-----------------------------------------------------------------------------------------------
// Returns a pointer to the master clock instance
//
Clock* Clock::GetMasterClock()
{
	return &s_masterClock;
}


//-----------------------------------------------------------------------------------------------
// Returns the total time of the engine (total time of the master clock)
//
uint64_t Clock::GetMasterTotalTime()
{
	return s_masterClock.m_totalData.m_hpc;
}


//-----------------------------------------------------------------------------------------------
// Returns the frame time of the master clock
//
float Clock::GetMasterDeltaTime()
{
	return (float) s_masterClock.m_frameData.m_seconds;
}


//-----------------------------------------------------------------------------------------------
// Returns the fps of the master clock
//
float Clock::GetMasterFPS()
{
	return (1.f / (float) s_masterClock.m_frameData.m_seconds);
}


//-----------------------------------------------------------------------------------------------
// Updates the time data of this clock and all children of this clock
//
void Clock::FrameStep(uint64_t elapsedHPC)
{
	m_frameCount++;

	if (m_isPaused)
	{
		elapsedHPC = 0;
	}
	else
	{
		// Apply scaling
		elapsedHPC = (uint64_t)((double)elapsedHPC * m_scale);
	}

	double elapsedSeconds = TimeSystem::PerformanceCountToSeconds(elapsedHPC);

	m_frameData.m_seconds = elapsedSeconds;
	m_frameData.m_hpc = elapsedHPC;

	m_totalData.m_seconds += elapsedSeconds;
	m_totalData.m_hpc += elapsedHPC;

	// Step all children
	for (int clockIndex = 0; clockIndex < (int) m_childClocks.size(); ++clockIndex)
	{
		m_childClocks[clockIndex]->FrameStep(elapsedHPC);
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the time data of the clock to all zero
//
void Clock::ResetTimeData()
{
	m_lastFrameHPC = GetPerformanceCounter();
	memset(&m_frameData, 0, sizeof(TimeData_t));
	memset(&m_totalData, 0, sizeof(TimeData_t));
	m_frameCount = 0;
}


//-----------------------------------------------------------------------------------------------
// Adds the given child clock to the list of children of this clock
//
void Clock::AddChild(Clock* child)
{
	if (child != nullptr)
	{
		m_childClocks.push_back(child);
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the clock's time scale to the one given
//
void Clock::SetScale(float newScale)
{
	m_scale = newScale;
}


//-----------------------------------------------------------------------------------------------
// Sets the pause state of the clock to the one given
//
void Clock::SetPaused(bool pauseState)
{
	m_isPaused = pauseState;
}


//-----------------------------------------------------------------------------------------------
// Returns this clock's frame time (scaled) in seconds
//
float Clock::GetDeltaTime() const
{
	return (float) m_frameData.m_seconds;
}


//-----------------------------------------------------------------------------------------------
// Returns this clock's frame time (scaled) in performance counts
//
uint64_t Clock::GetFrameHPC() const
{
	return m_frameData.m_hpc;
}


//-----------------------------------------------------------------------------------------------
// Returns this clock's total time passed (after scales), in seconds
//
float Clock::GetTotalSeconds() const
{
	return (float) m_totalData.m_seconds;
}


//-----------------------------------------------------------------------------------------------
// Returns this clock's total time passed (after scales), in performance counts
//
uint64_t Clock::GetTotalHPC() const
{
	return m_totalData.m_hpc;
}
