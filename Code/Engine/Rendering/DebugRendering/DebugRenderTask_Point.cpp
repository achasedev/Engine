/************************************************************************/
/* File: DebugRenderTask_Point.cpp
/* Author: Andrew Chase
/* Date: March 29th, 2018
/* Description: Implementation of the DebugRenderTask_Point class
/************************************************************************/
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Rendering/Materials/MaterialInstance.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask_Point.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
DebugRenderTask_Point::DebugRenderTask_Point(const Vector3& position, const DebugRenderOptions& options, float radius/*=1.0f*/)
	: DebugRenderTask(options, DEBUG_CAMERA_WORLD)
{
	RenderableDraw_t draw;
	draw.sharedMaterial = AssetDB::GetSharedMaterial("Debug_Render");
	draw.mesh = AssetDB::GetMesh("Point");

	m_renderable->AddDraw(draw);
	m_renderable->AddInstanceMatrix(Matrix44::MakeModelMatrix(position, Vector3::ZERO, Vector3(radius, radius, radius)));

	Material* materialInstance = m_renderable->GetMaterialInstance(0);
	materialInstance->GetEditableShader()->SetFillMode(m_options.m_isWireFrame ? FILL_MODE_WIRE : FILL_MODE_SOLID);
}


//-----------------------------------------------------------------------------------------------
// Renders the point as 7 intersecting lines
//
void DebugRenderTask_Point::Render() const
{
	SetupDrawState(m_options.m_renderMode);

	Renderer* renderer = Renderer::GetInstance();

	// Draw the point
	renderer->DrawRenderable(m_renderable);

	// Draw twice in XRAY mode
	if (m_options.m_renderMode == DEBUG_RENDER_XRAY)
	{
		SetupDrawState(DEBUG_RENDER_HIDDEN, DebugRenderSystem::DEFAULT_XRAY_COLOR_SCALE);

		// Second draw
		renderer->DrawRenderable(m_renderable);
	}
}
