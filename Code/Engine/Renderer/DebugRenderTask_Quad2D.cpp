/************************************************************************/
/* File: DebugRenderTask_Quad2D.cpp
/* Author: Andrew Chase
/* Date: April 1st, 2018
/* Description: Implementation of the screen space debug quad class
/************************************************************************/
#include "Engine/Core/AssetDB.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/DebugRenderTask_Quad2D.hpp"
//-----------------------------------------------------------------------------------------------
// Constructor
//
DebugRenderTask_Quad2D::DebugRenderTask_Quad2D(const AABB2& bounds, const DebugRenderOptions& options)
	: DebugRenderTask(options, DEBUG_CAMERA_SCREEN)
	, m_pixelBounds(bounds)
{
	BuildMesh();

	if (!options.m_isWireFrame)
	{
		Material* material = m_renderable->GetMaterialInstance(0);
		material->SetDiffuse(AssetDB::CreateOrGetTexture("Debug.png"));
	}
}


//-----------------------------------------------------------------------------------------------
// Builds the simple quad mesh
//
void DebugRenderTask_Quad2D::BuildMesh()
{
	MeshBuilder mb;

	mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);
	mb.Push2DQuad(m_pixelBounds, AABB2::UNIT_SQUARE_OFFCENTER);
	mb.FinishBuilding();

	m_renderable->SetMesh(mb.CreateMesh(), 0);
}


//-----------------------------------------------------------------------------------------------
// Renders the quad mesh
//
void DebugRenderTask_Quad2D::Render() const
{
	SetupDrawState(m_options.m_renderMode);
	Renderer* renderer = Renderer::GetInstance();
	renderer->DrawRenderable(m_renderable);
}
