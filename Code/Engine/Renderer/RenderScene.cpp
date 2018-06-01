/************************************************************************/
/* File: RenderScene.cpp
/* Author: Andrew Chase
/* Date: May 2nd, 2018
/* Description: Implementation of the RenderScene class
/************************************************************************/
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/RenderScene.hpp"


RenderScene* RenderScene::s_currentScene = nullptr;
std::map<std::string, RenderScene*> RenderScene::s_scenes;

//-----------------------------------------------------------------------------------------------
// Constructor
//
RenderScene::RenderScene(const std::string& name)
	: m_name(name)
{
}


//-----------------------------------------------------------------------------------------------
// Adds the given renderable to the list of renderables
//
void RenderScene::AddRenderable(Renderable* renderable)
{
	RemoveRenderable(renderable);
	m_renderables.push_back(renderable);
}


//-----------------------------------------------------------------------------------------------
// Adds the given light to the list of lights
//
void RenderScene::AddLight(Light* light)
{
	RemoveLight(light);
	m_lights.push_back(light);
}


//-----------------------------------------------------------------------------------------------
// Adds the given camera to the list of cameras
//
void RenderScene::AddCamera(Camera* camera)
{
	RemoveCamera(camera);
	m_cameras.push_back(camera);
}


//-----------------------------------------------------------------------------------------------
// Removes the given renderable from the list of renderables
//
void RenderScene::RemoveRenderable(Renderable* toRemove)
{
	int numRenderables = (int) m_renderables.size();
	for (int index = 0; index < numRenderables; ++index)
	{
		if (m_renderables[index] == toRemove)
		{
			m_renderables.erase(m_renderables.begin() + index);
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Removes the given light from the list of lights
//
void RenderScene::RemoveLight(Light* toRemove)
{
	int numLights = (int) m_lights.size();
	for (int index = 0; index < numLights; ++index)
	{
		if (m_lights[index] == toRemove)
		{
			m_lights.erase(m_lights.begin() + index);
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Removes the given camera from the list of cameras
//
void RenderScene::RemoveCamera(Camera* toRemove)
{
	int numCameras = (int) m_cameras.size();
	for (int index = 0; index < numCameras; ++index)
	{
		if (m_cameras[index] == toRemove)
		{
			m_cameras.erase(m_cameras.begin() + index);
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Sorts the cameras in order of their draw order, for rendering the scene through all cameras
//
void RenderScene::SortCameras()
{
	int numCameras = GetCameraCount();

	for (int first = 0; first < numCameras - 1; ++first)
	{
		int minIndex = first;
		unsigned int minOrder = m_cameras[minIndex]->GetDrawOrder();

		for (int second = first + 1; second < numCameras; ++second)
		{
			if (m_cameras[second]->GetDrawOrder() < minOrder)
			{
				minIndex = second;
				minOrder = m_cameras[minIndex]->GetDrawOrder();
			}
		}

		// A camera after first has an earlier draw order, so replace
		if (minIndex != first)
		{
			Camera* temp = m_cameras[first];
			m_cameras[first] = m_cameras[minIndex];
			m_cameras[minIndex] = temp;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the ambience of the scene
//
Rgba RenderScene::GetAmbience() const
{
	return m_ambience;
}


//-----------------------------------------------------------------------------------------------
// Returns the number of lights in the scene
//
int RenderScene::GetLightCount() const
{
	return (int) m_lights.size();
}


//-----------------------------------------------------------------------------------------------
// Returns the number of renderables in the scene
//
int RenderScene::GetRenderableCount() const
{
	return (int) m_renderables.size();
}


//-----------------------------------------------------------------------------------------------
// Returns the number of cameras in the scene
//
int RenderScene::GetCameraCount()
{
	return (int) m_cameras.size();
}


//-----------------------------------------------------------------------------------------------
// Returns the skybox of the scene if one was set, or nullptr otherwise
//
Skybox* RenderScene::GetSkybox() const
{
	return m_skybox;
}


//-----------------------------------------------------------------------------------------------
// Sets the skybox of the scene to the one provided
//
void RenderScene::SetSkybox(Skybox* skybox)
{
	m_skybox = skybox;
}


//-----------------------------------------------------------------------------------------------
// Sets the ambience of the scene to the value specified
//
void RenderScene::SetAmbience(const Rgba& ambience)
{
	m_ambience = ambience;
}
