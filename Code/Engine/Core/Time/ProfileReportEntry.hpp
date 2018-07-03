/************************************************************************/
/* File: ProfileReportEntry.hpp
/* Author: Andrew Chase
/* Date: June 30th, 2018
/* Description: Class to represent a single entry in a profile report
				Represents a set of measurements for a single scope profile
/************************************************************************/
#pragma once
#include <map>
#include <string>


class ProfileMeasurement;

class ProfileReportEntry
{
public:
	//-----Public Methods-----

	ProfileReportEntry(const std::string& name);
	~ProfileReportEntry();

	// Initializers
	void												PopulateTree(ProfileMeasurement* measurement);
	void												PopulateFlat(ProfileMeasurement* measurement);

	// Mutators
	void												AccumulateData(ProfileMeasurement* measurement);
	ProfileReportEntry*									GetOrCreateReportEntryForChild(const std::string childName);
	void												RecursivelyCalculatePercentTimes();


public:
	//-----Public Data-----

	// General
	std::string											m_name;
	unsigned int										m_callCount;

	// Timing
	uint64_t											m_totalTime;	// also called inclusive time
	uint64_t											m_selfTime;		// also called exclusive time, total time - sum of time of children, time it took by itself
	double												m_percentTime;

	// Traversing
	ProfileReportEntry* 								m_parent;
	std::map<std::string, ProfileReportEntry*>			m_children;

};
