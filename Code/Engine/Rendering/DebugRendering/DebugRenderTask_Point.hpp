/************************************************************************/
/* File: DebugRenderTask_Point.hpp
/* Author: Andrew Chase
/* Date: March 29th, 2018
/* Description: Class to represent a debug render point
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask.hpp"

class DebugRenderTask_Point : public DebugRenderTask
{
public:
	//-----Public Methods-----

	DebugRenderTask_Point(const Vector3& position, const DebugRenderOptions& options, float radius = 1.0f);

	virtual void Render() const override;


private:
	//-----Private Methods-----

	void BuildMesh();


private:
	//-----Private Data-----

	Vector3 m_position;
	float	m_radius;

};
