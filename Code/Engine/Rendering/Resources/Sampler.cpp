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

	// Filtering
	unsigned int glFilter = ToGLType(samplerFilter);

	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_MIN_FILTER, glFilter );
	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_MAG_FILTER, glFilter );
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
