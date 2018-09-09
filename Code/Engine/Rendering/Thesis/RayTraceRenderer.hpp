/************************************************************************/
/* File: RayTraceRenderer.hpp
/* Author: Andrew Chase
/* Date: September 4th, 2018
/* Description: Class to represent a ray tracing renderer interface
/************************************************************************/
#pragma once
#include "Engine/Math/IntVector2.hpp"

class Rgba;
class Camera;
class VoxelGrid;
class ComputeShader;
class RayTraceCamera;

class RayTraceRenderer
{

public:
	//-----Public Methods-----

	// Startup/Shutdown
	static void					Initialize();
	static void					ShutDown();

	// Drawing
	void						Draw(VoxelGrid* scene);

	// Accessors
	static RayTraceRenderer*	GetInstance();

	IntVector3	GetGlobalMaxItemDimensions();
	IntVector3	GetSingleGroupMaxItemDimensions();
	int			GetWorkGroupMaxItemCount();

	void OfflineTest(VoxelGrid* grid);
	
	RayTraceCamera* GetCamera();

private:
	//-----Private Methods-----

	// Singleton class, so hide constructors
	RayTraceRenderer();
	~RayTraceRenderer();
	RayTraceRenderer(const RayTraceRenderer& copy) = delete;

	static void RegisterConsoleCommands();


private:
	//-----Private Data-----

	static RayTraceRenderer* s_instance;	// Singleton instance

	Texture* m_outputTexture = nullptr;
	ComputeShader* m_computeShader = nullptr;
	RayTraceCamera* m_camera = nullptr;

	Rgba* m_testColorData = nullptr;
};
