/************************************************************************/
/* File: ProfileReportEntry.hpp
/* Author: Andrew Chase
/* Date: June 30th, 2018
/* Description: Class to represent a single entry in a profile report
				Represents a set of measurements for a single scope profile
/************************************************************************/
#pragma once
#include <vector>
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
	void												AccumulateData(ProfileReportEntry* entry);
	ProfileReportEntry*									GetOrCreateReportEntryForChild(const std::string childName);
	void												RecursivelyCalculatePercentTimes(double frameDurationSeconds);
	
	void												RecursivelySortChildrenBySelfTIme();
	void												RecursivelySortChildrenByTotalTime();

	std::string											GetAsStringForUI(unsigned int indent) const;


public:
	//-----Public Data-----

	// General
	std::string											m_name;
	unsigned int										m_callCount;

	// Timing
	uint64_t											m_totalTime;			// also called inclusive time
	uint64_t											m_selfTime;				// also called exclusive time, total time - sum of time of children, time it took by itself
	double												m_percentOfFrameTime;	// What fraction of the frame was spent in us including our children
	double												m_percentOfSelfTime;	// What fraction of our total time was exclusively spent in us, not counting children

	// Traversing
	ProfileReportEntry* 								m_parent;
	std::vector<ProfileReportEntry*>					m_children;

};
