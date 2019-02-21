/************************************************************************/
/* File: DebugRenderTask_Quad3D.cpp
/* Author: Andrew Chase
/* Date: March 31st, 2018
/* Description: Implementation of the debug render quad class
/************************************************************************/
#include "Engine/Math/AABB2.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Materials/Material.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask_Quad3D.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
DebugRenderTask_Quad3D::DebugRenderTask_Quad3D(const Vector3& position, const Vector2& dimensions, const DebugRenderOptions& options, const Vector3& rightVector /*= Vector3::DIRECTION_RIGHT*/, const Vector3& upVector /*= Vector3::DIRECTION_UP*/)
	: DebugRenderTask(options, DEBUG_CAMERA_WORLD)
	, m_position(position)
	, m_dimensions(dimensions)
	, m_rightVector(rightVector)
	, m_upVector(upVector)
{
	RenderableDraw_t draw;

	draw.sharedMaterial = AssetDB::GetSharedMaterial("Debug_Render");
	draw.mesh = BuildMesh();

	m_renderable->AddDraw(draw);
	m_renderable->AddInstanceMatrix(Matrix44::IDENTITY);

	if (!options.m_isWireFrame)
	{
		Material* material = m_renderable->GetMaterialInstance(0);

		if (options.m_customTexture != nullptr)
		{
			material->SetDiffuse(options.m_customTexture);
		}
		else
		{
			material->SetDiffuse(AssetDB::CreateOrGetTexture("Data/Images/Debug/Debug.png"));
		}
	}

	Material* materialInstance = m_renderable->GetMaterialInstance(0);
	materialInstance->GetEditableShader()->SetFillMode(m_options.m_isWireFrame ? FILL_MODE_WIRE : FILL_MODE_SOLID);
}


//-----------------------------------------------------------------------------------------------
// Builds the 3D quad mesh
//
Mesh* DebugRenderTask_Quad3D::BuildMesh()
{
	MeshBuilder mb;
	mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);
	mb.Push3DQuad(m_position, m_dimensions, AABB2::UNIT_SQUARE_OFFCENTER, Rgba::WHITE, m_rightVector, m_upVector);
	mb.FinishBuilding();
	
	m_deleteMesh = true;
	return mb.CreateMesh();
}


//-----------------------------------------------------------------------------------------------
// Renders the quad mesh
//
void DebugRenderTask_Quad3D::Render() const
{
	SetupDrawState(m_options.m_renderMode);

	Renderer* renderer = Renderer::GetInstance();

	renderer->DrawRenderable(m_renderable);

	// Draw twice in XRAY mode
	if (m_options.m_renderMode == DEBUG_RENDER_XRAY)
	{
		SetupDrawState(DEBUG_RENDER_HIDDEN, DebugRenderSystem::DEFAULT_XRAY_COLOR_SCALE);
		renderer->DrawRenderable(m_renderable);
	}
}
