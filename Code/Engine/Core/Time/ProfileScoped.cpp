/************************************************************************/
/* File: ProfileScoped.cpp
/* Author: Andrew Chase
/* Date: June 30th, 2018
/* Description: Implementation of the ProfileScoped class
/************************************************************************/
#include "Engine/Core/Time/Time.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
#include "Engine/Core/Time/ProfileScoped.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor - Stores the start performance count
//
ProfileScoped::ProfileScoped(const std::string& name)
	: m_name(name)
{
	m_startHPC = GetPerformanceCounter();
}


//-----------------------------------------------------------------------------------------------
// Destructor - prints the resulting time elapsed
//
ProfileScoped::~ProfileScoped()
{
	uint64_t deltaHPC = GetPerformanceCounter() - m_startHPC;
	float timeTaken = (float) TimeSystem::PerformanceCountToSeconds(deltaHPC) * 1000.f;

	
	if (DevConsole::GetInstance() != nullptr)
	{
		if (timeTaken < 1000.f)
		{
			ConsolePrintf("Profile for \"%s\" took %f milliseconds", m_name.c_str(), timeTaken);
		}
		else
		{
			timeTaken *= (1.f / 1000.f);
			ConsolePrintf("Profile for \"%s\" took %f seconds", m_name.c_str(), timeTaken);
		}
	}

	if (timeTaken < 1000.f)
	{
		DebuggerPrintf("Profile for \"%s\" took %f milliseconds", m_name.c_str(), timeTaken);
	}
	else
	{
		timeTaken *= (1.f / 1000.f);
		DebuggerPrintf("Profile for \"%s\" took %f seconds", m_name.c_str(), timeTaken);
	}
}
