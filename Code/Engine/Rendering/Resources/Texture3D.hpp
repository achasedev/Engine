#pragma once
#include "Engine/Math/IntVector3.hpp"

class Rgba;

class Texture3D
{
public:
	//-----Public Methods-----

	Texture3D();
	~Texture3D();

	bool		CreateFromFile(const char* filename);
	Texture3D*	Copy() const;

	// Accessors
	Rgba*			GetColorData() const;
	Rgba			GetColorAtCoords(const IntVector3& coords) const;
	IntVector3		GetDimensions() const;
	unsigned int	GetVoxelCount() const;

private:
	//-----Private Data-----

	Rgba* m_colorData;
	IntVector3 m_dimensions;

};
