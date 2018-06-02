/************************************************************************/
/* File: Time.cpp
/* Author: Andrew Chase
/* Date: March 2nd, 2018
/* Description: Implementation of the TimeSystem and LogProfileScope classes
/************************************************************************/	
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Engine/Core/Time/Time.hpp"
#include "Engine/Core/EngineCommon.hpp"


// Singleton TimeSystem instance
TimeSystem TimeSystem::s_timeSystem;


double TimeSystem::PerformanceCountToSeconds(const uint64_t hpc)
{
	return (double)hpc * s_timeSystem.m_secondsPerCount;

}


uint64_t TimeSystem::SecondsToPerformanceCount(double seconds)
{
	return (uint64_t) (seconds * (double) s_timeSystem.m_frequency);
}


TimeSystem::TimeSystem()
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	m_frequency = *(uint64_t*)&frequency;
	m_secondsPerCount = 1.0f / (double)m_frequency;
}


TimeSystem::~TimeSystem()
{
}


//----------------------------LogProfileScope Class---------------------------------------------------

LogProfileScope::LogProfileScope(const char* tag)
{
	m_tag = tag;
	m_startHPC = GetPerformanceCounter();
}

LogProfileScope::~LogProfileScope()
{
	uint64_t elapsed = GetPerformanceCounter() - m_startHPC;
	DebuggerPrintf("Profiler %s took %d seconds.", m_tag, elapsed);
}


//--------------------C FUNCTIONS--------------------

//-----------------------------------------------------------------------------------------------
// Returns the HPC of the system
//
uint64_t GetPerformanceCounter()
{
	LARGE_INTEGER currentCount;
	QueryPerformanceCounter( &currentCount );

	return *(uint64_t*)&currentCount;
}
