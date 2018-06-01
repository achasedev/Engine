/************************************************************************/
/* File: DebugRenderTask_Cube.cpp
/* Author: Andrew Chase
/* Date: March 31st, 2018
/* Description: Implementation of the debug render cube class
/************************************************************************/
#include "Engine/Core/AssetDB.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/DebugRenderTask_Cube.hpp"
//-----------------------------------------------------------------------------------------------
// Constructor
//
DebugRenderTask_Cube::DebugRenderTask_Cube(const Vector3& position, const DebugRenderOptions& options, const Vector3& dimensions)
	: DebugRenderTask(options, DEBUG_CAMERA_WORLD)
	, m_position(position)
	, m_dimensions(dimensions)
{
	BuildMesh();

	if (!options.m_isWireFrame)
	{
		Material* material = m_renderable->GetMaterialInstance(0);
		material->SetDiffuse(AssetDB::CreateOrGetTexture("Debug.png"));
	}
}


//-----------------------------------------------------------------------------------------------
// Builds the mesh for the cube
//
void DebugRenderTask_Cube::BuildMesh()
{
	MeshBuilder mb;

	mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);
	mb.PushCube(m_position, m_dimensions);
	mb.FinishBuilding();

	m_renderable->SetMesh(mb.CreateMesh<Vertex3D_PCU>(), 0);
}


//-----------------------------------------------------------------------------------------------
// Render the mesh
//
void DebugRenderTask_Cube::Render() const
{
	Renderer* renderer = Renderer::GetInstance();

	// Draw twice in XRAY mode
	// Draw XRAY first or else it will bleed through itself
	if (m_options.m_renderMode == DEBUG_RENDER_XRAY)
	{
		SetupDrawState(DEBUG_RENDER_HIDDEN, DebugRenderSystem::DEFAULT_XRAY_COLOR_SCALE);
		renderer->DrawMesh(m_renderable->GetMesh(0));
	}

	// Main draw
	SetupDrawState(m_options.m_renderMode);
	renderer->DrawRenderable(m_renderable);
}
