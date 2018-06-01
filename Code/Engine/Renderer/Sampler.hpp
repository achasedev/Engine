/************************************************************************/
/* File: Sampler.hpp
/* Author: Andrew Chase
/* Date: February 2nd, 2018
/* Description: Class to represent a GPU texture sampler
/************************************************************************/
#pragma once

class Sampler
{
public:
	//-----Public Data-----

	Sampler();
	~Sampler();

	bool Initialize();
	void Destroy();

	unsigned int GetHandle() const;

private:
	//-----Private Data-----

	unsigned int m_samplerHandle;

};