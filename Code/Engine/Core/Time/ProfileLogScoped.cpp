#include "Engine/Core/Time/Profiler.hpp"
#include "Engine/Core/Time/ProfileLogScoped.hpp"

ProfileLogScoped::ProfileLogScoped(const char* name)
{
	m_name = name;
	Profiler::PushMeasurement(name);
}

ProfileLogScoped::~ProfileLogScoped()
{
	Profiler::PopMeasurement();
}

