/************************************************************************/
/* File: RenderScene.hpp
/* Author: Andrew Chase
/* Date: May 2nd, 2018
/* Description: Class to represent a collection of objects used for rendering
/*				(Lights, renderables, cameras used for ForwardRenderingPath)
/************************************************************************/
#pragma once
#include <map>
#include <vector>
#include "Engine/Core/Rgba.hpp"

class Renderable;
class Light;
class Camera;
class Skybox;

class RenderScene
{

public:
	//-----Public Methods-----

	// To allow direct access to the lists for rendering
	friend class ForwardRenderingPath;

	RenderScene(const std::string& name);
	~RenderScene() {}
	RenderScene(const RenderScene& copy) = delete;

	// List mutators
	void AddRenderable(Renderable* renderable);
	void AddLight(Light* light);
	void AddCamera(Camera* camera);

	void RemoveRenderable(Renderable* toRemove);
	void RemoveLight(Light* toRemove);
	void RemoveCamera(Camera* toRemove);
	void RemoveAll();

	void SetSkybox(Skybox* skybox);
	void SetAmbience(const Rgba& ambience);

	void SortCameras();

	Rgba GetAmbience() const;

	// List accessors
	int GetLightCount() const;
	int GetRenderableCount() const;
	int GetCameraCount();

	Skybox* GetSkybox() const;


private:
	//-----Private Data-----

	std::string m_name;
	std::vector<Renderable*>	m_renderables;
	std::vector<Light*>			m_lights;
	std::vector<Camera*>		m_cameras;

	Rgba m_ambience;

	Skybox* m_skybox = nullptr;

};
