#include "Engine/Rendering/Thesis/VoxelGrid.hpp"
#include "Engine/Core/EngineCommon.hpp"

void VoxelGrid::Initialize()
{
	ProfileScoped("VoxelGrid::Initialize()");
	for (int i = 0; i < VOXEL_COUNT; ++i)
	{
		voxels[i].color = Rgba::GetRandomColor();
	}
}

