/************************************************************************/
/* File: Sampler.hpp
/* Author: Andrew Chase
/* Date: February 2nd, 2018
/* Description: Class to represent a GPU texture sampler
/************************************************************************/
#pragma once
#include "Engine/Rendering/OpenGL/glTypes.hpp"

class Sampler
{
public:
	//-----Public Data-----

	Sampler();
	~Sampler();

	bool Initialize(SamplerFilter filter, EdgeSampling edgeSampling);
	void Destroy();

	unsigned int GetHandle() const;

private:
	//-----Private Data-----

	unsigned int m_samplerHandle;

};