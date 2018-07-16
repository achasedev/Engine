/************************************************************************/
/* File: Time.hpp
/* Author: Andrew Chase
/* Date: March 2nd, 2018
/* Description: TimeSystem and LogProfileScope classes
/************************************************************************/
#pragma once
#include <stdint.h>
#include <string>

class TimeSystem
{
public:
	//-----Public Methods-----

	static double	PerformanceCountToSeconds(const uint64_t hpc);
	static uint64_t SecondsToPerformanceCount(double seconds);


private:
	//-----Private Methods-----

	TimeSystem();
	~TimeSystem();
	TimeSystem(const TimeSystem& copy) = delete;


private:
	//-----Private Data-----

	uint64_t m_frequency;
	double m_secondsPerCount;

	static TimeSystem s_timeSystem;
};


class LogProfileScope
{
public:
	//-----Public Methods-----

	LogProfileScope(const char* tag);
	~LogProfileScope();


private:
	//-----Private Data-----

	const char* m_tag;
	uint64_t m_startHPC;

};

//--------------------------------C FUNCTIONS-------------------------------------

uint64_t	GetPerformanceCounter();
std::string GetFormattedSystemDateAndTime();
std::string GetFormattedSystemTime();