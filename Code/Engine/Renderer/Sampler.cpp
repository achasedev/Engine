/************************************************************************/
/* File: Sampler.hpp
/* Author: Andrew Chase
/* Date: February 2nd, 2018
/* Description: Implementation of the Sampler Class
/************************************************************************/
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/glFunctions.hpp"


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
bool Sampler::Initialize()
{
	// create the sampler handle if needed; 
	if (m_samplerHandle == NULL) {
		glGenSamplers( 1, &m_samplerHandle ); 
		if (m_samplerHandle == NULL) {
			return false; 
		}
	}

	// setup wrapping
	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_S, GL_REPEAT );  
	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_T, GL_REPEAT );  
	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_R, GL_REPEAT );  

	// Filtering; 
	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
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
