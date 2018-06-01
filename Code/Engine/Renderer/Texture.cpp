/************************************************************************/
/* Project: Game Engine
/* File: Texture.cpp
/* Author:
/* Date: September 28th, 2017
/* Bugs: None
/* Description: Implementation of the Texture Class
/************************************************************************/
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/glFunctions.hpp"
#include "ThirdParty/stb/stb_image.h"


// Texture Data
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// |-------------------------------------------------------------------------------------------------------| //
// |      Image Format       |    Internal Format      |        Channels         |      Pixel Layout       | //
// |-------------------------|-------------------------|-------------------------|-------------------------| //
// |     R8 (1 channel)      |          GL_R8          |         GL_RED          |     GL_UNSIGNED_BYTE    | //
// |-------------------------|-------------------------|-------------------------|-------------------------| //
// |    RG8 (2 Channel)      |         GL_RG8          |         GL_RG           |     GL_UNSIGNED_BYTE    | //
// |-------------------------|-------------------------|-------------------------|-------------------------| //
// |    RGB8 (3 Channel)     |         GL_RGB8         |         GL_RGB          |     GL_UNSIGNED_BYTE    | //
// |-------------------------|-------------------------|-------------------------|-------------------------| //
// |    RGBA8 (4 Channel)    |        GL_RGBA8         |         GL_RGBA         |     GL_UNSIGNED_BYTE    | //
// |-------------------------|-------------------------|-------------------------|-------------------------| //
// |D24S8 (Depth24/Stencil8) |   GL_DEPTH24_STENCIL8   |     GL_DEPTH_STENCIL    |   GL_UNSIGNED_INT_24_8  | //
// |-------------------------|-------------------------|-------------------------|-------------------------| //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////


//-----For converting primitive types to GL types-----

// Internal Formats
int g_openGLInternalFormats[NUM_TEXTURE_FORMATS] =
{
	GL_R8,
	GL_RG8,
	GL_RGB8,
	GL_RGBA8,
	GL_DEPTH24_STENCIL8
};

// Channels
int g_openGLChannels[NUM_TEXTURE_FORMATS] =
{
	GL_RED,
	GL_RG,
	GL_RGB,
	GL_RGBA,
	GL_DEPTH_STENCIL
};

// Pixel Layouts
int g_openGLPixelLayouts[NUM_TEXTURE_FORMATS] = 
{
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_INT_24_8
};


Texture::Texture()
	: m_textureHandle(0)
	, m_dimensions(0, 0)
	, m_textureFormat(TEXTURE_FORMAT_RGBA8)
{
}

void Texture::CreateFromFile(const std::string& filename)
{
	Image* loadedImage = new Image(filename);

	// Flip the image so it isn't upsidedown
	loadedImage->FlipVertical();

	// Construct the Texture from the image
	CreateFromImage(loadedImage);

	// Free up the image data
	delete loadedImage;
}


//-----------------------------------------------------------------------------------------------
// Loads this texture from the image provided onto the graphics card
//
void Texture::CreateFromImage(const Image* image)
{
	if (m_textureHandle == NULL)
	{
		glGenTextures(1, &m_textureHandle);
		GL_CHECK_ERROR();
	}

	m_dimensions = image->GetDimensions();
	m_textureFormat = static_cast<TextureFormat>(image->GetNumComponentsPerTexel() - 1);

	// Use texture slot 0 for the operation
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureHandle);  

	// Create the GPU-side buffer
	glTexStorage2D(GL_TEXTURE_2D,
		1,										  // Number of mipmap levels
		g_openGLInternalFormats[m_textureFormat], // How is the memory stored on the GPU
		m_dimensions.x, m_dimensions.y);		  // Dimensions

	GL_CHECK_ERROR();

	// Copy the image data to the GPU buffer that we just created
	glTexSubImage2D(GL_TEXTURE_2D,
		0,										// Mip layer we're copying to
		0, 0,									// Pixel offset
		m_dimensions.x, m_dimensions.y,			// Dimensions
		g_openGLChannels[m_textureFormat],      // Which channels exist in the CPU buffer
		g_openGLPixelLayouts[m_textureFormat],  // How are those channels stored
		image->GetImageData());					// Cpu buffer to copy

	GL_CHECK_ERROR();
}

//-----------------------------------------------------------------------------------------------
// Returns the dimensions of the texture
//
IntVector2 Texture::GetDimensions() const
{
	return m_dimensions;
}


//-----------------------------------------------------------------------------------------------
// Returns the GPU handle for this texture
//
unsigned int Texture::GetHandle() const
{
	return m_textureHandle;
}


//-----------------------------------------------------------------------------------------------
// Creates a target object on the GPU, full of garbage data, used as an intermediate render target
//
bool Texture::CreateRenderTarget(unsigned int width, unsigned int height, TextureFormat format)
{
	if (m_textureHandle == NULL)
	{
		glGenTextures(1, &m_textureHandle);
		GL_CHECK_ERROR();
	}

	// Copy the texture - first, get use to be using texture unit 0 for this;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureHandle);  
	GL_CHECK_ERROR();

	// Create the GPU-side buffer
	glTexStorage2D( GL_TEXTURE_2D,
		1,								// Number of mipmap levels
		g_openGLInternalFormats[format], // How is the memory stored on the GPU
		width, height );				// Dimensions

	// Make sure it succeeded
	GL_CHECK_ERROR(); 

	// cleanup after myself; 
	glBindTexture( GL_TEXTURE_2D, NULL );

	// Set members
	m_dimensions = IntVector2((int)width, (int)height);  
	m_textureFormat = format; 

	return true; 
}


//-----------------------------------------------------------------------------------------------
// Copies a source texture's data to a destination texture on the GPU
//
bool Texture::CopyTexture(Texture* source, Texture* destination)
{
	// We need the source and destination both to be specified
	if (source == nullptr || destination == nullptr) 
	{
		return false; 
	}

	// Get the handles
	GLuint sourceHandle			= source->GetHandle();
	GLuint destinationHandle	= destination->GetHandle(); 

	// Ensure we don't copy onto ourselves
	if (destinationHandle == sourceHandle) 
	{
		return false; 
	}

	// For now, Ensure the textures have the same dimensions
	IntVector2 sourceDimensions			= source->GetDimensions();
	IntVector2 destinationDimensions	= destination->GetDimensions();

	if (sourceDimensions != destinationDimensions)
	{
		return false;
	}

	// Copy the data over
	glCopyImageSubData(sourceHandle,		GL_TEXTURE_2D, 0, 0, 0, 0,
					   destinationHandle,	GL_TEXTURE_2D, 0, 0, 0, 0,
					   sourceDimensions.x,	sourceDimensions.y, 1);
 
	GL_CHECK_ERROR();

	return GLSucceeded();
}
