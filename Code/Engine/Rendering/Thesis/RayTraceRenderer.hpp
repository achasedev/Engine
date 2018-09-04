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
class OctreeGrid;

class RayTraceRenderer
{

public:
	//-----Public Methods-----

	// Startup/Shutdown
	static void					Initialize();
	static void					ShutDown();

	// Drawing
	void						Draw(OctreeGrid* scene);
	void						WriteToFile(const char* filename);

	// Accessors
	static RayTraceRenderer*	GetInstance();


private:
	//-----Private Methods-----

	// Singleton class, so hide constructors
	RayTraceRenderer();
	~RayTraceRenderer();
	RayTraceRenderer(const RayTraceRenderer& copy) = delete;

	static void RegisterConsoleCommands();


private:
	//-----Private Data-----

	IntVector2	m_pixelDimensions;			// Pixel dimensions to render to (dimensions of the color buffer)
	Rgba*		m_colorData;				// Output "Framebuffer"

	static RayTraceRenderer* s_instance;	// Singleton instance

};
