/************************************************************************/
/* File: JobWorkerThread.hpp
/* Author: Andrew Chase
/* Date: May 3rd, 2019
/* Description: Class to represent a single thread within the JobSystem
/************************************************************************/
#pragma once
#include "Engine/Core/JobSystem/JobSystem.hpp"
#include <thread>
#include <string>



class Job;
class JobSystem;

class JobWorkerThread
{
public:
	//-----Public Methods-----

	JobWorkerThread(const char* name, WorkerThreadFlags flags, JobSystem* jobSystem);
	~JobWorkerThread();

	inline std::string	GetName() const { return m_name; }
	inline bool			IsRunning() const { return m_isRunning; }
	inline JobSystem*	GetOwningJobSystem() const { return m_jobSystem; }
	inline std::thread&	GetThreadHandle() { return m_threadHandle; }

	void				StopRunning();
	void				Join();


private:
	//-----Private Methods

	void JobWorkerThreadEntry();
	Job* DequeueJobForExecution();
	void MarkJobAsFinished(Job* finishedJob);


private:
	//-----Private Data-----

	std::string			m_name;
	std::thread			m_threadHandle;
	WorkerThreadFlags	m_workerFlags;
	bool				m_isRunning = true;
	JobSystem*			m_jobSystem = nullptr;

};
