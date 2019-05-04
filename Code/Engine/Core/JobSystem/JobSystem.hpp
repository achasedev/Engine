/************************************************************************/
/* File: JobSystem.hpp
/* Author: Andrew Chase
/* Date: May 3rd, 2019
/* Description: Class for the multi-threaded job system
/************************************************************************/
#pragma once
#include "Engine/Core/JobSystem/JobWorkerThread.hpp"
#include <vector>
#include <shared_mutex>

enum JobStatus
{
	JOB_STATUS_QUEUED,
	JOB_STATUS_RUNNING,
	JOB_STATUS_FINISHED,
	JOB_STATUS_NOT_FOUND
};

class Job;

class JobSystem
{
	friend class JobWorkerThread;

public:
	//-----Public Methods-----

	static void			Initialize();
	static void			Shutdown();
	static JobSystem*	GetInstance();

	void				CreateWorkerThread(const char* name, WorkerThreadFlags flags);
	void				DestroyWorkerThread(const char* name);
	void				DestroyAllWorkerThreads();

	int					QueueJob(Job* job);
	void				DestroyAllJobs();

	JobStatus			GetJobStatus(int jobID);
	bool				IsJobFinished(int jobID);

	void				FinalizeAllFinishedJobs();
	void				BlockUntilJobIsFinalized(int jobID);
	void				FinishAllJobsOfType(int jobType);


private:
	//-----Private Methods-----

	// Singleton only, use Initialize()
	JobSystem();
	~JobSystem();
	JobSystem(const JobSystem& copy) = delete;


private:
	//-----Private Data-----

	std::vector<JobWorkerThread*>	m_workerThreads;
	std::shared_mutex				m_queuedLock;
	std::shared_mutex				m_runningLock;
	std::shared_mutex				m_finishedLock;
	std::vector<Job*>				m_queuedJobs;
	std::vector<Job*>				m_runningJobs;
	std::vector<Job*>				m_finishedJobs;
	int								m_nextJobID = 0;

	static JobSystem*				s_instance;

};


//////////////////////////////////////////////////////////////////////////
// C Shortcut functions
//////////////////////////////////////////////////////////////////////////

int QueueJob(Job* job);
