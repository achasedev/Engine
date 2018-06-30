/************************************************************************/
/* File: ProfileReportEntry.cpp
/* Author: Andrew Chase
/* Date: June 30th, 2018
/* Description:  Implementation of the ProfileReportEntry class
/************************************************************************/
#include "Engine/Core/Time/Time.hpp"
#include "Engine/Core/Time/ProfileReportEntry.hpp"
#include "Engine/Core/Time/ProfileMeasurement.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
ProfileReportEntry::ProfileReportEntry(const std::string& name)
	: m_name(name)
	, m_callCount(0)
	, m_totalTime(0)
	, m_selfTime(0)
	, m_percentTime(0.f)
	, m_parent(nullptr)
{
}


//-----------------------------------------------------------------------------------------------
// Recursively deletes all children
//
ProfileReportEntry::~ProfileReportEntry()
{
	std::map<std::string, ProfileReportEntry*>::iterator itr = m_children.begin();

	for (itr; itr != m_children.end(); itr++)
	{
		delete itr->second;
	}

	m_children.clear();
}


//-----------------------------------------------------------------------------------------------
// Fills this report in a tree style, maintain hierarchy
//
void ProfileReportEntry::PopulateTree(ProfileMeasurement* measurement)
{
	AccumulateData(measurement);

	for (int childIndex = 0; childIndex < (int) measurement->m_children.size(); ++childIndex)
	{
		ProfileReportEntry* childEntry = GetOrCreateReportEntryForChild(measurement->m_children[childIndex]->m_name);
		childEntry->PopulateTree(measurement->m_children[childIndex]);
	}
}


//-----------------------------------------------------------------------------------------------
// Fills this report in a flat style, discarding hierarchy
//
void ProfileReportEntry::PopulateFlat(ProfileMeasurement* measurement)
{
	for (int childIndex = 0; childIndex < (int) measurement->m_children.size(); ++childIndex)
	{
		ProfileMeasurement* childMeasurement = measurement->m_children[childIndex];
		ProfileReportEntry* childEntry = GetOrCreateReportEntryForChild(childMeasurement->m_name);

		childEntry->AccumulateData(childMeasurement);
		PopulateFlat(childMeasurement);
	}
}


//-----------------------------------------------------------------------------------------------
// Accumulates the data from the given measurement on this entry
//
void ProfileReportEntry::AccumulateData(ProfileMeasurement* measurement)
{
	m_callCount++;
	m_totalTime += measurement->GetTotalTime_Inclusive();
	m_selfTime	+= measurement->GetTotalTime_Exclusive();
}


//-----------------------------------------------------------------------------------------------
// Returns the child entry on this entry given by childName, creating one if one doesn't exist
//
ProfileReportEntry* ProfileReportEntry::GetOrCreateReportEntryForChild(const std::string childName)
{
	bool entryExists = m_children.find(childName) != m_children.end();

	if (entryExists)
	{
		return m_children.at(childName);
	}

	// Child doesn't exist, so make a new one and add it to the list
	ProfileReportEntry* newChild = new ProfileReportEntry(childName);
	newChild->m_parent = this;
	m_children[childName] = newChild;

	return newChild;
}


//-----------------------------------------------------------------------------------------------
// Calculates the percentage times for all entries in this report, called on the root
//
void ProfileReportEntry::RecursivelyCalculatePercentTimes()
{
	if (m_parent == nullptr)
	{
		m_percentTime = 1.0f;
	}
	else
	{
		double mySeconds			= TimeSystem::PerformanceCountToSeconds(m_totalTime);
		double parentSeconds		= TimeSystem::PerformanceCountToSeconds(m_parent->m_totalTime);

		m_percentTime = (mySeconds / parentSeconds);
	}

	std::map<std::string, ProfileReportEntry*>::iterator itr;

	for (itr = m_children.begin(); itr != m_children.end(); itr++)
	{
		itr->second->RecursivelyCalculatePercentTimes();
	}
}
