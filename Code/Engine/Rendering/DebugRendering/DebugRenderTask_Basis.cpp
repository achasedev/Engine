/************************************************************************/
/* File: DebugRenderTask_Basis.cpp
/* Author: Andrew Chase
/* Date: March 30th, 2018
/* Description: Implementation of the Debug basis class
/************************************************************************/
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Materials/Material.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask_Basis.hpp"

//-----------------------------------------------------------------------------------------------
// Constructs the mesh for the basis
//
DebugRenderTask_Basis::DebugRenderTask_Basis(const Matrix44& basis, const DebugRenderOptions& options, float scale /*= 1.0f*/)
	: DebugRenderTask(options, DEBUG_CAMERA_WORLD)
	, m_basis(basis)
	, m_scale(scale)
{
	Mesh* mesh = BuildMesh();

	RenderableDraw_t draw;
	draw.sharedMaterial = AssetDB::GetSharedMaterial("Debug_Render");
	draw.mesh = mesh;

	m_renderable->AddDraw(draw);
	m_renderable->AddInstanceMatrix(Matrix44::IDENTITY);

	Material* materialInstance = m_renderable->GetMaterialInstance(0);
	materialInstance->GetEditableShader()->SetFillMode(m_options.m_isWireFrame ? FILL_MODE_WIRE : FILL_MODE_SOLID);
}


//-----------------------------------------------------------------------------------------------
// Builds the mesh for the basis
//
Mesh* DebugRenderTask_Basis::BuildMesh()
{
	// Set up the mesh vertices
	MeshBuilder mb;

	mb.BeginBuilding(PRIMITIVE_LINES, false);
	mb.SetUVs(Vector2::ZERO);
	Vector3 position = Matrix44::ExtractTranslation(m_basis);

	// I Basis
	mb.SetColor(Rgba::RED);
	mb.PushVertex(position);
	mb.PushVertex(position + m_basis.GetIVector().xyz() * m_scale);

	// J Basis
	mb.SetColor(Rgba::DARK_GREEN);
	mb.PushVertex(position);
	mb.PushVertex(position + m_basis.GetJVector().xyz() * m_scale);

	// K Basis
	mb.SetColor(Rgba::BLUE);
	mb.PushVertex(position);
	mb.PushVertex(position + m_basis.GetKVector().xyz() * m_scale);

	mb.FinishBuilding();

	m_deleteMesh = true;
	return mb.CreateMesh();
}


//-----------------------------------------------------------------------------------------------
// Draws the basis mesh to screen
//
void DebugRenderTask_Basis::Render() const
{
	Renderer* renderer = Renderer::GetInstance();

	SetupDrawState(m_options.m_renderMode);
	renderer->DrawMesh(m_renderable->GetMesh(0));

	// Draw twice in XRAY mode
	if (m_options.m_renderMode == DEBUG_RENDER_XRAY)
	{
		m_renderable->GetMaterialInstance(0)->GetEditableShader()->EnableDepth(DEPTH_TEST_GREATER, false);

		// Second draw
		renderer->DrawRenderable(m_renderable);
	}
}
