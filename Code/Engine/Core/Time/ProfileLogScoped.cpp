#include "Engine/Core/Time/Profiler.hpp"
#include "Engine/Core/Time/ProfileLogScoped.hpp"

ProfileLogScoped::ProfileLogScoped(const char* name)
{
	if (Profiler::GetInstance() != nullptr)
	{
		m_name = name;
		Profiler::PushMeasurement(name);
	}
}

ProfileLogScoped::~ProfileLogScoped()
{
	if (Profiler::GetInstance() != nullptr)
	{
		Profiler::PopMeasurement();
	}
}

