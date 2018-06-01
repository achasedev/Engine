/************************************************************************/
/* File: DebugRenderTask_Text2D.hpp
/* Author: Andrew Chase
/* Date: April 1st, 2018
/* Description: Class to represent Debug render text in screen space
/************************************************************************/
#pragma once
#include <string>
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/DebugRenderTask.hpp"

class DebugRenderTask_Text2D : public DebugRenderTask
{
public:
	//-----Public Methods-----

	DebugRenderTask_Text2D(const std::string& text, const AABB2& bounds, const DebugRenderOptions& options, float textHeight = 50.f);

	virtual void Render() const override;


private:
	//-----Private Data-----

	std::string m_text;
	AABB2 m_pixelBounds;

	float m_textHeight;

};
