#pragma once
#include <stdint.h>
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/MathUtils.hpp"

#define VOXEL_COUNT 19173961

struct Voxel_t
{
	uint8_t solidFlags;
	Vector3 color;

	// Pass in index, level
};

class VoxelGrid
{
public:

	void Initialize();

	bool IsLeaf(int level) { return Pow(2.0f, (float)level) == 256.f; }

public:

	IntVector3 m_dimensions;
	Voxel_t voxels[VOXEL_COUNT];
};
