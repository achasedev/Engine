#pragma once
#include <string>

class Stopwatch;

#define PROFILE_LOG_SCOPE(tag) ProfileLogScoped __timer_ ##__LINE__ ## (tag)

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
