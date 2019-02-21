/************************************************************************/
/* File: DebugRenderTask_Basis.hpp
/* Author: Andrew Chase
/* Date: March 30th, 2018
/* Description: Class to represent a render debug basis
/************************************************************************/
#pragma once
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask.hpp"

class Mesh;

class DebugRenderTask_Basis : public DebugRenderTask
{
public:
	//-----Public Methods-----

	DebugRenderTask_Basis(const Matrix44& basis, const DebugRenderOptions& options, float scale = 1.0f);

	virtual void Render() const override;

private:
	//-----Private Methods-----

	Mesh* BuildMesh();


private:
	//-----Private Data-----

	Matrix44	m_basis;
	float		m_scale;
};
