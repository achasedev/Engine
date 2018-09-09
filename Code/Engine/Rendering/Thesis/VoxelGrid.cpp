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

		float red, green, blue, alpha;
		color.GetAsFloats(red, green, blue, alpha);
		m_octree.voxels[i].color = Vector3(red, green, blue);

		// Set all flags to solid
		for (int childIndex = 0; childIndex < 8; ++childIndex)
		{
			m_octree.voxels[i].solidFlags[childIndex] = true;
		}

		if (i == 0)
		{
			ConsolePrintf("Color of first voxel is (%f,%f,%f)", red, green, blue);
		}
	}
}

void VoxelGrid::SetupForDraw()
{
	m_gpuBuffer.CopyToGPU(sizeof(OctreeStructure), &m_octree);
	//m_gpuBuffer.Bind(10);
}
