#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time/Profiler.hpp"
#include "Engine/Core/Time/ProfileReport.hpp"
#include "Engine/Core/Time/ProfileMeasurement.hpp"

Profiler* Profiler::s_instance = nullptr;	// Singleton instance

Profiler::Profiler()
	: m_currentStack(nullptr)
	, m_previousStack(nullptr)
	, m_currentReportIndex(-1)
	, m_reportType(REPORT_TYPE_TREE)
{
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

	if (m_currentStack == nullptr)
	{
		m_currentStack = measurement;
	}
	else
	{
		measurement->m_parent = m_currentStack;
		m_currentStack->m_children.push_back(measurement);
		m_currentStack = measurement;
	}

}

void Profiler::PopMeasurement()
{
	ASSERT_OR_DIE(m_currentStack != nullptr, Stringf("Error::Profiler::PopStack called when the current stack was empty"));

	m_currentStack->Finish();
	m_currentStack = m_currentStack->m_parent;
}

void Profiler::DestroyStack(ProfileMeasurement* stack)
{
	ASSERT_OR_DIE(stack->m_parent == nullptr, "Error: Profiler::DestroyStack called on a stack with a parented head");
	delete stack; // Recursive deconstructor; will delete all children
}


unsigned int Profiler::IncrementIndexWithWrapAround(unsigned int currentIndex) const
{
	unsigned int nextIndex = currentIndex + 1;

	if (nextIndex >= PROFILER_MAX_REPORT_COUNT)
	{
		nextIndex = 0;
	}

	return nextIndex;
}

unsigned int Profiler::DecrementIndexWithWrapAround(unsigned int currentIndex) const
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