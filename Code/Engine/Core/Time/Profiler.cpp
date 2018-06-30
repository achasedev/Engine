#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time/Profiler.hpp"
#include "Engine/Core/Time/ProfileReport.hpp"
#include "Engine/Core/Time/ProfileMeasurement.hpp"
#include "Engine/Core/Time/ProfileReportEntry.hpp"

Profiler* Profiler::s_instance = nullptr;	// Singleton instance

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


Profiler::~Profiler()
{
	if (m_currentStack != nullptr)
	{
		delete m_currentStack;
	}

	if (m_previousStack != nullptr)
	{
		delete m_previousStack;
	}
}


void Profiler::Initialize()
{
	s_instance = new Profiler();
}

void Profiler::Shutdown()
{
	delete s_instance;
	s_instance = nullptr;
}

#include "Engine/Core/Time/Time.hpp"
void PrintRecursive(const std::string& indent, ProfileReportEntry* stack)
{
	float totalTime		= 1000.f * TimeSystem::PerformanceCountToSeconds(stack->m_totalTime);
	float selfTime		= 1000.f * TimeSystem::PerformanceCountToSeconds(stack->m_selfTime);
	std::string text = indent + Stringf(" Name: %s - CallCount: %i - Total time: %f - Exclusive Time: %f\n", stack->m_name.c_str(), stack->m_callCount, totalTime, selfTime);
	DebuggerPrintf(text.c_str());

	std::map<std::string, ProfileReportEntry*>::iterator itr;
	for (itr = stack->m_children.begin(); itr != stack->m_children.end(); itr++)
	{
		PrintRecursive(indent + "-", itr->second);
	}
}

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

void Profiler::BeginFrame()
{
	// Set up the stack frame
	if (s_instance->m_currentStack != nullptr)
	{
		if (s_instance->m_previousStack != nullptr)
		{
			s_instance->DestroyStack(s_instance->m_previousStack);
		}

		s_instance->m_previousStack = s_instance->m_currentStack;
		s_instance->PopMeasurement();

		ASSERT_OR_DIE(s_instance->m_currentStack == nullptr, "Error: Profiler::MarkFrame called before the previous frame could finish"); // if not null, someone forgot to pop after a push somewhere
	}

	s_instance->PushMeasurement("Frame");
}

void Profiler::EndFrame()
{
	if (s_instance->m_previousStack != nullptr)
	{
		s_instance->BuildReportForFrame(s_instance->m_previousStack);
	}
}

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

void Profiler::PopMeasurement()
{
	ASSERT_OR_DIE(s_instance->m_currentStack != nullptr, Stringf("Error::Profiler::PopStack called when the current stack was empty"));

	s_instance->m_currentStack->Finish();
	s_instance->m_currentStack = s_instance->m_currentStack->m_parent;
}

bool Profiler::IsProfilerOpen()
{
	return s_instance->m_isOpen;
}

Profiler* Profiler::GetInstance()
{
	return s_instance;
}

void Profiler::DestroyStack(ProfileMeasurement* stack)
{
	ASSERT_OR_DIE(stack->m_parent == nullptr, "Error: Profiler::DestroyStack called on a stack with a parented head");
	delete stack; // Recursive deconstructor; will delete all children
}


unsigned int Profiler::IncrementIndexWithWrapAround(unsigned int currentIndex)
{
	unsigned int nextIndex = currentIndex + 1;

	if (nextIndex >= PROFILER_MAX_REPORT_COUNT)
	{
		nextIndex = 0;
	}

	return nextIndex;
}

unsigned int Profiler::DecrementIndexWithWrapAround(unsigned int currentIndex)
{
	unsigned int nextIndex = currentIndex - 1;

	if (nextIndex < 0)
	{
		nextIndex = PROFILER_MAX_REPORT_COUNT - 1;
	}

	return nextIndex;
}

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