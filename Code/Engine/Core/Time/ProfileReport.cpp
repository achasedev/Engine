#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time/ProfileReport.hpp"
#include "Engine/Core/Time/ProfileReportEntry.hpp"
#include "Engine/Core/Time/ProfileMeasurement.hpp"

ProfileReport::ProfileReport()
	: m_rootEntry(nullptr)
{
}

ProfileReport::~ProfileReport()
{
}

void ProfileReport::InitializeAsTreeReport(ProfileMeasurement* stack)
{
	ASSERT_OR_DIE(m_rootEntry == nullptr, "Error: ProfileReport::InitializeAsTreeReport called on an already initialized report");

	m_rootEntry = new ProfileReportEntry(stack->m_name);
	m_rootEntry->PopulateTree(stack);

	Finalize();
}

void ProfileReport::InitializeAsFlatReport(ProfileMeasurement* stack)
{
	ASSERT_OR_DIE(m_rootEntry == nullptr, "Error: ProfileReport::InitializeAsFlatReport called on an already initialized report");

	m_rootEntry = new ProfileReportEntry(stack->m_name);
	m_rootEntry->PopulateFlat(stack);

	Finalize();
}

void ProfileReport::Finalize()
{
	// Calculate all percent times
	m_rootEntry->RecursivelyCalculatePercentTimes();

	TODO("Sort entries by total time");
}
