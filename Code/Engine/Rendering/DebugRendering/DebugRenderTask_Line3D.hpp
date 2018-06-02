/************************************************************************/
/* File: DebugRenderTask_Line3D.hpp
/* Author: Andrew Chase
/* Date: March 30th, 2018
/* Description: Class to represent a debug render line in 3D
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask.hpp"

class DebugRenderTask_Line3D : public DebugRenderTask
{
public:
	//-----Public Methods-----

	DebugRenderTask_Line3D(const Vector3& startPosition, const Vector3& endPosition, const DebugRenderOptions& options, const Rgba& endStartColor, const Rgba& endEndColor, float lineWidth = 1.0f);

	virtual void Render() const override;


private:
	//-----Private Data-----

	Vector3 m_startPosition;
	Vector3 m_endPosition;

	float m_lineWidth;

	Rgba m_endStartColor;
	Rgba m_endEndColor;

};
