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
public:
	//-----Public Methods-----

	virtual void	Execute() = 0;
	virtual void	JobCompleteCallback() {}
	int				GetID();


private:
	//-----Private Data-----

	int			m_jobID = -1;
	int			m_jobType = -1;
	uint32_t	m_jobChannels = 0xffffffff;

};
