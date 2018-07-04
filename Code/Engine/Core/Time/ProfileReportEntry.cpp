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
// Sorts each node's children in descending order by total time, for printing
//
void ProfileReportEntry::RecursivelySortChildrenByTotalTime()
{
	// Sort this node's children in descending order by total time
	for (int endIndex = (int) m_children.size() - 1; endIndex > 0; --endIndex)
	{
		for (int startIndex = 0; startIndex < endIndex; ++startIndex)
		{
			if (m_children[startIndex]->m_totalTime < m_children[startIndex + 1]->m_totalTime)
			{
				ProfileReportEntry* temp = m_children[endIndex];
				m_children[endIndex] = m_children[startIndex];
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
	std::string totalText;
	double totalValue = TimeSystem::PerformanceCountToSeconds(m_totalTime);
	if (totalValue < 0.001) // Less than a millisecond, use microseconds
	{
		totalValue *= (1000.0 * 1000.0);
		totalText = Stringf("%.2f us", totalValue);
	}
	else if (totalValue < 1.0) // Less than a second but greater than a millisecond, use milliseconds 
	{
		totalValue *= 1000.0;
		totalText = Stringf("%.2f ms", totalValue);
	}
	else
	{
		totalText = Stringf("%.2f s", totalValue);
	}

	std::string selfText;
	double selfValue = TimeSystem::PerformanceCountToSeconds(m_selfTime);
	if (selfValue < 0.001) // Less than a millisecond, use microseconds
	{
		selfValue *= (1000.0 * 1000.0);
		selfText = Stringf("%.2f us", selfValue);
	}
	else if (selfValue < 1.0) // Less than a second but greater than a millisecond, use milliseconds 
	{
		selfValue *= 1000.0;
		selfText = Stringf("%.2f ms", selfValue);
	}
	else
	{
		selfText = Stringf("%.2f s", selfValue);
	}

	std::string text = Stringf("%-*s%-*s%-*i%-*.2f%-*s%-*.2f%-*s", 
		indent, "", 44 - indent, m_name.c_str(), 8, m_callCount, 10, m_percentOfFrameTime, 10, totalText.c_str(), 10, m_percentOfSelfTime, 10, selfText.c_str());

	return text;
}
