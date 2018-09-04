/************************************************************************/
/* File: OctreeGrid.cpp
/* Author: Andrew Chase
/* Date: September 4th, 2018
/* Description: Implementation of the OctreeGrid class
/************************************************************************/
#include "Engine/Rendering/Thesis/OctreeGrid.hpp"
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
// Initializes the tree to hold a grid of random colors
//
void OctreeGrid::Initialize()
{
	m_dimensions = IntVector3(256, 256, 256);
	ProfileScoped("VoxelGrid::Initialize()");
	for (int i = 0; i < VOXEL_COUNT; ++i)
	{
		Rgba color = Rgba::GetRandomColor();

		float red, green, blue, alpha;
		color.GetAsFloats(red, green, blue, alpha);
		voxels[i].color = Vector3(red, green, blue);
		voxels[i].solidFlags = 0xFF;
	}
}
