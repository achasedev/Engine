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
	: m_currentStack(nullptr)
	, m_previousStack(nullptr)
	, m_currentReportIndex(0)
	, m_reportType(REPORT_TYPE_TREE)
	, m_isOpen(true)
{
	// Initialize all reports to nullptr
	for (int i = 0; i < PROFILER_MAX_REPORT_COUNT; ++i)
	{
		m_reports[i] = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Profiler::~Profiler()
{
	// Profile stacks

	if (m_currentStack != nullptr)
	{
		delete m_currentStack;
	}

	if (m_previousStack != nullptr)
	{
		delete m_previousStack;
	}


	// Reports
	for (int i = 0; i < PROFILER_MAX_REPORT_COUNT; ++i)
	{
		if (m_reports[i] != nullptr)
		{
			delete m_reports[i];
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
	if (m_reports[m_currentReportIndex] != nullptr)
	{
		PrintRecursive("", m_reports[m_currentReportIndex]->m_rootEntry);
	}
	else
	{
		DebuggerPrintf("THE REPORT AT THE CURRENT WAS NULL\n");
	}
}


//-----------------------------------------------------------------------------------------------
// Marks the start of the profile frame
//
void Profiler::BeginFrame()
{
	// Set up the stack frame
	if (s_instance->m_currentStack != nullptr)
	{
		if (s_instance->m_previousStack != nullptr)
		{
			DestroyStack(s_instance->m_previousStack);
		}

		s_instance->m_previousStack = s_instance->m_currentStack;
		s_instance->PopMeasurement();

		ASSERT_OR_DIE(s_instance->m_currentStack == nullptr, "Error: Profiler::MarkFrame called before the previous frame could finish"); // if not null, someone forgot to pop after a push somewhere
	}

	s_instance->PushMeasurement("Frame");
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
	if (s_instance->m_previousStack != nullptr)
	{
		s_instance->BuildReportForFrame(s_instance->m_previousStack);
	}
}


//-----------------------------------------------------------------------------------------------
// Pushes a new profile measurement to the current stack, starting the stack if there isn't one yet
//
void Profiler::PushMeasurement(const char* name)
{
	ProfileMeasurement* measurement = new ProfileMeasurement(name);

	if (s_instance->m_currentStack == nullptr)
	{
		s_instance->m_currentStack = measurement;
	}
	else
	{
		measurement->m_parent = s_instance->m_currentStack;
		s_instance->m_currentStack->m_children.push_back(measurement);
		s_instance->m_currentStack = measurement;
	}

}


//-----------------------------------------------------------------------------------------------
// Sets the current stack pointer back one level, to point at the current measurement's parent
//
void Profiler::PopMeasurement()
{
	ASSERT_OR_DIE(s_instance->m_currentStack != nullptr, Stringf("Error::Profiler::PopStack called when the current stack was empty"));

	s_instance->m_currentStack->Finish();
	s_instance->m_currentStack = s_instance->m_currentStack->m_parent;
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
void Profiler::BuildReportForFrame(ProfileMeasurement* stack)
{
	ProfileReport* report = new ProfileReport();

	switch (m_reportType)
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

	m_currentReportIndex = IncrementIndexWithWrapAround(m_currentReportIndex);

	if (m_reports[m_currentReportIndex] != nullptr)
	{
		delete m_reports[m_currentReportIndex];
	}

	m_reports[m_currentReportIndex] = report;
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
