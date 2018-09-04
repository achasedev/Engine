#include "Engine/Rendering/Thesis/VoxelGrid.hpp"
#include "Engine/Core/EngineCommon.hpp"

void VoxelGrid::Initialize()
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

