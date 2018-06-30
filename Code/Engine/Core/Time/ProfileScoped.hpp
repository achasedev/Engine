#pragma once
#include <string>

class Stopwatch;

class ProfileScoped
{
public:
	//-----Public Methods-----

	ProfileScoped(const std::string& name);
	~ProfileScoped();


private:
	//-----Private Data-----

	std::string m_name;
	
	uint64_t m_startHPC;

};