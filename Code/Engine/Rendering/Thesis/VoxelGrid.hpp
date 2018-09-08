/************************************************************************/
/* File: OctreeGrid.hpp
/* Author: Andrew Chase
/* Date: September 4th, 2018
/* Description: Class to represent a grid of cubes used for voxel rendering,
				structured as an Octree
/************************************************************************/
#pragma once
#include <stdint.h>
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/Buffers/ShaderStorageBuffer.hpp"

// Size of the tree to encapsulate all voxels + parents for a 256^3 grid
#define VOXEL_COUNT 19173961

// Struct to represent a single tree node
struct OctreeNode_t
{
	Vector3 color;		// The color of the voxel
	float paddingf;
};

struct OctreeStructure
{
	IntVector3		m_dimensions;
	int padding;
	OctreeNode_t	voxels[VOXEL_COUNT];
};

class VoxelGrid
{
public:
	//-----Public Methods-----
	void Initialize();

	void SetupForDraw();

private:
	//-----Private Methods-----



public:

	OctreeStructure m_octree;
	ShaderStorageBuffer m_gpuBuffer;

};
