/************************************************************************/
/* File: ProfileReport.hpp
/* Author: Andrew Chase
/* Date: June 30th, 2018
/* Description: Class to represent a profile result for a single frame
/************************************************************************/
#pragma once
#include <vector>

#define PROFILER_MAX_REPORT_COUNT (128)

enum eReportType
{
	REPORT_TYPE_TREE,
	REPORT_TYPE_FLAT,
	NUM_REPORT_TYPES
};

class ProfileReport;
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

	// Accessors
	static bool									IsProfilerOpen();
	static Profiler*							GetInstance();


private:
	//-----Private Methods-----

	// Singleton class, so no constructor/destructing publicly
	Profiler();
	~Profiler();
	Profiler(const Profiler& copy) = delete;

	void										BuildReportForFrame(ProfileMeasurement* stack);


private:
	//-----Private Data-----

	// Stacks, used for measuring
	ProfileMeasurement* m_currentStack;
	ProfileMeasurement* m_previousStack;

	// Reports
	int m_currentReportIndex;
	eReportType m_reportType;
	ProfileReport* m_reports[PROFILER_MAX_REPORT_COUNT];

	// State
	bool m_isOpen;

	// Singleton instance
	static Profiler* s_instance;

};
