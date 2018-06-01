/************************************************************************/
/* File: DebugRenderTask.cpp
/* Author: Andrew Chase
/* Date: March 29th, 2018
/* Description: Implementation for the DebugRenderTask base class
/************************************************************************/
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRenderTask.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
DebugRenderTask::DebugRenderTask(const DebugRenderOptions& options, DebugCamera renderSpace)
{
	m_options = options;
	m_timeToLive = options.m_lifetime;
	m_isFinished = false;
	m_cameraSpace = renderSpace;

	// Set up the renderable
	Material* material = AssetDB::CreateOrGetSharedMaterial("Debug_Render");
	m_renderable = new Renderable(Matrix44::IDENTITY, (Mesh*)nullptr, material);	// Set up the mesh in the subclasses, no transform needed

	// Set the fill mode, depth will be set during draws
	Material* materialInstance = m_renderable->GetMaterialInstance(0);
	materialInstance->GetEditableShader()->SetFillMode(m_options.m_isWireFrame ? FILL_MODE_WIRE : FILL_MODE_SOLID);
}


//-----------------------------------------------------------------------------------------------
// Virtual destructor
//
DebugRenderTask::~DebugRenderTask()
{
	// Renderable destructor doesn't delete meshes, so do it here
	Mesh* mesh = m_renderable->GetMesh(0);
	if (mesh != nullptr)
	{
		delete mesh;
	}

	delete m_renderable;
}


//-----------------------------------------------------------------------------------------------
// Update - decrements TTL and checks if finished (TTL < 0)
//
void DebugRenderTask::Update()
{
	float deltaTime = Clock::GetMasterDeltaTime();
	m_timeToLive -= deltaTime;

	if (m_timeToLive < 0.f)
	{
		m_isFinished = true;
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the task
//
void DebugRenderTask::Render() const
{
}


//-----------------------------------------------------------------------------------------------
// Returns true if the task is finished (TTL < 0)
//
bool DebugRenderTask::IsFinished() const
{
	return m_isFinished;
}


//-----------------------------------------------------------------------------------------------
// Sets up the render state for the draw call
//
void DebugRenderTask::SetupDrawState(DebugRenderMode modeToDraw, float colorScale /*= 1.0f*/) const
{
	Material* materialToUse = m_renderable->GetMaterialInstance(0);
	Shader* shaderToUse = materialToUse->GetEditableShader();

	Rgba drawColor = CalculateDrawColor(colorScale);

	//-----Setup the depth-----
	switch (modeToDraw)
	{
	case DEBUG_RENDER_HIDDEN:
		shaderToUse->EnableDepth(DEPTH_TEST_GREATER, true);
		break;
	case DEBUG_RENDER_USE_DEPTH:
		shaderToUse->EnableDepth(DEPTH_TEST_LESS, true);
		break;
	case DEBUG_RENDER_IGNORE_DEPTH:
		shaderToUse->DisableDepth();
		break;
	case DEBUG_RENDER_XRAY:
		shaderToUse->EnableDepth(DEPTH_TEST_LESS, true);	// This is for the "normal" draw of the xray draw
		break;
	default:
		break;
	}

	// Set up the camera to use
	Renderer* renderer = Renderer::GetInstance();
	switch (m_cameraSpace)
	{
	case DEBUG_CAMERA_SCREEN:
		renderer->SetCurrentCamera(DebugRenderSystem::GetScreenCamera());
		break;
	case DEBUG_CAMERA_WORLD:
		renderer->SetCurrentCamera(DebugRenderSystem::GetWorldCamera());
		break;
	default:
		break;
	}

	// Set the tint
	float red, green, blue, alpha;
	drawColor.GetAsFloats(red, green, blue, alpha);
	materialToUse->SetProperty("TINT", Vector4(red, green, blue, alpha));
}


//-----------------------------------------------------------------------------------------------
// Finds the draw color and returns it
//
Rgba DebugRenderTask::CalculateDrawColor(float scale /*= 1.0f*/) const
{
	// Determine draw color
	float normalizedTime = 1.f;
	if (m_options.m_lifetime != 0.f)
	{
		normalizedTime = (m_options.m_lifetime - m_timeToLive) / m_options.m_lifetime;
	}

	Rgba drawColor = Interpolate(m_options.m_startColor, m_options.m_endColor, normalizedTime);
	drawColor.ScaleRGB(scale);

	return drawColor;
}
