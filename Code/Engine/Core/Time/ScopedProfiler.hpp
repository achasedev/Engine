#pragma once
#include <string>

class Stopwatch;

class ScopedProfiler
{
public:
	//-----Public Methods-----

	ScopedProfiler(const std::string& name);
	~ScopedProfiler();


private:
	//-----Private Data-----

	std::string m_name;
	
	uint64_t m_startHPC;

};