/************************************************************************/
/* File: DebugRenderTask_Line2D.hpp
/* Author: Andrew Chase
/* Date: April 1st, 2018
/* Description: Class to represent a debug render line in 2D
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask.hpp"

class DebugRenderTask_Line2D : public DebugRenderTask
{
public:
	//-----Public Methods-----

	DebugRenderTask_Line2D(const Vector2& startPosition, const Vector2& endPosition, const DebugRenderOptions& options, const Rgba& endStartColor, const Rgba& endEndColor, float lineWidth = 1.0f);

	virtual void Render() const override;


private:
	//-----Private Data-----

	// Vector3's used for Renderer::DrawLine()
	Vector3 m_startPosition;
	Vector3 m_endPosition;

	float m_lineWidth;

	Rgba m_endStartColor;
	Rgba m_endEndColor;

};
