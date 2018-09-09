/************************************************************************/
/* File: OctreeGrid.cpp
/* Author: Andrew Chase
/* Date: September 4th, 2018
/* Description: Implementation of the OctreeGrid class
/************************************************************************/
#include "Engine/Rendering/Thesis/VoxelGrid.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"

//-----------------------------------------------------------------------------------------------
// Initializes the tree to hold a grid of random colors
//
void VoxelGrid::Initialize()
{
	m_octree.m_dimensions = IntVector3(256, 256, 256);
	ProfileScoped("VoxelGrid::Initialize()");
	for (int i = 0; i < VOXEL_COUNT; ++i)
	{
		Rgba color = Rgba::GetRandomColor();
 		//Rgba color = Rgba::RED;
// 
// 		if (i % 2 == 0)
// 		{
// 			color = Rgba::BLUE;
// 		}

		float red, green, blue, alpha;
		color.GetAsFloats(red, green, blue, alpha);
		m_octree.voxels[i].color = Vector3(red, green, blue);
	}

	m_octree.voxels[0].flags = 1;
	// Set some to not be rendered
	ZeroOut(1, 2);
	ZeroOut(1, 3);
	ZeroOut(1, 4);

	ZeroOut(1, 5);
	ZeroOut(1, 6);
	ZeroOut(1, 7);
	ZeroOut(1, 8);
}

void VoxelGrid::SetupForDraw()
{
	static bool test = true;
	if (test)
	{
		m_gpuBuffer.Bind(10);
		size_t size = sizeof(OctreeStructure);
		m_gpuBuffer.CopyToGPU(sizeof(OctreeStructure), &m_octree);
		test = false;
	}
}

void VoxelGrid::ZeroOut(int level, int parentIndex)
{
	m_octree.voxels[parentIndex].flags = 0;

	if (level >= 8)
	{
		return;
	}

	for (int childOffset = 0; childOffset < 8; ++childOffset)
	{
		int childIndex = 8 * parentIndex + 1 + childOffset;
		ZeroOut(level + 1, childIndex);
	}
}
