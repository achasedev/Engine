/************************************************************************/
/* File: FrameBuffer.cpp
/* Author: Andrew Chase
/* Date: February 13th, 2018
/* Description: Implementation of the FrameBuffer class
/************************************************************************/
#include "Engine/Rendering/Buffers/FrameBuffer.hpp"
#include "Engine/Rendering/OpenGL/glFunctions.hpp"
#include "Engine/Rendering/Resources/Texture.hpp"


//-----------------------------------------------------------------------------------------------
// Default constructor
//
FrameBuffer::FrameBuffer()
	: m_colorTarget(nullptr)
	, m_depthTarget(nullptr)
	, m_width(0)
	, m_height(0)
{
	glGenFramebuffers( 1, &m_handle ); 
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers( 1, &m_handle ); 
}


//-----------------------------------------------------------------------------------------------
// Sets the color target to the one passed
//
void FrameBuffer::SetColorTarget(Texture* colorTarget)
{
	m_colorTarget = colorTarget;
}


//-----------------------------------------------------------------------------------------------
// Sets the depth stencil target to the one passed
//
void FrameBuffer::SetDepthTarget(Texture* depthTarget)
{
	m_depthTarget = depthTarget;
}


//-----------------------------------------------------------------------------------------------
// Returns the width of the color target (depth target should match it)
//
unsigned int FrameBuffer::GetWidth() const
{
	return m_width;
}


//-----------------------------------------------------------------------------------------------
// Returns the height of the color target (depth target should match it)
//
unsigned int FrameBuffer::GetHeight() const
{
	return m_height;
}


//-----------------------------------------------------------------------------------------------
// Returns the aspect ratio for the frame buffer
//
float FrameBuffer::GetAspect() const
{
	return (float) m_width / (float) m_height;
}


//-----------------------------------------------------------------------------------------------
// Returns the GPU handle for this buffer
//
unsigned int FrameBuffer::GetHandle() const
{
	return m_handle;
}


//-----------------------------------------------------------------------------------------------
// Binds the targets to the GPU
//
bool FrameBuffer::Finalize()
{
	GL_CHECK_ERROR();

	// Set the viewport, based on the dimensions of the targets
	// One target should be present (at least) and they should match
	if (m_colorTarget != nullptr)
	{
		IntVector2 dimensions = m_colorTarget->GetDimensions();
		glViewport(0, 0, dimensions.x, dimensions.y);
	}
	else
	{
		IntVector2 dimensions = m_depthTarget->GetDimensions();
		glViewport(0, 0, dimensions.x, dimensions.y);
	}

	GL_CHECK_ERROR();

	glBindFramebuffer( GL_FRAMEBUFFER, m_handle ); 

	GL_CHECK_ERROR();

	// keep track of which outputs go to which attachments; 
	GLenum targets[1]; 

	// Bind a color target to an attachment point
	// and keep track of which locations to to which attachments. 
	unsigned int colorHandle = (m_colorTarget == nullptr ? NULL : m_colorTarget->GetHandle());
	glFramebufferTexture( GL_FRAMEBUFFER, 
		GL_COLOR_ATTACHMENT0 + 0, 
		colorHandle, 0); 
	// 0 to to attachment 0
	targets[0] = GL_COLOR_ATTACHMENT0 + 0; 

	GL_CHECK_ERROR();

	// Update target bindings
	glDrawBuffers( 1, targets ); 
	
	GL_CHECK_ERROR();

	// Bind depth if available;
	if (m_depthTarget == nullptr) {
		glFramebufferTexture( GL_FRAMEBUFFER, 
			GL_DEPTH_STENCIL_ATTACHMENT, 
			NULL, 0); 
	} else {
		glFramebufferTexture( GL_FRAMEBUFFER, 
			GL_DEPTH_STENCIL_ATTACHMENT, 
			m_depthTarget->GetHandle(), 0); 
	}

	// Error Check - recommend only doing in debug for performance
#if defined(_DEBUG)
	GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		DebuggerPrintf( "Failed to create framebuffer:  %u", status );
		return false;
	}
#endif

	// Success (so target sizes match)
	if (m_colorTarget != nullptr)
	{
		m_width		= m_colorTarget->GetDimensions().x;
		m_height	= m_colorTarget->GetDimensions().y;
	}
	else
	{
		m_width		= m_depthTarget->GetDimensions().x;
		m_height	= m_depthTarget->GetDimensions().y;
	}

	return true;
}
