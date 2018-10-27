#pragma once
#include "Engine/Math/IntVector3.hpp"
#include <stdint.h>

#define MAX_TEXTURE_VOXEL_WIDTH (32)
#define MAX_TEXTURE_BYTE_WIDTH (MAX_TEXTURE_VOXEL_WIDTH / 8)
class Rgba;

class VoxelTexture
{
public:
	//-----Public Methods-----

	VoxelTexture();
	~VoxelTexture();

	bool			CreateFromFile(const char* filename);
	bool			CreateFromColorStream(const Rgba* colors, const IntVector3& dimensions);
	VoxelTexture*	Clone() const;

	// Mutators
	void			SetColorAtCoords(const IntVector3& coords, const Rgba& color);
	void			SetColorAtIndex(unsigned int index, const Rgba& color);

	// Accessors
	Rgba			GetColorAtCoords(const IntVector3& coords) const;
	Rgba			GetColorAtIndex(unsigned int index) const;
	IntVector3		GetDimensions() const;
	unsigned int	GetVoxelCount() const;
	uint32_t		GetCollisionByteForRow(int localY, int localZ) const;

	// Producers
	bool			DoLocalCoordsHaveCollision(const IntVector3& coords) const;


private:
	//-----Private Methods-----

	bool AreCoordsValid(int x, int y, int z) const;


private:
	//-----Private Data-----

	uint32_t*	m_collisionFlags = nullptr;
	Rgba*		m_colorData = nullptr;
	IntVector3	m_dimensions;

};
