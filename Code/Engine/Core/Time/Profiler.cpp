/************************************************************************/
/* File: Profiler.cpp
/* Author: Andrew Chase
/* Date: June 30th, 2018
/* Description: Implementation of the Profiler class
/************************************************************************/
#include "Engine/Core/Time/Time.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time/Profiler.hpp"
#include "Engine/Core/Time/ProfileReport.hpp"
#include "Engine/Core/Time/ProfileMeasurement.hpp"
#include "Engine/Core/Time/ProfileReportEntry.hpp"

// Singleton instance
Profiler* Profiler::s_instance = nullptr;	

// C functions
unsigned int	IncrementIndexWithWrapAround(unsigned int currentIndex);
unsigned int	DecrementIndexWithWrapAround(unsigned int currentIndex);
void			DestroyStack(ProfileMeasurement* stack);


//-----------------------------------------------------------------------------------------------
// Constructor
//
Profiler::Profiler()
	: m_generatingReportType(REPORT_TYPE_FLAT)
	, m_isOpen(true)
	, m_isGeneratingReports(true)
	, m_currentFrameNumber(0)
{
	// Initialize all reports to nullptr
	for (int i = 0; i < PROFILER_MAX_REPORT_COUNT; ++i)
	{
		m_measurements[i] = nullptr;
		m_reports[i] = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Profiler::~Profiler()
{
	// Profile stacks
	for (int i = 0; i < PROFILER_MAX_REPORT_COUNT; ++i)
	{
		if (m_measurements[i] != nullptr)
		{
			delete m_measurements[i];
			m_measurements[i] = nullptr;
		}
	}


	// Reports
	for (int i = 0; i < PROFILER_MAX_REPORT_COUNT; ++i)
	{
		if (m_reports[i] != nullptr)
		{
			delete m_reports[i];
			m_reports[i] = nullptr;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Static function used to create the singleton instance (constructor is private)
//
void Profiler::Initialize()
{
	s_instance = new Profiler();
}


//-----------------------------------------------------------------------------------------------
// Static function used to destroy the singleton instance (destructor is private)
//
void Profiler::Shutdown()
{
	delete s_instance;
	s_instance = nullptr;
}

// Temporary for debugging
void PrintRecursive(const std::string& indent, ProfileReportEntry* stack)
{
	double totalTime	= 1000.f * TimeSystem::PerformanceCountToSeconds(stack->m_totalTime);
	double selfTime		= 1000.f * TimeSystem::PerformanceCountToSeconds(stack->m_selfTime);
	std::string text	= indent + Stringf(" Name: %s - CallCount: %i - Total time: %f - Exclusive Time: %f\n", stack->m_name.c_str(), stack->m_callCount, totalTime, selfTime);
	DebuggerPrintf(text.c_str());

	std::map<std::string, ProfileReportEntry*>::iterator itr;
	for (itr = stack->m_children.begin(); itr != stack->m_children.end(); itr++)
	{
		PrintRecursive(indent + "-", itr->second);
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the profiler results to screen
//
void Profiler::Render()
{
	// Only render if the profiler is open
	if (m_isOpen)
	{
		if (m_reports[0] != nullptr)
		{
			PrintRecursive("", m_reports[0]->m_rootEntry);
		}
		else
		{
			DebuggerPrintf("THE REPORT AT 0 WAS NULL\n");
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Marks the start of the profile frame
//
void Profiler::BeginFrame()
{
	s_instance->m_currentFrameNumber++;

	// Set up the stack frame
	if (s_instance->m_measurements[0] != nullptr) // Check if we already have a measurement
	{
		if (s_instance->m_measurements[PROFILER_MAX_REPORT_COUNT - 1] != nullptr)
		{
			DestroyStack(s_instance->m_measurements[PROFILER_MAX_REPORT_COUNT - 1]);
		}

		for (int i = PROFILER_MAX_REPORT_COUNT - 1; i > 0; --i)
		{
			s_instance->m_measurements[i] = s_instance->m_measurements[i - 1];
		}

		s_instance->PopMeasurement(); // Pop the stack at 0, which finalizes the last frame, and should make [0] null

		ASSERT_OR_DIE(s_instance->m_measurements[0] == nullptr, "Error: Profiler::MarkFrame called before the previous frame could finish"); // if not null, someone forgot to pop after a push somewhere
	}

	// Making a report for the last frame before starting the report generation of the previous frame
	if (s_instance->m_measurements[1] != nullptr && s_instance->m_isGeneratingReports)
	{
		ProfileReport* report = BuildReportForFrame(s_instance->m_measurements[1]);
		s_instance->PushReport(report);
	}

	s_instance->PushMeasurement("Frame"); // Push a new frame into [0]
}


//-----------------------------------------------------------------------------------------------
// Checks for input on the profile system, for when the profiler is open
//
void Profiler::ProcessInput()
{
}


//-----------------------------------------------------------------------------------------------
// Builds the report for the previous frame data, DOES NOT mark the end of a profile frame
//
void Profiler::EndFrame()
{
}


//-----------------------------------------------------------------------------------------------
// Pushes a new profile measurement to the current stack, starting the stack if there isn't one yet
//
void Profiler::PushMeasurement(const char* name)
{
	ProfileMeasurement* measurement = new ProfileMeasurement(name);

	// Set the parent relationship, and the appropriate frame number (though current stack frame number should equal current frame number on profiler)
	if (s_instance->m_measurements[0] == nullptr)
	{
		measurement->m_frameNumber = s_instance->m_currentFrameNumber;
		s_instance->m_measurements[0] = measurement;
	}
	else
	{
		measurement->m_frameNumber = s_instance->m_measurements[0]->m_frameNumber;
		measurement->m_parent = s_instance->m_measurements[0];
		s_instance->m_measurements[0]->m_children.push_back(measurement);
		s_instance->m_measurements[0] = measurement;
	}

}


//-----------------------------------------------------------------------------------------------
// Sets the current stack pointer back one level, to point at the current measurement's parent
//
void Profiler::PopMeasurement()
{
	ASSERT_OR_DIE(s_instance->m_measurements[0] != nullptr, Stringf("Error::Profiler::PopStack called when the current stack was empty"));

	s_instance->m_measurements[0]->Finish();
	s_instance->m_measurements[0] = s_instance->m_measurements[0]->m_parent;
}


//-----------------------------------------------------------------------------------------------
// Sets whether we should be generating new reports or not
// If starting generation, it will regenerate all reports
//
void Profiler::SetReportGeneration(bool shouldGenerate, eReportType reportType)
{
	// Get switched state
	bool justStartedGenerating = (shouldGenerate && !s_instance->m_isGeneratingReports);
	bool justSwitchedType = (shouldGenerate && s_instance->m_isGeneratingReports && s_instance->m_generatingReportType != reportType);

	// Update state
	s_instance->m_generatingReportType = reportType;
	s_instance->m_isGeneratingReports = shouldGenerate;

	// Check to generate the report list
	if (justStartedGenerating || justSwitchedType)
	{
		s_instance->UpdateReports();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns whether or not the profiler is open, used for rendering
//
bool Profiler::IsProfilerOpen()
{
	return s_instance->m_isOpen;
}


//-----------------------------------------------------------------------------------------------
// Returns the profiler singleton instance
//
Profiler* Profiler::GetInstance()
{
	return s_instance;
}


//-----------------------------------------------------------------------------------------------
// Builds the report to represent the given performance frame
//
ProfileReport* Profiler::BuildReportForFrame(ProfileMeasurement* stack)
{
	ProfileReport* report = new ProfileReport(stack->m_frameNumber);

	switch (s_instance->m_generatingReportType)
	{
	case REPORT_TYPE_TREE:
		report->InitializeAsTreeReport(stack);
		break;
	case REPORT_TYPE_FLAT:
		report->InitializeAsFlatReport(stack);
		break;
	default:
		break;
	}

	return report;
}


//-----------------------------------------------------------------------------------------------
// Adds the given report to the list in the front, removing and deleting the last if it is full
//
void Profiler::PushReport(ProfileReport* report)
{
	// Check the end
	if (m_reports[PROFILER_MAX_REPORT_COUNT - 1] != nullptr)
	{
		delete m_reports[PROFILER_MAX_REPORT_COUNT - 1];
	}

	// Shift all report pointers over one
	for (int i = PROFILER_MAX_REPORT_COUNT - 1; i > 0; --i)
	{
		s_instance->m_reports[i] = s_instance->m_reports[i - 1];
	}

	// Put the new report at the front
	s_instance->m_reports[0] = report;
}


//-----------------------------------------------------------------------------------------------
// Constructs all the reports in the parallel report array to reflect the current measurement array
//
void Profiler::UpdateReports()
{
	for (int index = 0; index < PROFILER_MAX_REPORT_COUNT; ++index)
	{
		// Cleanup first (slow but safe)
		if (m_reports[index] != nullptr)
		{
			delete m_reports[index];
			m_reports[index] = nullptr;
		}

		if (m_measurements[index] != nullptr)
		{
			m_reports[index] = BuildReportForFrame(m_measurements[index]);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Recursively deletes the measurement stack
//
void DestroyStack(ProfileMeasurement* stack)
{
	ASSERT_OR_DIE(stack->m_parent == nullptr, "Error: Profiler::DestroyStack called on a stack with a parented head");
	delete stack; // Recursive deconstructor; will delete all children
}


//-----------------------------------------------------------------------------------------------
// Increments the index to wrap around the report array
//
unsigned int IncrementIndexWithWrapAround(unsigned int currentIndex)
{
	unsigned int nextIndex = currentIndex + 1;

	if (nextIndex >= PROFILER_MAX_REPORT_COUNT)
	{
		nextIndex = 0;
	}

	return nextIndex;
}


//-----------------------------------------------------------------------------------------------
// Decrements the index to wrap around the report array
//
unsigned int DecrementIndexWithWrapAround(unsigned int currentIndex)
{
	unsigned int nextIndex = currentIndex - 1;

	if (nextIndex < 0)
	{
		nextIndex = PROFILER_MAX_REPORT_COUNT - 1;
	}

	return nextIndex;
}
