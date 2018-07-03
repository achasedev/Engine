/************************************************************************/
/* File: ProfileReport.hpp
/* Author: Andrew Chase
/* Date: June 30th, 2018
/* Description: Class to represent a profile result for a single frame
/************************************************************************/
#pragma once
#include <vector>
#include "Engine/Core/Time/ProfileReport.hpp"

#define PROFILER_MAX_REPORT_COUNT (512)

class ProfileMeasurement;

class Profiler
{
public:
	//-----Public Methods-----

	// Initialization
	static void									Initialize();
	static void									Shutdown();
	
	// Per frame
	static void									BeginFrame();
	void										ProcessInput();
	void										Render();
	static void									EndFrame();
												
	// Mutators											
	static void									PushMeasurement(const char* name);
	static void									PopMeasurement();
	static void									SetReportGeneration(bool shouldGenerate, eReportType reportType);
	
	// Accessors
	static bool									IsProfilerOpen();
	static Profiler*							GetInstance();


private:
	//-----Private Methods-----

	// Singleton class, so no constructor/destructing publicly
	Profiler();
	~Profiler();
	Profiler(const Profiler& copy) = delete;

	static ProfileReport*						BuildReportForFrame(ProfileMeasurement* stack);
	void										PushReport(ProfileReport* report);

	void										UpdateReports(); // Used when we need to regenerate all the reports at once, for starting generation or switching types


private:
	//-----Private Data-----

	// Stacks, used for measuring, 0 is always the latest one
	ProfileMeasurement*		m_measurements[PROFILER_MAX_REPORT_COUNT];

	// Reports, 0 is always the latest
	eReportType				m_generatingReportType;
	ProfileReport*			m_reports[PROFILER_MAX_REPORT_COUNT]; // Parallel array to measurement, but LAGS BEHIND ONE FRAME (report at 0 is measurement 1)

	// State
	bool					m_isOpen;
	bool					m_isGeneratingReports;
	int						m_currentFrameNumber;

	// Singleton instance
	static Profiler*		s_instance;

};
