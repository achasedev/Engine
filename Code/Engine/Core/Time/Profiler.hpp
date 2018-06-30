#pragma once
#include <vector>

#define PROFILE_LOG_SCOPE(tag) ProfileLogScoped __timer_ ##__LINE__ ## (tag)
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

	static void BeginFrame();
	static void EndFrame();

	void PushMeasurement(const char* name);
	void PopMeasurement();

	void DestroyStack(ProfileMeasurement* stack);
	  

private:
	//-----Private Methods-----

	Profiler();
	~Profiler();
	Profiler(const Profiler& copy) = delete;

	void BuildReportForFrame(ProfileMeasurement* stack);

	unsigned int IncrementIndexWithWrapAround(unsigned int currentIndex) const;
	unsigned int DecrementIndexWithWrapAround(unsigned int currentIndex) const;


private:
	//-----Private Data-----

	ProfileMeasurement* m_currentStack;
	ProfileMeasurement* m_previousStack;

	int m_currentReportIndex;
	eReportType m_reportType;
	ProfileReport* m_reports[PROFILER_MAX_REPORT_COUNT];

	static Profiler* s_instance;	// Singleton instance

};
