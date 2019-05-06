/************************************************************************/
/* File: JobSystem.cpp
/* Author: Andrew Chase
/* Date: May the 4th (be with you) 2019
/* Description: 
/************************************************************************/
#include "Engine/Core/JobSystem/Job.hpp"
#include "Engine/Core/JobSystem/JobSystem.hpp"
#include "Engine/Core/JobSystem/JobWorkerThread.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

JobSystem* JobSystem::s_instance = nullptr;


//-----------------------------------------------------------------------------------------------
// Creates the singleton instance, does not create any worker threads
//
void JobSystem::Initialize()
{
	ASSERT_OR_DIE(s_instance == nullptr, "JobSystem::Initialize() called twice!");
	s_instance = new JobSystem();
}


//-----------------------------------------------------------------------------------------------
// Deletes the singleton instance
//
void JobSystem::Shutdown()
{
	ASSERT_OR_DIE(s_instance != nullptr, "JobSystem::Shutdown() called without being initialized");

	delete s_instance;
	s_instance = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Returns the singleton instance
//
JobSystem* JobSystem::GetInstance()
{
	return s_instance;
}


//-----------------------------------------------------------------------------------------------
// Creates a thread that will begin pulling jobs from the queue, with the given work flags
//
void JobSystem::CreateWorkerThread(const char* name, WorkerThreadFlags flags)
{
	JobWorkerThread* workerThread = new JobWorkerThread(name, flags, this);
	m_workerThreads.push_back(workerThread);
}


//-----------------------------------------------------------------------------------------------
// Tells the thread to finish the job it is currently working on and then destroys it
//
void JobSystem::DestroyWorkerThread(const char* name)
{
	int numThreads = (int)m_workerThreads.size();

	for (int threadIndex = 0; threadIndex < numThreads; ++threadIndex)
	{
		JobWorkerThread* workerThread = m_workerThreads[threadIndex];

		if (workerThread->GetName() == name)
		{
			m_workerThreads.erase(m_workerThreads.begin() + threadIndex);

			workerThread->StopRunning();
			workerThread->Join();

			delete workerThread;
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Tells all worker thread to finish the job they're executing, then join
//
void JobSystem::DestroyAllWorkerThreads()
{
	// Tell all to stop running first, for less potential waiting
	int numThreads = (int)m_workerThreads.size();

	for (int threadIndex = 0; threadIndex < numThreads; ++threadIndex)
	{
		m_workerThreads[threadIndex]->StopRunning();
	}

	// Now try to join all of them
	for (int threadIndex = 0; threadIndex < numThreads; ++threadIndex)
	{
		m_workerThreads[threadIndex]->Join();
	}

	m_workerThreads.clear();
}


//-----------------------------------------------------------------------------------------------
// Adds the given job to the "todo" list for worker threads to work on
// Returns the ID assigned to the job
//
int JobSystem::QueueJob(Job* job)
{
	job->m_jobID = m_nextJobID;

	m_queuedLock.lock();
	{
		m_queuedJobs.push_back(job);
	}
	m_queuedLock.unlock();

	m_nextJobID++;
	if (m_nextJobID < 0)
	{
		m_nextJobID = 0;
	}

	return job->m_jobID;
}


//-----------------------------------------------------------------------------------------------
// Clears and deletes all jobs that exist in the JobSystem
//
void JobSystem::DestroyAllJobs()
{
	// Queued - just delete them
	m_queuedLock.lock();
	{
		int numQueued = (int)m_queuedJobs.size();

		for (int queuedIndex = 0; queuedIndex < numQueued; ++queuedIndex)
		{
			delete m_queuedJobs[queuedIndex];
		}

		m_queuedJobs.clear();
	}

	m_queuedLock.unlock();

	// This list *SHOULD* be empty
	ASSERT_OR_DIE(m_runningJobs.size() == 0, "JobSystem destructor still had running jobs");

	// Finished jobs - Don't finalize, since we cannot guarantee anything still exists
	m_finishedLock.lock();
	{
		int numFinished = (int)m_finishedJobs.size();

		for (int finishedIndex = 0; finishedIndex < numFinished; ++finishedIndex)
		{
			delete m_finishedJobs[finishedIndex];
		}

		m_finishedJobs.clear();
	}
	m_finishedLock.unlock();
}


//-----------------------------------------------------------------------------------------------
// Returns the current status of the job given by the ID
//
JobStatus JobSystem::GetJobStatus(int jobID)
{
	bool jobFound = false;

	// Check queued jobs
	m_queuedLock.lock_shared();
	{
		int numQueued = (int)m_queuedJobs.size();

		for (int queuedIndex = 0; queuedIndex < numQueued; ++queuedIndex)
		{
			if (m_queuedJobs[queuedIndex]->m_jobID == jobID)
			{
				jobFound = true;
				break;
			}
		}
	}
	m_queuedLock.unlock_shared();

	if (jobFound)
	{
		return JOB_STATUS_QUEUED;
	}

	// Check running jobs
	m_runningLock.lock_shared();
	{
		int numRunning = (int)m_runningJobs.size();

		for (int runningIndex = 0; runningIndex < numRunning; ++runningIndex)
		{
			if (m_runningJobs[runningIndex]->m_jobID == jobID)
			{
				jobFound = true;
				break;
			}
		}
	}
	m_runningLock.unlock_shared();

	if (jobFound)
	{
		return JOB_STATUS_RUNNING;
	}

	// Check finished jobs
	m_finishedLock.lock_shared();
	{
		int numFinished = (int)m_finishedJobs.size();

		for (int finishedIndex = 0; finishedIndex < numFinished; ++finishedIndex)
		{
			if (m_finishedJobs[finishedIndex]->m_jobID == jobID)
			{
				jobFound = true;
				break;
			}
		}
	}
	m_finishedLock.unlock_shared();

	if (jobFound)
	{
		return JOB_STATUS_FINISHED;
	}

	return JOB_STATUS_NOT_FOUND;
}


//-----------------------------------------------------------------------------------------------
// Returns true if this job has finished running and is waiting for finalization
//
bool JobSystem::IsJobFinished(int jobID)
{
	bool jobFinished = false;

	// Check finished jobs
	m_finishedLock.lock_shared();
	{
		int numFinished = (int)m_finishedJobs.size();

		for (int finishedIndex = 0; finishedIndex < numFinished; ++finishedIndex)
		{
			if (m_finishedJobs[finishedIndex]->m_jobID == jobID)
			{
				jobFinished = true;
			}
		}
	}
	m_finishedLock.unlock_shared();

	return jobFinished;
}


//-----------------------------------------------------------------------------------------------
// Finalizes all jobs in the finished list
//
void JobSystem::FinalizeAllFinishedJobs()
{
	m_finishedLock.lock();
	{
		int numFinished = (int)m_finishedJobs.size();

		for (int finishedIndex = 0; finishedIndex < numFinished; ++finishedIndex)
		{
			m_finishedJobs[finishedIndex]->Finalize();
			delete m_finishedJobs[finishedIndex];
		}

		m_finishedJobs.clear();
	}
	m_finishedLock.unlock();
}


//------------------------------------------------------------------------------
// Finalizes all jobs in the finished list that are the given ID
//
void JobSystem::FinalizeAllFinishedJobsOfType(int jobType)
{
	m_finishedLock.lock();
	{
		int numFinished = (int)m_finishedJobs.size();

		for (int finishedIndex = 0; finishedIndex < numFinished; ++finishedIndex)
		{
			if (m_finishedJobs[finishedIndex]->m_jobType == jobType)
			m_finishedJobs[finishedIndex]->Finalize();
			delete m_finishedJobs[finishedIndex];
		}

		m_finishedJobs.clear();
	}
	m_finishedLock.unlock();
}


//-----------------------------------------------------------------------------------------------
// Waits until the given job is complete, then immediately finalizes and destroys it
//
void JobSystem::BlockUntilJobIsFinalized(int jobID)
{
	while (!IsJobFinished(jobID))
	{
		// Spinwait
	}

	// Job is done - find it and finalize it, then delete it
	m_finishedLock.lock();
	{
		int numFinished = (int)m_finishedJobs.size();

		for (int finishedIndex = 0; finishedIndex < numFinished; ++finishedIndex)
		{
			if (m_finishedJobs[finishedIndex]->m_jobID == jobID)
			{
				m_finishedJobs[finishedIndex]->Finalize();

				delete m_finishedJobs[finishedIndex];
				m_finishedJobs.erase(m_finishedJobs.begin() + finishedIndex);
				break;
			}
		}
	}
	m_finishedLock.unlock();
}


//-----------------------------------------------------------------------------------------------
// Waits until all jobs of the give type are in the finished list only, and when done so finalizes
// and deletes all
// *NOTE* Only guarentees that it will wait for all job types that are pushed BEFORE this function
// is called - Don't push any more after calling this!
//
void JobSystem::BlockUntilAllJobsOfTypeAreFinalized(int jobType)
{
	bool jobOfTypeStillQueuedOrRunning = true;

	while (jobOfTypeStillQueuedOrRunning)
	{
		jobOfTypeStillQueuedOrRunning = false;

		// Check queued jobs
		m_queuedLock.lock_shared();
		m_runningLock.lock_shared();
		{
			int numQueued = (int)m_queuedJobs.size();

			for (int queuedIndex = 0; queuedIndex < numQueued; ++queuedIndex)
			{
				if (m_queuedJobs[queuedIndex]->m_jobType == jobType)
				{
					jobOfTypeStillQueuedOrRunning = true;
					break;
				}
			}

			int numRunning = (int)m_runningJobs.size();
			for (int runningIndex = 0; runningIndex < numRunning; ++runningIndex)
			{
				if (m_runningJobs[runningIndex]->m_jobType == jobType)
				{
					jobOfTypeStillQueuedOrRunning = true;
					break;
				}
			}
		}
		m_runningLock.unlock_shared();
		m_queuedLock.unlock_shared();
	}

	// No jobs of the given type are queued or running...
	// *Technically* someone could push a new job of the given type RIGHT NOW, but they shouldn't
	// be pushing more after this function is called.....
	// i.e. only the thread calling this function should be the one pushing jobs of this type, at least
	// at the same time

	// Finalize all the finished jobs of the type
	m_finishedLock.lock();
	{
		int numFinished = (int)m_finishedJobs.size();

		for (int finishedIndex = numFinished - 1; finishedIndex >= 0; --finishedIndex)
		{
			if (m_finishedJobs[finishedIndex]->m_jobType == jobType)
			{
				m_finishedJobs[finishedIndex]->Finalize();

				delete m_finishedJobs[finishedIndex];
				m_finishedJobs.erase(m_finishedJobs.begin() + finishedIndex);
			}
		}
	}
	m_finishedLock.unlock();
}


//-----------------------------------------------------------------------------------------------
// Constructor
//
JobSystem::JobSystem()
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
JobSystem::~JobSystem()
{
	DestroyAllWorkerThreads();
	DestroyAllJobs();
}


//---C FUNCTION----------------------------------------------------------------------------------
// Shortcut/Helper function for queueing a job to the JobSystem singleton instance
// Returns the ID of the job
//
int QueueJob(Job* job)
{
	JobSystem* jobSystem = JobSystem::GetInstance();
	return jobSystem->QueueJob(job);
}
