/************************************************************************/
/* File: DebugRenderTask_Quad3D.hpp
/* Author: Andrew Chase
/* Date: March 31st, 2018
/* Description: Class to represent a Debug render Quad in 3D
/************************************************************************/
#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask.hpp"

class DebugRenderTask_Quad3D : public DebugRenderTask
{
public:
	//-----Public Methods-----

	DebugRenderTask_Quad3D(const Vector3& position, const Vector2& dimensions, const DebugRenderOptions& options, const Vector3& rightVector = Vector3::DIRECTION_RIGHT, const Vector3& upVector = Vector3::DIRECTION_UP);

	virtual void Render() const override;


private:
	//-----Private Methods-----

	Mesh* BuildMesh();


private:
	//-----Private Data-----

	Vector3 m_position;
	Vector2 m_dimensions;

	Vector3 m_rightVector;
	Vector3 m_upVector;

};
