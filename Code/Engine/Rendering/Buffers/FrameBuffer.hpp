/************************************************************************/
/* File: FrameBuffer.hpp
/* Author: Andrew Chase
/* Date: February 13th, 2018
/* Description: Class to represent a color/depth render target
/************************************************************************/
#pragma once

class Texture;

class FrameBuffer
{
public:
	//-----Public Methods-----

	FrameBuffer(); 
	~FrameBuffer();

	void SetColorTarget(Texture* color_target); 
	void SetDepthTarget(Texture* depth_target); 

	unsigned int GetWidth() const;
	unsigned int GetHeight() const;
	unsigned int GetHandle() const;

	bool Finalize(); 


public:
	//-----Private Data-----

	unsigned int	m_handle; 
	Texture*		m_colorTarget; 
	Texture*		m_depthTarget;

	unsigned int	m_width;
	unsigned int	m_height;
};