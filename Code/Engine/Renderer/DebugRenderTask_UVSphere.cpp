/************************************************************************/
/* File: DebugRenderTask_UVSphere.cpp
/* Author: Andrew Chase
/* Date: March 31st, 2018
/* Description: Implementation of the Debug UV Sphere class class
/************************************************************************/
#include "Engine/Core/AssetDB.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/DebugRenderTask_UVSphere.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
DebugRenderTask_UVSphere::DebugRenderTask_UVSphere(const Vector3& position, const DebugRenderOptions& options, float radius /*= 1.0f*/, unsigned int numSlices /*= 4*/, unsigned int numWedges /*= 8*/)
	: DebugRenderTask(options, DEBUG_CAMERA_WORLD)
	, m_position(position)
	, m_numSlices(numSlices)
	, m_numWedges(numWedges)
	, m_radius(radius)
{
	BuildMesh();

	if (!options.m_isWireFrame)
	{
		Material* material = m_renderable->GetMaterialInstance(0);
		material->SetDiffuse(AssetDB::CreateOrGetTexture("Debug.png"));
	}
}


//-----------------------------------------------------------------------------------------------
// Builds the UV sphere mesh for drawing
//
void DebugRenderTask_UVSphere::BuildMesh()
{
	MeshBuilder mb;
	mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);
	mb.PushUVSphere(m_position, m_radius, m_numWedges, m_numSlices);
	mb.FinishBuilding();
	m_renderable->SetMesh(mb.CreateMesh(), 0);
}


//-----------------------------------------------------------------------------------------------
// Render mesh
//
void DebugRenderTask_UVSphere::Render() const
{
	Renderer* renderer = Renderer::GetInstance();

	// Draw twice in XRAY mode
	if (m_options.m_renderMode == DEBUG_RENDER_XRAY)
	{
		SetupDrawState(DEBUG_RENDER_HIDDEN, DebugRenderSystem::DEFAULT_XRAY_COLOR_SCALE);
		renderer->DrawRenderable(m_renderable);
	}

	SetupDrawState(m_options.m_renderMode);
	renderer->DrawRenderable(m_renderable);
}
