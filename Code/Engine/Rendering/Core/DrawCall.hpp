/************************************************************************/
/* File: DrawCall.hpp
/* Author: Andrew Chase
/* Date: May 2nd, 2018
/* Description: Class to represent a single GPU draw call
/************************************************************************/
#pragma once
#include <vector>
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Rendering/Core/Light.hpp"
#include "Engine/Rendering/Shaders/Shader.hpp"

class Mesh;
class Material;
class Renderable;

class DrawCall
{
public:
	//-----Public Methods-----

	// Accessors
	Mesh*			GetMesh() const;
	Material*		GetMaterial() const;
	Matrix44		GetModelMatrix(unsigned int index) const;
	const Matrix44* GetModelMatrixBuffer() const;
	int				GetModelMatrixCount() const;
	unsigned int	GetVAOHandle() const;

	int			GetSortOrder() const;
	int			GetNumLights() const;
	Light*		GetLight(unsigned int index) const;
	Rgba		GetAmbience() const;

	// Mutators
	bool SetDataFromRenderable(Renderable* renderable, int dcIndex);
	
	void SetAmbience(const Rgba& ambience);
	void SetLight(unsigned int index, Light* light);
	void SetNumLightsInUse(unsigned int numLightsInUse);


private:
	//-----Private Data-----

	Mesh*		m_mesh;
	Material*	m_material;

	std::vector<Matrix44> m_modelMatrices;

	// Lights
	Rgba m_ambience;
	int m_numLightsInUse;
	Light* m_lights[MAX_NUMBER_OF_LIGHTS];

	// For sorting in the ForwardRenderingPath
	int m_layer;
	SortingQueue m_renderQueue;

	unsigned int m_vaoHandle;

};
