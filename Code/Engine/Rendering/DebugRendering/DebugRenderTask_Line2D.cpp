/************************************************************************/
/* File: DebugRenderTask_Line2D.hpp
/* Author: Andrew Chase
/* Date: April 1st, 2018
/* Description: Class to represent a debug render line in 2D
/************************************************************************/
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Materials/Material.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask_Line2D.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
DebugRenderTask_Line2D::DebugRenderTask_Line2D(const Vector2& startPosition, const Vector2& endPosition, const DebugRenderOptions& options, const Rgba& endStartColor, const Rgba& endEndColor, float lineWidth /*= 1.0f*/)
	: DebugRenderTask(options, DEBUG_CAMERA_SCREEN)
	, m_startPosition(Vector3(startPosition.x, startPosition.y, 0.f))
	, m_endPosition(Vector3(endPosition.x, endPosition.y, 0.f))
	, m_endStartColor(endStartColor)
	, m_endEndColor(endEndColor)
	, m_lineWidth(lineWidth)
{
}


//-----------------------------------------------------------------------------------------------
// Renders the line to screen
//
void DebugRenderTask_Line2D::Render() const
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

	// Draw the line (no xray draw)
	renderer->DrawLine(m_startPosition, startColor, m_endPosition, endColor, m_lineWidth);
}
