/************************************************************************/
/* File: ProfileMeasurement.hpp
/* Author: Andrew Chase
/* Date: June 30th, 2018
/* Description: Class to represent a single scoped measure of a Profile stack
/************************************************************************/
#pragma once
#include <string>
#include <vector>
#include <stdint.h>


class ProfileMeasurement
{

public:
	//-----Public Methods-----

	// Initializers
	ProfileMeasurement(const char* name);
	~ProfileMeasurement();

	// Mutators
	void Finish();

	// Producers
	uint64_t GetTotalTime_Inclusive() const;
	uint64_t GetTotalTime_Exclusive() const;


public:
	//-----Public Data-----

	std::string m_name;
	uint64_t	m_startHPC;
	uint64_t	m_endHPC;

	ProfileMeasurement* m_parent;
	std::vector<ProfileMeasurement*> m_children;

};
