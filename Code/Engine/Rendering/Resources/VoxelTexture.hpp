#pragma once
#include "Engine/Math/IntVector3.hpp"
#include <stdint.h>

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
	uint8_t			GetCollisionByteThatContainsCoords(const IntVector3& coords) const;

	// Producers
	bool			DoLocalCoordsHaveCollision(const IntVector3& coords) const;


private:
	//-----Private Data-----

	uint8_t*	m_collisionFlags = nullptr;
	Rgba*		m_colorData = nullptr;
	IntVector3	m_dimensions;

};
