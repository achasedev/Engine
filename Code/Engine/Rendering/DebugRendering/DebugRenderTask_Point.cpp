/************************************************************************/
/* File: DebugRenderTask_Point.cpp
/* Author: Andrew Chase
/* Date: March 29th, 2018
/* Description: Implementation of the DebugRenderTask_Point class
/************************************************************************/
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask_Point.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
DebugRenderTask_Point::DebugRenderTask_Point(const Vector3& position, const DebugRenderOptions& options, float radius/*=1.0f*/)
	: DebugRenderTask(options, DEBUG_CAMERA_WORLD)
	, m_position(position)
	, m_radius(radius)
{
	BuildMesh();
}


//-----------------------------------------------------------------------------------------------
//  Builds the mesh for the point draw
//
void DebugRenderTask_Point::BuildMesh()
{
	MeshBuilder mb;
	mb.BeginBuilding(PRIMITIVE_LINES, false);

	mb.PushPoint(m_position, Rgba::WHITE, m_radius);
	mb.FinishBuilding();

	m_renderable->SetMesh(mb.CreateMesh(), 0);
}


//-----------------------------------------------------------------------------------------------
// Renders the point as 7 intersecting lines
//
void DebugRenderTask_Point::Render() const
{
	SetupDrawState(m_options.m_renderMode);

	Renderer* renderer = Renderer::GetInstance();

	// Draw the point
	renderer->DrawMesh(m_renderable->GetMesh(0));

	// Draw twice in XRAY mode
	if (m_options.m_renderMode == DEBUG_RENDER_XRAY)
	{
		SetupDrawState(DEBUG_RENDER_HIDDEN, DebugRenderSystem::DEFAULT_XRAY_COLOR_SCALE);

		// Second draw
		renderer->DrawRenderable(m_renderable);
	}
}
