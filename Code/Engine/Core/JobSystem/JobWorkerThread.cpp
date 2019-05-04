/************************************************************************/
/* File: JobWorkerThread.cpp
/* Author: Andrew Chase
/* Date: May 3rd, 2019
/* Description: Implementation of the JobWorkerThread class
/************************************************************************/
#include "Engine/Core/JobSystem/JobSystem.hpp"
#include "Engine/Core/JobSystem/JobWorkerThread.hpp"


//---C Function-----------------------------------------------------------------
// Thread entry function for *ALL* JobWorkerThreads
//
void JobWorkerThreadEntry(JobWorkerThread* jobThread)
{
	JobSystem* jobSystem = jobThread->GetOwningJobSystem();

	while (jobThread->IsRunning())
	{
		// Get a job

		// Execute it if we got one

		// Put it in finished list
	}
}


//------------------------------------------------------------------------------
// Constructor
//
JobWorkerThread::JobWorkerThread(const char* name, WorkerThreadFlags flags, JobSystem* jobSystem)
	: m_name(name)
	, m_workerFlags(flags)
	, m_jobSystem(jobSystem)
{
	m_threadHandle = std::thread(JobWorkerThreadEntry, this);
}


//------------------------------------------------------------------------------
// Destructor
//
JobWorkerThread::~JobWorkerThread()
{
	m_isRunning = false;
	m_threadHandle.join();
}
