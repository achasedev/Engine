/************************************************************************/
/* File: Stopwatch.hpp
/* Author: Andrew Chase
/* Date: March 15th, 2018
/* Description: Class to represent a clock-based stopwatch/timer
/************************************************************************/
#pragma once
#include <stdint.h>

class Clock;

class Stopwatch
{
public:
	//-----Public Methods-----

	Stopwatch(Clock* referenceClock);

	// Mutators
	void	Reset();
	void	SetClock(Clock* clock);
	void	SetInterval(float seconds);
	void	SetElapsedTime(float secondsElapsed);
	bool	CheckAndReset();
	bool	DecrementByIntervalOnce();
	int		DecrementByIntervalAll();

	// Accessors/Producers
	float	GetElapsedTime() const;
	float	GetElapsedTimeNormalized() const;
	bool	HasIntervalElapsed() const;
	
	float	GetTotalSeconds() const;
	float	GetDeltaSeconds() const;


private:
	//-----Private Data-----

	Clock* m_referenceClock;

	uint64_t m_startHPC;
	uint64_t m_intervalHPC;

};
