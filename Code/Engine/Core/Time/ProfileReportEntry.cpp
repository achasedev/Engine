/************************************************************************/
/* File: ProfileReportEntry.cpp
/* Author: Andrew Chase
/* Date: June 30th, 2018
/* Description:  Implementation of the ProfileReportEntry class
/************************************************************************/
#include "Engine/Core/Time/Time.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
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
	, m_percentOfFrameTime(0.f)
	, m_parent(nullptr)
{
}


//-----------------------------------------------------------------------------------------------
// Recursively deletes all children
//
ProfileReportEntry::~ProfileReportEntry()
{
	for (int index = 0; index < (int) m_children.size(); ++index)
	{
		delete m_children[index];
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
	ProfileReportEntry* currEntry = GetOrCreateReportEntryForChild(measurement->m_name);
	currEntry->AccumulateData(measurement);

	for (int childIndex = 0; childIndex < (int) measurement->m_children.size(); ++childIndex)
	{
		PopulateFlat(measurement->m_children[childIndex]);
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
// Accumulates the data from the given report entry on this entry
//
void ProfileReportEntry::AccumulateData(ProfileReportEntry* entry)
{
	m_callCount += entry->m_callCount;
	m_totalTime += entry->m_totalTime;
	m_selfTime += entry->m_selfTime;
}


//-----------------------------------------------------------------------------------------------
// Returns the child entry on this entry given by childName, creating one if one doesn't exist
//
ProfileReportEntry* ProfileReportEntry::GetOrCreateReportEntryForChild(const std::string childName)
{
	for (int index = 0; index < (int) m_children.size(); ++index)
	{
		if (m_children[index]->m_name == childName)
		{
			return m_children[index];
		}
	}

	// Child doesn't exist, so make a new one and add it to the list
	ProfileReportEntry* newChild = new ProfileReportEntry(childName);
	newChild->m_parent = this;
	m_children.push_back(newChild);

	return newChild;
}


//-----------------------------------------------------------------------------------------------
// Calculates the percentage times for all entries in this report, called on the root
//
void ProfileReportEntry::RecursivelyCalculatePercentTimes(double frameDurationSeconds)
{
	double totalTimeSeconds = TimeSystem::PerformanceCountToSeconds(m_totalTime);
	double selfTimeSeconds = TimeSystem::PerformanceCountToSeconds(m_selfTime);

	m_percentOfFrameTime = 100.0 * (totalTimeSeconds / frameDurationSeconds); // * 100 to put it in a readable format as percents
	m_percentOfSelfTime = 100.0 * (selfTimeSeconds / frameDurationSeconds);

	for (int index = 0; index < (int) m_children.size(); ++index)
	{
		m_children[index]->RecursivelyCalculatePercentTimes(frameDurationSeconds);
	}
}


//-----------------------------------------------------------------------------------------------
// Sorts each node's children in descending order by self time, for printing
//
void ProfileReportEntry::RecursivelySortChildrenBySelfTIme()
{
	// Sort this node's children in descending order by self time
	bool done = false;
	while (!done)
	{
		done = true;

		for (int startIndex = 0; startIndex < (int) m_children.size() - 1; ++startIndex)
		{
			if (m_children[startIndex]->m_selfTime < m_children[startIndex + 1]->m_selfTime)
			{
				done = false;
				ProfileReportEntry* temp = m_children[startIndex + 1];
				m_children[startIndex + 1] = m_children[startIndex];
				m_children[startIndex] = temp;
			}
		}
	}

	// Recursively sort the children's children
	for (int index = 0; index < (int) m_children.size(); ++index)
	{
		m_children[index]->RecursivelySortChildrenBySelfTIme();
	}
}


//-----------------------------------------------------------------------------------------------
// Sorts each node's children in descending order by total time, for printing
//
void ProfileReportEntry::RecursivelySortChildrenByTotalTime()
{
	// Sort this node's children in descending order by total time
	bool done = false;
	while (!done)
	{
		done = true;

		for (int startIndex = 0; startIndex < (int) m_children.size() - 1; ++startIndex)
		{
			if (m_children[startIndex]->m_totalTime < m_children[startIndex + 1]->m_totalTime)
			{
				done = false;
				ProfileReportEntry* temp = m_children[startIndex + 1];
				m_children[startIndex + 1] = m_children[startIndex];
				m_children[startIndex] = temp;
			}
		}
	}

	// Recursively sort the children's children
	for (int index = 0; index < (int) m_children.size(); ++index)
	{
		m_children[index]->RecursivelySortChildrenByTotalTime();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the entry as a string for UI rendering
//
std::string ProfileReportEntry::GetAsStringForUI(unsigned int indent) const
{
	// Additional formatting for the time values - using labels
	double totalValue = TimeSystem::PerformanceCountToSeconds(m_totalTime);
	std::string totalText = Stringf("%*.2f%*s", 4, totalValue, 3, "s");
	if (totalValue < 0.1) // Less than 100 milliseconds, use milliseconds
	{
		totalValue *= 1000.0;
		totalText = Stringf("%*.2f%*s", 4, totalValue, 3, "ms");

		if (totalValue < 0.1) // Less than 100 microseconds, use microseconds
		{
			totalValue *= 1000.0;
			totalText = Stringf("%*.2f%*s", 4, totalValue, 3, "us");
		}
	}


	double selfValue = TimeSystem::PerformanceCountToSeconds(m_selfTime);
	std::string selfText = Stringf("%*.2f%*s", 4, selfValue, 3, "s");
	if (selfValue < 0.1) // Less than 100 milliseconds, use milliseconds
	{
		selfValue *= 1000.0;
		selfText = Stringf("%*.2f%*s", 4, selfValue, 3, "ms");

		if (selfValue < 0.1) // Less than 100 microseconds, use microseconds
		{
			selfValue *= 1000.0;
			selfText = Stringf("%*.2f%*s", 4, selfValue, 3, "us");
		}
	}

	std::string percentFrameText = Stringf("%.2f %%", m_percentOfFrameTime);
	std::string percentSelfText = Stringf("%.2f %%", m_percentOfSelfTime);

	std::string text = Stringf("%-*s%-*s%*i%*s%*s%*s%*s", 
		indent, "", 44 - indent, m_name.c_str(), 8, m_callCount, 10, percentFrameText.c_str(), 10, totalText.c_str(), 10, percentSelfText.c_str(), 10, selfText.c_str());

	return text;
}
