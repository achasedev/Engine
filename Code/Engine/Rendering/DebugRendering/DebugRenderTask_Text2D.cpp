/************************************************************************/
/* File: DebugRenderTask_Text2D.cpp
/* Author: Andrew Chase
/* Date: April 1st, 2018
/* Description: Implementation of the 2D debug text class
/************************************************************************/
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Materials/Material.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask_Text2D.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
DebugRenderTask_Text2D::DebugRenderTask_Text2D(const std::string& text, const AABB2& bounds, const DebugRenderOptions& options, float textHeight /*= 50.f*/)
	: DebugRenderTask(options, DEBUG_CAMERA_SCREEN)
	, m_text(text)
	, m_pixelBounds(bounds)
	, m_textHeight(textHeight)
{
	delete m_renderable;
	m_renderable = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Renders the text
//
void DebugRenderTask_Text2D::Render() const
{
	Rgba drawColor = CalculateDrawColor();

	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(DebugRenderSystem::GetScreenCamera());
	BitmapFont* defaultFont = AssetDB::CreateOrGetBitmapFont("Data/Images/Fonts/Default.png");
	renderer->DrawTextInBox2D(m_text, m_pixelBounds, Vector2(0.f, 0.f), m_textHeight, TEXT_DRAW_OVERRUN, defaultFont, drawColor);
}
