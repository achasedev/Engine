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

	static void Initialize();
	static void Shutdown();

	void ProcessInput();
	void Render();

	static void BeginFrame();
	static void EndFrame();

	static void PushMeasurement(const char* name);
	static void PopMeasurement();

	static bool			IsProfilerOpen();
	static Profiler* GetInstance();


private:
	//-----Private Methods-----

	Profiler();
	~Profiler();
	Profiler(const Profiler& copy) = delete;

	void								BuildReportForFrame(ProfileMeasurement* stack);

	static unsigned int					IncrementIndexWithWrapAround(unsigned int currentIndex);
	static unsigned int					DecrementIndexWithWrapAround(unsigned int currentIndex);

	static void							DestroyStack(ProfileMeasurement* stack);


private:
	//-----Private Data-----

	ProfileMeasurement* m_currentStack;
	ProfileMeasurement* m_previousStack;

	int m_currentReportIndex;
	eReportType m_reportType;
	ProfileReport* m_reports[PROFILER_MAX_REPORT_COUNT];

	bool m_isOpen;
	static Profiler* s_instance;	// Singleton instance

};
