/************************************************************************/
/* File: Gif.hpp
/* Author: Andrew Chase
/* Date: July 4th, 2018
/* Description: Class to represent an image loaded from a gif format
/************************************************************************/
#pragma once
#include <vector>
#include "Engine/Core/Image.hpp"

class Texture;
class Stopwatch;

class Gif
{
public:
	//-----Public Methods-----

	// Initialization/Destruction
	Gif();
	~Gif();
	bool LoadFromFile(const std::string& filepath);

	// Accessors
	IntVector2	GetDimensions() const;
	Texture*	GetNextFrame();


private:
	//-----Private Data-----

	// General image data
	int m_numFrames;
	int m_numComponentsPerTexel;
	IntVector2 m_frameDimensions;
	unsigned char* m_gifData;

	// For playback
	Stopwatch* m_stopwatch;
	unsigned int m_currFrameIndex;
	std::vector<Texture*> m_frameTextures;

};