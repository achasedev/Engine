/************************************************************************/
/* File: DebugRenderTask_Skeleton.cpp
/* Author: Andrew Chase
/* Date: June 15th, 2018
/* Description: Implementation of the DebugRenderTask_Skeleton class
/************************************************************************/
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Animation/SkeletonBase.hpp"
#include "Engine/Rendering/Materials/MaterialInstance.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask_Skeleton.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
DebugRenderTask_Skeleton::DebugRenderTask_Skeleton(const SkeletonBase* skeleton, const Matrix44& transform, const DebugRenderOptions& options)
	: DebugRenderTask(options, DEBUG_CAMERA_WORLD)
{
	AssembleMesh(skeleton);
	m_renderable->AddInstanceMatrix(transform);
}


//-----------------------------------------------------------------------------------------------
// Renders the skeleton to screen
//
void DebugRenderTask_Skeleton::Render() const
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


//-----------------------------------------------------------------------------------------------
// Builds the mesh for the skeleton
//
void DebugRenderTask_Skeleton::AssembleMesh(const SkeletonBase* skeleton)
{
	MeshBuilder mb;
	mb.BeginBuilding(PRIMITIVE_LINES, false);
	Mesh* cube = AssetDB::CreateOrGetMesh("Cube");

	for (unsigned int boneIndex = 0; boneIndex < skeleton->GetBoneCount(); ++boneIndex)
	{
		int parentIndex = skeleton->GetBoneData(boneIndex).parentIndex;
		if (parentIndex != -1)
		{
			Vector3 start = Matrix44::ExtractTranslation(skeleton->GetBoneData(parentIndex).boneToMeshMatrix);
			Vector3 end = Matrix44::ExtractTranslation(skeleton->GetBoneData(boneIndex).boneToMeshMatrix);

			mb.PushLine(start, end);
		}

		RenderableDraw_t cubedraw;
		
		cubedraw.sharedMaterial = cubedraw.sharedMaterial = AssetDB::GetSharedMaterial("Debug_Render");
		cubedraw.mesh = cube;
		cubedraw.drawMatrix = skeleton->GetBoneData(boneIndex).boneToMeshMatrix;

		if (boneIndex == 0)
		{
			cubedraw.drawMatrix = cubedraw.drawMatrix * Matrix44::MakeScale(Vector3(5.f, 5.f, 5.f));
		}
		m_renderable->AddDraw(cubedraw);
	}

	mb.FinishBuilding();
	Mesh* mesh = mb.CreateMesh<Vertex3D_PCU>();

	RenderableDraw_t draw;
	draw.sharedMaterial = draw.sharedMaterial = AssetDB::GetSharedMaterial("Debug_Render");
	draw.mesh = mesh;

	m_renderable->AddDraw(draw);
}
