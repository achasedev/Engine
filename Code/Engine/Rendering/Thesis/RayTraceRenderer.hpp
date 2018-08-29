#pragma once
#include "Engine/Math/IntVector2.hpp"

class Rgba;
class Camera;

class RayTraceRenderer
{

public:
	//-----Public Methods-----

	void Draw(Camera* camera);
	void WriteToFile(const char* filename);

	static void Initialize();
	static void ShutDown();

	static RayTraceRenderer* GetInstance();


private:
	//-----Private Methods-----

	RayTraceRenderer();
	~RayTraceRenderer();
	RayTraceRenderer(const RayTraceRenderer& copy) = delete;

	static void RegisterConsoleCommands();


private:
	//-----Private Data-----

	IntVector2 m_pixelDimensions;
	Rgba* m_colorData;

	static RayTraceRenderer* s_instance;

};
