#pragma once
#include <string>

#define PROFILE_LOG_SCOPE(tag) ProfileLogScoped __timer_ ##__LINE__ ## (tag)
#define PROFILE_LOG_SCOPE_FUNCTION() ProfileLogScoped __timer_ ##__LINE__ ## (__FUNCTION__ ##)

class ProfileLogScoped
{
public:
	//-----Public Methods-----

	ProfileLogScoped(const char* name);
	~ProfileLogScoped();


private:
	//-----Private Data-----

	std::string m_name;

};
