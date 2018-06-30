#include "Engine/Core/Time/Time.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
#include "Engine/Core/Time/ProfileScoped.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor - starts the stopwatch
//
ProfileScoped::ProfileScoped(const std::string& name)
	: m_name(name)
{
	m_startHPC = GetPerformanceCounter();
}


//-----------------------------------------------------------------------------------------------
// Destructor - prints the resulting time and cleans up
//
ProfileScoped::~ProfileScoped()
{
	uint64_t deltaHPC = GetPerformanceCounter() - m_startHPC;
	float milliseconds = (float) TimeSystem::PerformanceCountToSeconds(deltaHPC) * 1000.f;

	ConsolePrintf("Profile for \"%s\" took %f milliseconds", m_name.c_str(), milliseconds);
	DebuggerPrintf("Profile for \"%s\" took %f milliseconds\n", m_name.c_str(), milliseconds);
}
