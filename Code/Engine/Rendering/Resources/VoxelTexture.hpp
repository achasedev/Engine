#pragma once
#include "Engine/Math/IntVector3.hpp"

class Rgba;

class VoxelTexture
{
public:
	//-----Public Methods-----

	VoxelTexture();
	~VoxelTexture();

	bool		CreateFromFile(const char* filename);
	bool		CreateFromColorStream(const Rgba* colors, const IntVector3& dimensions);
	VoxelTexture* Clone() const;

	// Mutators
	void			SetColorAtCoords(const IntVector3& coords, const Rgba& color);
	void			SetColorAtIndex(unsigned int index, const Rgba& color);

	// Accessors
	Rgba			GetColorAtCoords(const IntVector3& coords) const;
	Rgba			GetColorAtIndex(unsigned int index) const;
	IntVector3		GetDimensions() const;
	unsigned int	GetVoxelCount() const;

private:
	//-----Private Data-----

	Rgba*		m_colorData;
	IntVector3	m_dimensions;

};
