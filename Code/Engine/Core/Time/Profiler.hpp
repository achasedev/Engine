/************************************************************************/
/* File: ProfileReport.hpp
/* Author: Andrew Chase
/* Date: June 30th, 2018
/* Description: Class to represent a profile result for a single frame
/************************************************************************/
#pragma once
#include <vector>
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Time/ProfileReport.hpp"

#define PROFILER_MAX_REPORT_COUNT (128)

class Gif;
class Mesh;
class MaterialInstance;
class ProfileMeasurement;

class Profiler
{
public:
	//-----Public Methods-----

	// Initialization
	static void									Initialize();
	static void										InitializeUILayout();
	static void										InitializeConsoleCommands();
	static void									Shutdown();
	
	// Per frame
	static void									BeginFrame();
	void										ProcessInput();
	void											ProcessMouseInput();
	void												ProcessLeftClick();
	void												ProcessRightClick();
	void											ProcessKeyboardInput();
	void										Render();
	static void									EndFrame();
												
	// Mutators											
	static void									PushMeasurement(const char* name);
	static void									PopMeasurement();
	static void									SetGeneratingReportType(eReportType reportType);

	static void									Show();
	static void									Hide();
	static void									Pause();
	static void									Resume();

	void										SetSelectionState(int firstIndex, int secondIndex, bool isSelecting);


	// Accessors
	static bool									IsProfilerOpen();
	static Profiler*							GetInstance();

	// Producers
	float										GetAverageTotalTime(int startIndex, int endIndex) const;


private:
	//-----Private Methods-----

	// Singleton class, so no constructor/destructing publicly
	Profiler();
	~Profiler();
	Profiler(const Profiler& copy) = delete;

	static ProfileReport*						BuildReportForFrame(ProfileMeasurement* stack);
	void										PushReport(ProfileReport* report);

	void										FlushReports(); // Used when we need to regenerate all the reports at once, for starting generation or switching types


	// UI Rendering
	void										RenderTitleInfo() const;
	void										RenderGraph() const;
	void										RenderData() const;
	void											RecursivelyPrintEntry(unsigned int indent, AABB2& drawBounds, ProfileReportEntry* entry) const;


private:
	//-----Private Data-----

	// Stacks, used for measuring, 0 is always the latest one
	ProfileMeasurement*		m_measurements[PROFILER_MAX_REPORT_COUNT];

	// Reports, 0 is always the latest
	eReportType				m_generatingReportType;
	ProfileReport*			m_reports[PROFILER_MAX_REPORT_COUNT]; // Parallel array to measurement, but LAGS BEHIND ONE FRAME (report at 0 is measurement 1)

	// State
	bool					m_isOpen;
	bool					m_isPaused;
	int						m_currentFrameNumber;
	float					m_framesPerSecond;

	// Graph selection
	int						m_firstSelectionIndex;
	int						m_secondSelectionIndex;
	bool					m_isSelectingFrames;

	// Singleton instance
	static Profiler*		s_instance;

	// UI
	static AABB2 s_fpsBorderBounds;
	static AABB2 s_frameBorderBounds;
	static AABB2 s_titleBorderBounds;
	static AABB2 s_graphBorderBounds;
	static AABB2 s_viewDataBorderBounds;
	static AABB2 s_viewHeadingBorderBounds;
	static AABB2 s_graphDetailsBorderBounds;
	static AABB2 s_rottyTopsBorderBounds;

	static AABB2 s_titleBounds;
	static AABB2 s_fpsBounds;
	static AABB2 s_frameBounds;
	static AABB2 s_graphBounds;
	static AABB2 s_viewDataBounds;
	static AABB2 s_viewHeadingBounds;
	static AABB2 s_graphDetailsBounds;
	static AABB2 s_rottyTopsBackgroundBounds;
	static AABB2 s_rottyTopsTextureBounds;

	static float s_titleFontSize;
	static float s_fpsFrameFontSize;
	static float s_viewHeadingFontSize;
	static float s_viewDataFontSize;
	static float s_borderThickness;

	static std::string s_titleText;
	static std::string s_fpsframeText;
	static std::string s_viewHeadingText;

	static Rgba s_backgroundColor;
	static Rgba s_borderColor;
	static Rgba s_fontColor;
	static Rgba s_fontHighlightColor;
	static Rgba s_graphRedColor;
	static Rgba s_graphYellowColor;
	static Rgba s_graphGreenColor;
	static Rgba s_graphSelectionColor;

	static Mesh* s_graphMesh;

	// RottyTops
	static Gif*					s_rottyTopsGif;
	static MaterialInstance*	s_rottyTopsMaterial;

};
