/************************************************************************/
/* File: JobSystem.hpp
/* Author: Andrew Chase
/* Date: May 3rd, 2019
/* Description: Class for the multi-threaded job system
/************************************************************************/
#pragma once
#include "Engine/Core/JobSystem/JobWorkerThread.hpp"
#include <vector>
#include <mutex>

enum JobStatus
{
	JOB_STATUS_QUEUED,
	JOB_STATUS_RUNNING,
	JOB_STATUS_COMPLETED
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
	void				QueueJob(Job* job);

	JobStatus			GetJobStatus(int jobID);
	bool				IsJobComplete(int jobID);

	void				FinishCompletedJobs();
	void				FinishJob(int jobID);
	void				FinishAllJobsOfType(int jobType);


private:
	//-----Private Data-----

	std::vector<JobWorkerThread*>	m_workerThreads;
	std::mutex						m_queuedLock;
	std::mutex						m_runningLock;
	std::mutex						m_finishedLock;
	std::vector<Job*>				m_queuedJobs;
	std::vector<Job*>				m_runningJobs;
	std::vector<Job*>				m_finishedJobs;

	static JobSystem*				s_instance;

};
