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

// Size of the tree to encapsulate all voxels + parents for a 256^3 grid
#define VOXEL_COUNT 19173961

// Struct to represent a single tree node
struct OctreeNode_t
{
	uint8_t solidFlags;	// Flags if any children are non-empty
	Vector3 color;		// The color of the voxel
};


class OctreeGrid
{
public:
	//-----Public Methods-----
	void Initialize();
	bool IsLeaf(int level) { return Pow(2.0f, (float)level) == (float)m_dimensions.x; }

public:

	IntVector3		m_dimensions;
	OctreeNode_t	voxels[VOXEL_COUNT];

};
