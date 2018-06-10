/************************************************************************/
/* File: DebugRenderTask_Cube.hpp
/* Author: Andrew Chase
/* Date: March 31st, 2018
/* Description: Class to represent a Debug render cube (wire or solid)
/************************************************************************/
#pragma once
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask.hpp"

class DebugRenderTask_Cube : public DebugRenderTask
{
public:
	//-----Public Methods-----

	DebugRenderTask_Cube(const Vector3& position, const DebugRenderOptions& options, const Vector3& dimensions);

	virtual void Render() const override;


private:
	//-----Private Methods-----


private:
	//-----Private Data-----

	Vector3 m_position;
	Vector3 m_dimensions;
};
