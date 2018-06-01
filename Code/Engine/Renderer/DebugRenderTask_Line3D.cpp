/************************************************************************/
/* File: DebugRenderTask_Line3D.cpp
/* Author: Andrew Chase
/* Date: March 30th, 2018
/* Description: Implementation of the 3D debug line class
/************************************************************************/
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/DebugRenderTask_Line3D.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
DebugRenderTask_Line3D::DebugRenderTask_Line3D(const Vector3& startPosition, const Vector3& endPosition, const DebugRenderOptions& options, const Rgba& endStartColor, const Rgba& endEndColor, float lineWidth /*= 1.0f*/)
	: DebugRenderTask(options, DEBUG_CAMERA_WORLD)
	, m_startPosition(startPosition)
	, m_endPosition(endPosition)
	, m_lineWidth(lineWidth)
	, m_endStartColor(endStartColor)
	, m_endEndColor(endEndColor)
{
}


//-----------------------------------------------------------------------------------------------
// Renders the line to screen
//
void DebugRenderTask_Line3D::Render() const
{
	SetupDrawState(m_options.m_renderMode);

	Rgba startColor = CalculateDrawColor();

	// Get the end color
	float normalizedTime = 1.f;
	if (m_options.m_lifetime != 0.f)
	{
		normalizedTime = (m_options.m_lifetime - m_timeToLive) / m_options.m_lifetime;
	}

	Rgba endColor = Interpolate(m_endStartColor, m_endEndColor, normalizedTime);

	Renderer* renderer = Renderer::GetInstance();

	// Draw the point
	renderer->DrawLine(m_startPosition, startColor, m_endPosition, endColor, m_lineWidth);

	// Draw twice in XRAY mode
	if (m_options.m_renderMode == DEBUG_RENDER_XRAY)
	{
		SetupDrawState(DEBUG_RENDER_HIDDEN, DebugRenderSystem::DEFAULT_XRAY_COLOR_SCALE);
		startColor.ScaleRGB(0.25f);
		endColor.ScaleRGB(0.25f);

		// Second draw
		renderer->DrawLine(m_startPosition, startColor, m_endPosition, endColor, m_lineWidth);
	}
}
