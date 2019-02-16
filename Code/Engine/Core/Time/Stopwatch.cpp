/************************************************************************/
/* File: Stopwatch.cpp
/* Author: Andrew Chase
/* Date: March 15th, 2018
/* Description: Implementation of the Stopwatch class
/************************************************************************/
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/Time.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"

TODO("Pause stopwatch");
//-----------------------------------------------------------------------------------------------
// Constructor, refers to the master clock of nullptr is passed
//
Stopwatch::Stopwatch(Clock* referenceClock)
	: m_referenceClock(referenceClock)
{
	if (m_referenceClock == nullptr)
	{
		m_referenceClock = Clock::GetMasterClock();
	}

	Reset();
}


//-----------------------------------------------------------------------------------------------
// Default constructor
//
Stopwatch::Stopwatch()
{
	m_referenceClock = Clock::GetMasterClock();

	Reset();
}


//-----------------------------------------------------------------------------------------------
// Sets the start and interval hpc to the current time, effectively "setting the stopwatch to 0"
//
void Stopwatch::Reset()
{
	m_startHPC = m_referenceClock->GetTotalHPC();
	m_endHPC = m_startHPC;
}


//-----------------------------------------------------------------------------------------------
// Sets the stopwatch clock to the one specified 
//
void Stopwatch::SetClock(Clock* clock)
{
	m_referenceClock = clock;

	if (m_referenceClock == nullptr)
	{
		m_referenceClock = Clock::GetMasterClock();
	}
}


//-----------------------------------------------------------------------------------------------
// Set the stopwatch start time to now and the interval time to be seconds into the future
//
void Stopwatch::SetInterval(float seconds)
{
	uint64_t interval = TimeSystem::SecondsToPerformanceCount(seconds);
	m_startHPC = m_referenceClock->GetTotalHPC();
	m_endHPC = m_startHPC + interval;
}


//-----------------------------------------------------------------------------------------------
// Sets the clock's elapsed time to be the time specified
//
void Stopwatch::SetElapsedTime(float secondsElapsed)
{
	// Save off the interval length to preserve it
	uint64_t intervalLength = m_endHPC - m_startHPC;

	uint64_t elapsedHPC = TimeSystem::SecondsToPerformanceCount(secondsElapsed);
	uint64_t currentHPC = m_referenceClock->GetTotalHPC();

	m_startHPC = currentHPC - elapsedHPC;
	m_endHPC = m_startHPC + intervalLength;
}


//-----------------------------------------------------------------------------------------------
// Checks if the interval has elapsed, and if so resets the stopwatch and returns true
// Otherwise returns false
//
bool Stopwatch::CheckAndReset()
{
	if (HasIntervalElapsed())
	{
		Reset();
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// If the stopwatch current time is passed the interval, will decrement tne stopwatch elapsed time
// by interval length and return true. Otherwise does nothing and returns false
//
bool Stopwatch::DecrementByIntervalOnce()
{
	if (HasIntervalElapsed())
	{
		uint64_t interval = m_endHPC - m_startHPC;

		m_startHPC += interval;
		m_endHPC += interval;
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Decrements the stopwatch by interval continuously until time elasped is less than the interval,
// and return the number of decrements
//
int Stopwatch::DecrementByIntervalAll()
{
	uint64_t currentHPC = m_referenceClock->GetTotalHPC();
	uint64_t interval = m_endHPC - m_startHPC;

	int numElapses = 0;
	while (m_endHPC < currentHPC)
	{
		m_startHPC += interval;
		m_endHPC += interval;
		numElapses++;
	}

	return numElapses;
}


//-----------------------------------------------------------------------------------------------
// Returns the time elapsed since the stopwatch was last reset
//
float Stopwatch::GetElapsedTime() const
{
	uint64_t currentHPC = m_referenceClock->GetTotalHPC();
	uint64_t elapsedHPC = currentHPC - m_startHPC;

	return (float) TimeSystem::PerformanceCountToSeconds(elapsedHPC);
}


//-----------------------------------------------------------------------------------------------
// Returns a value between 0 and 1.0, where 0 represents no time passed, and 1 means the full
// interval has passed
//
float Stopwatch::GetElapsedTimeNormalized() const
{
	float elapsedSeconds = GetElapsedTime();
	float intervalSeconds = (float) TimeSystem::PerformanceCountToSeconds(m_endHPC - m_startHPC);

	return (elapsedSeconds / intervalSeconds);
}


//-----------------------------------------------------------------------------------------------
// Returns the time in seconds until the stopwatch's interval will elapse
//
float Stopwatch::GetTimeUntilIntervalEnds() const
{
	uint64_t currentHPC = m_referenceClock->GetTotalHPC();

	float endSeconds = (float) TimeSystem::PerformanceCountToSeconds(m_endHPC);
	float currentSeconds = (float) TimeSystem::PerformanceCountToSeconds(currentHPC);

	float diff = endSeconds - currentSeconds;
	float timeUntilIntervalEnds = ClampFloat(diff, 0.f, diff);

	return timeUntilIntervalEnds;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the current time is passed the interval marker (stopwatch running longer than
// the set interval), false otherwise
//
bool Stopwatch::HasIntervalElapsed() const
{
	uint64_t currentHPC = m_referenceClock->GetTotalHPC();
	
	return (currentHPC >= m_endHPC);
}


//-----------------------------------------------------------------------------------------------
// Returns the total time of the stopwatch's reference clock
//
float Stopwatch::GetTotalSeconds() const
{
	return m_referenceClock->GetTotalSeconds();
}


//-----------------------------------------------------------------------------------------------
// Returns the delta time of the stopwatch's reference clock
//
float Stopwatch::GetDeltaSeconds() const
{
	return m_referenceClock->GetDeltaTime();
}
