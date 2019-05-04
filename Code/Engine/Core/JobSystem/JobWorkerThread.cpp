/************************************************************************/
/* File: JobWorkerThread.cpp
/* Author: Andrew Chase
/* Date: May 3rd, 2019
/* Description: Implementation of the JobWorkerThread class
/************************************************************************/
#include "Engine/Core/JobSystem/Job.hpp"
#include "Engine/Core/JobSystem/JobSystem.hpp"
#include "Engine/Core/JobSystem/JobWorkerThread.hpp"


//------------------------------------------------------------------------------
// Constructor
//
JobWorkerThread::JobWorkerThread(const char* name, WorkerThreadFlags flags, JobSystem* jobSystem)
	: m_name(name)
	, m_workerFlags(flags)
	, m_jobSystem(jobSystem)
{
	m_threadHandle = std::thread(&JobWorkerThread::JobWorkerThreadEntry, this);
}


//------------------------------------------------------------------------------
// Destructor
//
JobWorkerThread::~JobWorkerThread()
{
	m_isRunning = false;
	m_threadHandle.join();
}


//------------------------------------------------------------------------------
// Thread entry function for *ALL* JobWorkerThreads
//
void JobWorkerThread::JobWorkerThreadEntry()
{
	while (m_isRunning)
	{
		// Get a job
		Job* nextJob = DequeueJobForExecution();

		// Execute it if we got one
		if (nextJob != nullptr)
		{
			nextJob->Execute();

			// Put it in finished list
			MarkJobAsFinished(nextJob);
		}
		else
		{
			// No jobs for us exist, might as well sleep a little since there might not be a job soon
			// Also reduces the amount of contention for accessing the vectors on the JobSystem
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Gets a job from the JobSystem to execute that satisfies this worker thread's flags
// Removes the job from the queued list and puts it in the running list
//
Job* JobWorkerThread::DequeueJobForExecution()
{
	Job* jobToExecute = nullptr;

	m_jobSystem->m_queuedLock.lock();
	{
		int numQueuedJobs = (int)m_jobSystem->m_queuedJobs.size();

		for (int queueIndex = 0; queueIndex = numQueuedJobs; ++queueIndex)
		{
			uint32_t jobFlags = m_jobSystem->m_queuedJobs[queueIndex]->GetJobFlags();

			if (jobFlags & m_workerFlags == jobFlags)
			{
				jobToExecute = m_jobSystem->m_queuedJobs[queueIndex];
				m_jobSystem->m_queuedJobs.erase(m_jobSystem->m_queuedJobs.begin() + queueIndex);
				break;
			}
		}
	}
	m_jobSystem->m_queuedLock.unlock();


	m_jobSystem->m_runningLock.lock();
	{
		m_jobSystem->m_runningJobs.push_back(jobToExecute);
	}
	m_jobSystem->m_runningLock.unlock();

	return jobToExecute;
}


//-----------------------------------------------------------------------------------------------
// Removes the given job from the running list and adds it to the finished list
//
void JobWorkerThread::MarkJobAsFinished(Job* finishedJob)
{
	std::vector<Job*>& runningJobs = m_jobSystem->m_runningJobs;
	std::vector<Job*>& finishedJobs = m_jobSystem->m_finishedJobs;

	m_jobSystem->m_runningLock.lock();
	{
		int numRunningJobs = (int)runningJobs.size();

		for (int runningIndex = 0; runningIndex < numRunningJobs; ++runningIndex)
		{
			if (runningJobs[runningIndex] == finishedJob)
			{
				runningJobs.erase(runningJobs.begin() + runningIndex);
			}
		}
	}
	m_jobSystem->m_runningLock.unlock();

	m_jobSystem->m_finishedLock.lock();
	{
		finishedJobs.push_back(finishedJob);
	}
	m_jobSystem->m_finishedLock.unlock();
}
