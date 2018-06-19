/************************************************************************/
/* File: Sampler.hpp
/* Author: Andrew Chase
/* Date: February 2nd, 2018
/* Description: Implementation of the Sampler Class
/************************************************************************/
#include "Engine/Rendering/Resources/Sampler.hpp"
#include "Engine/Rendering/OpenGL/glFunctions.hpp"

//-----------------------------------------------------------------------------------------------
// Default Constructor
//
Sampler::Sampler() 
	: m_samplerHandle(NULL)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor - deletes sampler from GPU
//
Sampler::~Sampler()
{
	Destroy();
}


//-----------------------------------------------------------------------------------------------
// Creates and sets up the sampler on the GPU
//
bool Sampler::Initialize(SamplerFilter samplerFilter, EdgeSampling edgeSampling)
{
	// create the sampler handle if needed; 
	if (m_samplerHandle == NULL) {
		glGenSamplers( 1, &m_samplerHandle ); 
		if (m_samplerHandle == NULL) {
			return false; 
		}
	}

	// setup wrapping (edge sampling)
	unsigned int glEdgeSampling = ToGLType(edgeSampling);

	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_S, glEdgeSampling );  
	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_T, glEdgeSampling );  
	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_R, glEdgeSampling );  

	GL_CHECK_ERROR();

	// Filtering

	// Min filter
	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_MIN_FILTER, ToGLType(samplerFilter));

	// Mag filter - doesn't use MipMap filters
	SamplerFilter magFilter = samplerFilter;

	// Use linear
	if (samplerFilter == SAMPLER_FILTER_LINEAR_MIPMAP_NEAREST || samplerFilter == SAMPLER_FILTER_LINEAR_MIPMAP_LINEAR)
	{
		magFilter = SAMPLER_FILTER_LINEAR;
	}

	// Use nearest
	if (samplerFilter == SAMPLER_FILTER_NEAREST_MIPMAP_LINEAR || samplerFilter == SAMPLER_FILTER_NEAREST_MIPMAP_NEAREST)
	{
		magFilter = SAMPLER_FILTER_NEAREST;
	}

	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_MAG_FILTER, ToGLType(magFilter));

	GL_CHECK_ERROR();

	return true; 
}


//-----------------------------------------------------------------------------------------------
// Deletes the sampler from the GPU
//
void Sampler::Destroy()
{
	if (m_samplerHandle != NULL) {
		glDeleteSamplers( 1, &m_samplerHandle ); 
		m_samplerHandle = NULL; 
	}
} 


//-----------------------------------------------------------------------------------------------
// Returns the GPU handle for this sampler
//
unsigned int Sampler::GetHandle() const
{
	return m_samplerHandle;
}
