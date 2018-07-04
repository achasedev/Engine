/************************************************************************/
/* File: Gif.cpp
/* Author: Andrew Chase
/* Date: July 4th, 2018
/* Description: Implementation of the Gif class
/************************************************************************/
#include "Engine/Core/Gif.hpp"
#include "Engine/Core/File.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Rendering/Resources/Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image.h"


//-----------------------------------------------------------------------------------------------
// Constructor
//
Gif::Gif()
	: m_gifData(nullptr)
	, m_currFrameIndex(0)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Gif::~Gif()
{
	delete m_stopwatch;
	m_stopwatch = nullptr;

	free((void*)m_gifData);
	m_gifData = nullptr;
	
	for (int index = 0; index < (int) m_frameTextures.size(); ++index)
	{
		delete m_frameTextures[index];
	}

	m_frameTextures.clear();
}


//-----------------------------------------------------------------------------------------------
// Loads the gif from the file given by filepath, and constructs all textures for the gif
//
bool Gif::LoadFromFile(const std::string& filepath)
{
	// Load the data in a raw buffer first
	size_t size;
	void* data = FileReadToNewBuffer(filepath.c_str(), size);
	
	if (data == nullptr)
	{
		return false;
	}

	// Convert the raw data to a sequential order of frames (RGBA)
	// We flip vertically so they are oriented correctly for textures

	int* delays;
	stbi_set_flip_vertically_on_load(1);
	m_gifData = (unsigned char*) stbi_load_gif_from_memory((const stbi_uc*)data, (int) size, &delays, &m_frameDimensions.x, &m_frameDimensions.y, &m_numFrames, &m_numComponentsPerTexel, 0);
	stbi_set_flip_vertically_on_load(0);

	if (m_gifData == nullptr)
	{
		return false;
	}

	// Make the textures from the image data
	for (int frameIndex = 0; frameIndex < m_numFrames - 1; ++frameIndex)
	{
		int imageOffset = (m_frameDimensions.x * m_frameDimensions.y * m_numComponentsPerTexel) * frameIndex;
		unsigned char* imageStart = &m_gifData[imageOffset];

		Texture* texture = new Texture();
		texture->CreateFromRawData(m_frameDimensions, m_numComponentsPerTexel, imageStart, false);

		m_frameTextures.push_back(texture);
	}


	// For now just play the gif at the rate given by the first frame's delay
	m_stopwatch = new Stopwatch(nullptr);
	m_stopwatch->SetInterval((float) delays[0] / 1000.f);

	// Return success
	return true;
}


//-----------------------------------------------------------------------------------------------
// Returns the frame dimensions of this gif
//
IntVector2 Gif::GetDimensions() const
{
	return m_frameDimensions;
}


//-----------------------------------------------------------------------------------------------
// Returns the next frame to play of the gif, using its internal stopwatch for timing
//
Texture* Gif::GetNextFrame()
{
	m_currFrameIndex += (int) m_stopwatch->DecrementByIntervalAll();

	if (m_currFrameIndex > (unsigned int) m_frameTextures.size() - 1)
	{
		m_currFrameIndex = 0;
	}

	return m_frameTextures[m_currFrameIndex];
}
