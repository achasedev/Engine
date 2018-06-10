/************************************************************************/
/* File: DebugRenderTask_UVSphere.cpp
/* Author: Andrew Chase
/* Date: March 31st, 2018
/* Description: Implementation of the Debug UV Sphere class class
/************************************************************************/
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Materials/Material.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask_UVSphere.hpp"

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
	RenderableDraw_t draw;
	draw.sharedMaterial = AssetDB::GetSharedMaterial("Debug_Render");
	draw.mesh = BuildMesh();

	m_renderable->AddDraw(draw);
	m_renderable->AddInstanceMatrix(Matrix44::IDENTITY);

	if (!options.m_isWireFrame)
	{
		Material* material = m_renderable->GetMaterialInstance(0);
		material->SetDiffuse(AssetDB::CreateOrGetTexture("Data/Images/Debug/Debug.png"));
	}

	Material* materialInstance = m_renderable->GetMaterialInstance(0);
	materialInstance->GetEditableShader()->SetFillMode(m_options.m_isWireFrame ? FILL_MODE_WIRE : FILL_MODE_SOLID);
}


//-----------------------------------------------------------------------------------------------
// Builds the UV sphere mesh for drawing
//
Mesh* DebugRenderTask_UVSphere::BuildMesh()
{
	MeshBuilder mb;
	mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);
	mb.PushUVSphere(m_position, m_radius, m_numWedges, m_numSlices);
	mb.FinishBuilding();

	m_deleteMesh = true;
	return mb.CreateMesh();
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
