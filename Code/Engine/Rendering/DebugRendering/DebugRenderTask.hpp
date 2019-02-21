/************************************************************************/
/* File: DebugRenderTask.hpp
/* Author: Andrew Chase
/* Date: March 29th, 2018
/* Description: Base class for all Debug Render primitives
/************************************************************************/
#pragma once
#include "Engine/Core/Rgba.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"

class Texture;
class Renderable;

// Enumeration for depth rendering
enum DebugRenderMode
{
	DEBUG_RENDER_IGNORE_DEPTH,
	DEBUG_RENDER_USE_DEPTH,
	DEBUG_RENDER_HIDDEN,
	DEBUG_RENDER_XRAY,
	NUM_DEBUG_MODES
};

enum DebugCamera
{
	DEBUG_CAMERA_SCREEN,
	DEBUG_CAMERA_WORLD
};


// Struct for debug render settings that all tasks have
struct DebugRenderOptions
{
	DebugRenderOptions()
		: m_startColor(Rgba::WHITE)
		, m_endColor(Rgba::WHITE)
		, m_lifetime(0.f)
		, m_renderMode(DEBUG_RENDER_USE_DEPTH)
		, m_isWireFrame(false)
		, m_customTexture(nullptr)
	{}

	void operator=(const DebugRenderOptions& copyFrom)
	{
		m_startColor	= copyFrom.m_startColor;
		m_endColor		= copyFrom.m_endColor;
		m_lifetime		= copyFrom.m_lifetime;
		m_renderMode	= copyFrom.m_renderMode;
		m_isWireFrame	= copyFrom.m_isWireFrame;
		m_customTexture	= copyFrom.m_customTexture;
	}

	Rgba m_startColor;
	Rgba m_endColor;
	float m_lifetime;
	DebugRenderMode m_renderMode;
	bool m_isWireFrame;
	Texture* m_customTexture = nullptr;
};


class DebugRenderTask
{
public:
	//-----Public Methods-----

	DebugRenderTask(const DebugRenderOptions& options, DebugCamera renderSpace);
	virtual ~DebugRenderTask();

	void			Update();
	virtual void	Render() const;

	bool IsFinished() const;	// Finished when TTL is < 0


protected:
	//-----Protected Methods-----

	void SetupDrawState(DebugRenderMode modeToDraw, float colorScale = 1.0f) const;
	Rgba CalculateDrawColor(float colorScale = 1.0f) const;


protected:
	//-----Protected Data-----

	DebugRenderOptions	m_options;
	DebugCamera			m_cameraSpace;

	Renderable*			m_renderable = nullptr;

	float m_timeToLive;
	bool m_isFinished = false;
	bool m_deleteMesh = false;
};
