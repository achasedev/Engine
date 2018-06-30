/************************************************************************/
/* File: ProfileScoped.hpp
/* Author: Andrew Chase
/* Date: June 30th, 2018
/* Description: Class to represent a Scoped Debug printed performance profile
/************************************************************************/
#pragma once
#include <string>
#define PROFILE_LOG_SCOPE(tag) ProfileLogScoped __timer_ ##__LINE__ ## (tag)

class Stopwatch;

class ProfileScoped
{
public:
	//-----Public Methods-----

	ProfileScoped(const std::string& name);
	~ProfileScoped();


private:
	//-----Private Data-----

	std::string		m_name;
	uint64_t		m_startHPC;

};
