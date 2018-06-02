/************************************************************************/
/* File: DebugRenderTask_UVSphere.hpp
/* Author: Andrew Chase
/* Date: March 31st, 2018
/* Description: Class to represent a Debug render sphere
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask.hpp"


class DebugRenderTask_UVSphere : public DebugRenderTask
{
public:
	//-----Public Methods-----

	DebugRenderTask_UVSphere(const Vector3& position, const DebugRenderOptions& options,  float radius = 1.0f, unsigned int numSlices = 4, unsigned int numWedges = 8);

	virtual void Render() const override;


private:
	//-----Private Methods-----

	void BuildMesh();


private:
	//-----Private Data-----

	Vector3 m_position;

	unsigned int	m_numSlices;
	unsigned int	m_numWedges;
	float			m_radius;

};
