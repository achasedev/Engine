/************************************************************************/
/* File: DebugRenderTask_Cube.cpp
/* Author: Andrew Chase
/* Date: March 31st, 2018
/* Description: Implementation of the debug render cube class
/************************************************************************/
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Materials/Material.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask_Cube.hpp"
//-----------------------------------------------------------------------------------------------
// Constructor
//
DebugRenderTask_Cube::DebugRenderTask_Cube(const Vector3& position, const DebugRenderOptions& options, const Vector3& dimensions)
	: DebugRenderTask(options, DEBUG_CAMERA_WORLD)
{
	RenderableDraw_t draw;
	draw.sharedMaterial = AssetDB::GetSharedMaterial("Debug_Render");
	draw.mesh = AssetDB::GetMesh("Cube");

	m_renderable->AddDraw(draw);
	m_renderable->AddInstanceMatrix(Matrix44::MakeModelMatrix(position, Vector3::ZERO, dimensions));

	if (!options.m_isWireFrame)
	{
		Material* material = m_renderable->GetMaterialInstance(0);
		material->SetDiffuse(AssetDB::CreateOrGetTexture("Data/Images/Debug/Debug.png"));
	}

	Material* materialInstance = m_renderable->GetMaterialInstance(0);
	materialInstance->GetEditableShader()->SetFillMode(m_options.m_isWireFrame ? FILL_MODE_WIRE : FILL_MODE_SOLID);
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
