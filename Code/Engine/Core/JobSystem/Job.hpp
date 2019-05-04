/************************************************************************/
/* File: Job.hpp
/* Author: Andrew Chase
/* Date: May 3rd, 2019
/* Description: Class for representing a single task to be completed by
/*				the JobSystem; Abstract class, must be derived from
/************************************************************************/
#pragma once
#include <stdint.h>


class Job
{
	friend class JobSystem;

public:
	//-----Public Methods-----

	Job() {}
	virtual ~Job() {}

	virtual void		Execute() = 0; // No parameters, since this is meant to be derived from, so parameters exist on the derived class
	virtual void		Finalize() {}
	inline int			GetID() const { return m_jobID; }
	inline uint32_t		GetJobFlags() const { return m_jobFlags; }


private:
	//-----Private Data-----

	int			m_jobID = -1;
	int			m_jobType = -1;
	uint32_t	m_jobFlags = 0xffffffff;

};
