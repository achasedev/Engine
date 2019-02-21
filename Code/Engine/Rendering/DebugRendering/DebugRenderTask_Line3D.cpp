/************************************************************************/
/* File: DebugRenderTask_Line3D.cpp
/* Author: Andrew Chase
/* Date: March 30th, 2018
/* Description: Implementation of the 3D debug line class
/************************************************************************/
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Materials/Material.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask_Line3D.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
DebugRenderTask_Line3D::DebugRenderTask_Line3D(const Vector3& startPosition, const Vector3& endPosition, const DebugRenderOptions& options, float lineWidth /*= 1.0f*/)
	: DebugRenderTask(options, DEBUG_CAMERA_WORLD)
	, m_startPosition(startPosition)
	, m_endPosition(endPosition)
	, m_lineWidth(lineWidth)
{
	Mesh* mesh = BuildMesh();

	RenderableDraw_t draw;
	draw.sharedMaterial = AssetDB::GetSharedMaterial("Debug_Render");
	draw.mesh = mesh;

	m_renderable->AddDraw(draw);
	m_renderable->AddInstanceMatrix(Matrix44::IDENTITY);

	Material* materialInstance = m_renderable->GetMaterialInstance(0);
	materialInstance->GetEditableShader()->SetFillMode(FILL_MODE_SOLID);
}


//-----------------------------------------------------------------------------------------------
// Renders the line to screen
//
void DebugRenderTask_Line3D::Render() const
{
	Renderer* renderer = Renderer::GetInstance();

	SetupDrawState(m_options.m_renderMode);

	renderer->SetGLLineWidth(m_lineWidth);

	renderer->DrawMesh(m_renderable->GetMesh(0));

	// Draw twice in XRAY mode
	if (m_options.m_renderMode == DEBUG_RENDER_XRAY)
	{
		Material* material = m_renderable->GetMaterialInstance(0);
		material->GetEditableShader()->EnableDepth(DEPTH_TEST_GREATER, false);

		// Second draw
		material->SetProperty("TINT", Vector4(0.5f, 0.5f, 0.5f, 0.8f));

		renderer->DrawRenderable(m_renderable);
	}

	renderer->SetGLLineWidth(1.0f);
}


//-----------------------------------------------------------------------------------------------
// Builds the mesh used to render the line
//
Mesh* DebugRenderTask_Line3D::BuildMesh()
{
	// Set up the mesh vertices
	MeshBuilder mb;

	mb.BeginBuilding(PRIMITIVE_LINES, false);
	mb.SetUVs(Vector2::ZERO);

	mb.SetColor(m_options.m_startColor);
	mb.PushVertex(m_startPosition);

	mb.SetColor(m_options.m_endColor);
	mb.PushVertex(m_endPosition);

	mb.FinishBuilding();

	m_deleteMesh = true;
	return mb.CreateMesh();
}
