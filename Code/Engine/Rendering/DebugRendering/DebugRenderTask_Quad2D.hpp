/************************************************************************/
/* File: DebugRenderTask_Quad2D.hpp
/* Author: Andrew Chase
/* Date: April 1st, 2018
/* Description: Class to represent a Debug render Quad in screen space
/************************************************************************/
#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask.hpp"

class DebugRenderTask_Quad2D : public DebugRenderTask
{
public:
	//-----Public Methods-----

	DebugRenderTask_Quad2D(const AABB2& bounds, const DebugRenderOptions& options);

	virtual void Render() const override;

private:
	//-----Private Methods-----

	void BuildMesh();


private:
	//-----Private Data-----

	AABB2 m_pixelBounds;

};
