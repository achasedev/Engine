/************************************************************************/
/* File: Profiler.cpp
/* Author: Andrew Chase
/* Date: June 30th, 2018
/* Description: Implementation of the Profiler class
/************************************************************************/
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Core/Time/Time.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time/Profiler.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Core/Time/ProfileReport.hpp"
#include "Engine/Core/Time/ProfileMeasurement.hpp"
#include "Engine/Core/Time/ProfileReportEntry.hpp"
#include "Engine/Rendering/Materials/Material.hpp"

// Singleton instance
Profiler*		Profiler::s_instance = nullptr;	

// UI constants
AABB2			Profiler::s_fpsBorderBounds;
AABB2			Profiler::s_frameBorderBounds;
AABB2			Profiler::s_titleBorderBounds;
AABB2			Profiler::s_graphBorderBounds;
AABB2			Profiler::s_viewDataBorderBounds;


AABB2			Profiler::s_titleBounds;
AABB2			Profiler::s_fpsBounds;
AABB2			Profiler::s_frameBounds;
AABB2			Profiler::s_graphBounds;
AABB2			Profiler::s_viewDataBounds;
AABB2			Profiler::s_viewHeadingBorderBounds;
AABB2			Profiler::s_viewHeadingBounds;


float			Profiler::s_titleFontSize;
float			Profiler::s_fpsFrameFontSize;
float			Profiler::s_viewHeadingFontSize;
float			Profiler::s_viewDataFontSize;
float			Profiler::s_borderThickness;

std::string		Profiler::s_titleText;
std::string		Profiler::s_fpsframeText;
std::string		Profiler::s_viewHeadingText;


Rgba			Profiler::s_backgroundColor	= Rgba(0,0,0,180);
Rgba			Profiler::s_borderColor		= Rgba(15, 60, 120, 200);
Rgba			Profiler::s_fontColor			= Rgba(100, 100, 100, 255);


// C functions
unsigned int	IncrementIndexWithWrapAround(unsigned int currentIndex);
unsigned int	DecrementIndexWithWrapAround(unsigned int currentIndex);
void			DestroyStack(ProfileMeasurement* stack);


//-----------------------------------------------------------------------------------------------
// Constructor
//
Profiler::Profiler()
	: m_generatingReportType(REPORT_TYPE_TREE)
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

	// Set static constants
	AABB2 bounds = Renderer::GetUIBounds();
	Vector2 dimensions = bounds.GetDimensions();

	s_titleFontSize			= 48.f;
	s_fpsFrameFontSize		= 48.f;
	s_viewHeadingFontSize	= 20.f;
	s_viewDataFontSize		= 20.f;
	s_borderThickness		= 5.f;

	s_titleBorderBounds = AABB2(Vector2(0.f, dimensions.y - s_titleFontSize - (2.f * s_borderThickness)), 
		Vector2(0.333f * dimensions.x, dimensions.y));

	s_fpsBorderBounds = AABB2(s_titleBorderBounds.GetBottomRight(), 
		Vector2(s_titleBorderBounds.maxs.x + ((dimensions.x - s_titleBorderBounds.GetDimensions().x) * 0.5f), bounds.maxs.y));

	s_fpsBounds = s_fpsBorderBounds;
	s_fpsBounds.AddPaddingToSides(-s_borderThickness, -s_borderThickness);

	s_frameBorderBounds = AABB2(s_fpsBorderBounds.GetBottomRight(), bounds.maxs);

	s_frameBounds = s_frameBorderBounds;
	s_frameBounds.AddPaddingToSides(-s_borderThickness, -s_borderThickness);

	s_graphBorderBounds = AABB2(Vector2(0.f, 0.8f * dimensions.y), Vector2(bounds.maxs.x, s_fpsBorderBounds.mins.y));

	s_viewHeadingBorderBounds = AABB2(Vector2(0.f, s_graphBorderBounds.mins.y - s_viewHeadingFontSize - (2.f * s_borderThickness)), s_graphBorderBounds.GetBottomRight());
	s_viewHeadingBounds = s_viewHeadingBorderBounds;
	s_viewHeadingBounds.AddPaddingToSides(-s_borderThickness, -s_borderThickness);


	s_viewDataBorderBounds = AABB2(bounds.mins, s_viewHeadingBorderBounds.GetBottomRight());
	
	s_viewDataBounds = s_viewDataBorderBounds;
	s_viewDataBounds.AddPaddingToSides(-s_borderThickness, -s_borderThickness);

	s_graphBounds = s_graphBorderBounds;
	s_graphBounds.AddPaddingToSides(-s_borderThickness, -s_borderThickness);

	s_titleBounds = s_titleBorderBounds;
	s_titleBounds.AddPaddingToSides(-s_borderThickness, -s_borderThickness);
}


//-----------------------------------------------------------------------------------------------
// Static function used to destroy the singleton instance (destructor is private)
//
void Profiler::Shutdown()
{
	delete s_instance;
	s_instance = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Draws the profiler results to screen
//
void Profiler::Render()
{
	// Only render if the profiler is open
	if (m_isOpen)
	{
		Renderer* renderer = Renderer::GetInstance();
		renderer->SetCurrentCamera(renderer->GetUICamera());
		
		// TITLE, FPS, FRAME COUNT
		RenderTitleInfo();

		// GRAPH
		RenderGraph();

		// DATA
		RenderData();
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

	// Update the fps if we can
	if (s_instance->m_measurements[1] != nullptr)
	{
		float frameTime = (float) TimeSystem::PerformanceCountToSeconds(s_instance->m_measurements[1]->GetTotalTime_Inclusive());
		s_instance->m_framesPerSecond = (1.0f / frameTime);
	}
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
// Renders the title information (title, fps, frame count) to screen
//
void Profiler::RenderTitleInfo() const
{
	Renderer* renderer	= Renderer::GetInstance();
	Material* material	= AssetDB::GetSharedMaterial("UI");
	BitmapFont* font	= AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png");

	// Title bounds
	renderer->Draw2DQuad(s_titleBorderBounds,	AABB2::UNIT_SQUARE_OFFCENTER, s_borderColor,		material);
	renderer->Draw2DQuad(s_titleBounds,			AABB2::UNIT_SQUARE_OFFCENTER, s_backgroundColor,		material);
												
	// Fps bounds								
	renderer->Draw2DQuad(s_fpsBorderBounds,		AABB2::UNIT_SQUARE_OFFCENTER, s_borderColor,		material);
	renderer->Draw2DQuad(s_fpsBounds,			AABB2::UNIT_SQUARE_OFFCENTER, s_backgroundColor,		material);
												
	// Frame count bounds						
	renderer->Draw2DQuad(s_frameBorderBounds,	AABB2::UNIT_SQUARE_OFFCENTER, s_borderColor,		material);
	renderer->Draw2DQuad(s_frameBounds,			AABB2::UNIT_SQUARE_OFFCENTER, s_backgroundColor, material);

	// Text
	std::string frameText	= Stringf("FRAME: %*i",		6,	m_currentFrameNumber);
	std::string fpsText		= Stringf("FPS: %*.2f",		8,	m_framesPerSecond);

	renderer->DrawTextInBox2D("PROFILER",	s_titleBounds,		Vector2::ZERO, s_titleFontSize,		TEXT_DRAW_OVERRUN, font, s_fontColor);
	renderer->DrawTextInBox2D(frameText,	s_frameBounds,		Vector2::ZERO, s_fpsFrameFontSize,	TEXT_DRAW_OVERRUN, font, s_fontColor);
	renderer->DrawTextInBox2D(fpsText,		s_fpsBounds,		Vector2::ZERO, s_fpsFrameFontSize,	TEXT_DRAW_OVERRUN, font, s_fontColor);
}


//-----------------------------------------------------------------------------------------------
// Renders the performance graph to screen
//
void Profiler::RenderGraph() const
{
	Renderer* renderer	= Renderer::GetInstance();
	Material* material	= AssetDB::GetSharedMaterial("UI");
	BitmapFont* font	= AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png");

	renderer->Draw2DQuad(s_graphBorderBounds,	AABB2::UNIT_SQUARE_OFFCENTER, s_borderColor,		material);
	renderer->Draw2DQuad(s_graphBounds,			AABB2::UNIT_SQUARE_OFFCENTER, s_backgroundColor,	material);
}


//-----------------------------------------------------------------------------------------------
// Renders the latest frame data to screen
//
void Profiler::RenderData() const
{
	Renderer* renderer	= Renderer::GetInstance();
	Material* material	= AssetDB::GetSharedMaterial("UI");
	BitmapFont* font	= AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png");

	renderer->Draw2DQuad(s_viewHeadingBorderBounds,		AABB2::UNIT_SQUARE_OFFCENTER, s_borderColor, material);
	renderer->Draw2DQuad(s_viewHeadingBounds,			AABB2::UNIT_SQUARE_OFFCENTER, s_backgroundColor, material);
	renderer->Draw2DQuad(s_viewDataBorderBounds,		AABB2::UNIT_SQUARE_OFFCENTER, s_borderColor,	material);
	renderer->Draw2DQuad(s_viewDataBounds,				AABB2::UNIT_SQUARE_OFFCENTER, s_backgroundColor, material);

	//std::string headingText = Stringf("FUNCTION NAME%*sCALLS%*s%% TOTAL%*sTIME%*s%% SELF%*sTIME", 12, "", 5, "", 8, "", 6, "", 8, "");
	std::string headingText = Stringf("%-*s%-*s%-*s%-*s%-*s%-*s", 
		44, "FUNCTION NAME", 8, "CALLS", 10, "% TOTAL", 10, "TIME", 10, "% SELF", 10, "TIME");
	
	renderer->DrawTextInBox2D(headingText, s_viewHeadingBounds, Vector2::ZERO, s_viewHeadingFontSize, TEXT_DRAW_OVERRUN, font, s_fontColor);


	// Render all the report entries
	ProfileReport* report = m_reports[0];
	if (report == nullptr) { return; }

	std::string dataString;
	ConstructDataString(0, dataString, report->m_rootEntry);

	renderer->DrawTextInBox2D(dataString, s_viewDataBounds, Vector2::ZERO, s_viewDataFontSize, TEXT_DRAW_OVERRUN, font, s_fontColor);
}


//-----------------------------------------------------------------------------------------------
// Prints the current entry on the list, and
//
void Profiler::ConstructDataString(unsigned int indent, std::string& out_string, ProfileReportEntry* entry) const
{
	std::string text = entry->GetAsStringForUI(indent);

	if (out_string.size() != 0)
	{
		out_string += "\n";
	}

	out_string += text;

	// Recursively call on children
	for (int childIndex = 0; childIndex < (int) entry->m_children.size(); ++childIndex)
	{
		ConstructDataString(indent + 1, out_string, entry->m_children[childIndex]);
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
