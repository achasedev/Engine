/************************************************************************/
/* File: ProfileReport.cpp
/* Author: Andrew Chase
/* Date: June 30th, 2018
/* Description: Implementation of the ProfileReport class
/************************************************************************/
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time/ProfileReport.hpp"
#include "Engine/Core/Time/ProfileReportEntry.hpp"
#include "Engine/Core/Time/ProfileMeasurement.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
ProfileReport::ProfileReport()
	: m_rootEntry(nullptr)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
ProfileReport::~ProfileReport()
{
	delete m_rootEntry;
}


//-----------------------------------------------------------------------------------------------
// Fills this report in a tree format, so it can be printed showing hierarchy
//
void ProfileReport::InitializeAsTreeReport(ProfileMeasurement* stack)
{
	ASSERT_OR_DIE(m_rootEntry == nullptr, "Error: ProfileReport::InitializeAsTreeReport called on an already initialized report");

	m_rootEntry = new ProfileReportEntry(stack->m_name);
	m_rootEntry->PopulateTree(stack);

	Finalize();
}


//-----------------------------------------------------------------------------------------------
// Fills this report in a flat format, so it can be printed flat
//
void ProfileReport::InitializeAsFlatReport(ProfileMeasurement* stack)
{
	ASSERT_OR_DIE(m_rootEntry == nullptr, "Error: ProfileReport::InitializeAsFlatReport called on an already initialized report");

	m_rootEntry = new ProfileReportEntry(stack->m_name);
	m_rootEntry->PopulateFlat(stack);

	Finalize();
}


//-----------------------------------------------------------------------------------------------
// Performs any tasks that need to be done after the report data is completely filled in
//
void ProfileReport::Finalize()
{
	// Calculate all percent times
	m_rootEntry->RecursivelyCalculatePercentTimes();

	TODO("Sort entries by total time");
}
