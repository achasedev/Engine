#pragma once
#include <stdint.h>
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/MathUtils.hpp"

#define VOXEL_COUNT 256*256*256

struct Voxel_t
{
	//uint8_t solidFlags;
	Rgba color;

	int GetChildIndex(int index) { return (index - 1) / 8; }
	bool IsLeaf(int level) { return Pow(2, level) == 256;  }

	// Pass in index, level
};

class VoxelGrid
{
public:

	void Initialize();


private:

	IntVector3 m_dimensions;
	Voxel_t voxels[VOXEL_COUNT];
};
