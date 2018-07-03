/************************************************************************/
/* File: ProfileMeasurement.cpp
/* Author: Andrew Chase
/* Date: June 30th, 2018
/* Description: Implementation of the ProfileMeasurement class
/************************************************************************/
#include "Engine/Core/Time/Time.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time/ProfileMeasurement.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
ProfileMeasurement::ProfileMeasurement(const char* name)
	: m_name(name)
{
	m_startHPC = GetPerformanceCounter();
	m_parent = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
ProfileMeasurement::~ProfileMeasurement()
{
	for (int childIndex = 0; childIndex < (int) m_children.size(); ++childIndex)
	{
		delete m_children[childIndex];
	}

	m_children.clear();
}


//-----------------------------------------------------------------------------------------------
// Sets the end time for the measurement
//
void ProfileMeasurement::Finish()
{
	m_endHPC = GetPerformanceCounter();
}


//-----------------------------------------------------------------------------------------------
// Returns the total time spent in this measurement (includes children)
//
uint64_t ProfileMeasurement::GetTotalTime_Inclusive() const
{
	//ASSERT_OR_DIE(m_endHPC > m_startHPC, "Error: ProfileMeasurment::GetTotalElapsedTime() called on measurement with zero elapsed time");

	uint64_t elapsedHPC = m_endHPC - m_startHPC;

	return elapsedHPC;
}


//-----------------------------------------------------------------------------------------------
// Returns the total time spent in this measurement alone, excluding child measurements
//
uint64_t ProfileMeasurement::GetTotalTime_Exclusive() const
{
	// Get total child time
	uint64_t totalChildHPC = 0;
	for (int childIndex = 0; childIndex < (int) m_children.size(); ++childIndex)
	{
		totalChildHPC += m_children[childIndex]->GetTotalTime_Inclusive();
	}

	// Calculate total - child time
	uint64_t totalElapsedHPC	= m_endHPC - m_startHPC;
	uint64_t exclusiveTime		= totalElapsedHPC - totalChildHPC;

	return exclusiveTime;
}
