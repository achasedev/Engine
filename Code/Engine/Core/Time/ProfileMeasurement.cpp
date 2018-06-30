#include "Engine/Core/Time/Time.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time/ProfileMeasurement.hpp"

ProfileMeasurement::ProfileMeasurement(const char* name)
	: m_name(name)
{
	m_startHPC = GetPerformanceCounter();
	m_parent = nullptr;
}

ProfileMeasurement::~ProfileMeasurement()
{
	for (int childIndex = 0; childIndex < (int) m_children.size(); ++childIndex)
	{
		delete m_children[childIndex];
	}

	m_children.clear();
}

void ProfileMeasurement::Finish()
{
	m_endHPC = GetPerformanceCounter();
}

uint64_t ProfileMeasurement::GetTotalTime_Inclusive() const
{
	ASSERT_OR_DIE(m_endHPC > m_startHPC, "Error: ProfileMeasurment::GetTotalElapsedTime() called on measurement with zero elapsed time");

	uint64_t elapsedHPC = m_endHPC - m_startHPC;

	return elapsedHPC;
}

uint64_t ProfileMeasurement::GetTotalTime_Exclusive() const
{
	uint64_t totalElapsedHPC = m_endHPC - m_startHPC;

	uint64_t totalChildHPC = 0;

	for (int childIndex = 0; childIndex < (int) m_children.size(); ++childIndex)
	{
		totalChildHPC += m_children[childIndex]->GetTotalTime_Inclusive();
	}

	uint64_t exclusiveTime = totalElapsedHPC - totalChildHPC;

	return exclusiveTime;
}

