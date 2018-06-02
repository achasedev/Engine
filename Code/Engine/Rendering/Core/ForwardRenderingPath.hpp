/************************************************************************/
/* File: ForwardRenderingPath.hpp
/* Author: Andrew Chase
/* Date: May 2nd, 2018
/* Description: Class used for drawing a scene in a forward rendering path
/*				Static class - cannot be instantiated
/************************************************************************/
#pragma once

class RenderScene;
class Camera;
class Renderer;

class ForwardRenderingPath
{
public:
	//-----Public Methods-----

	ForwardRenderingPath() = delete;

	static void Render(RenderScene* scene);


private:
	//-----Private Methods-----

	static void ConstructDrawCallsForRenderable(Renderable* renderable, RenderScene* scene, std::vector<DrawCall>& drawCalls);

	static void SortDrawCalls(std::vector<DrawCall>& drawCalls);
	static void RenderSceneForCamera(Camera* camera, RenderScene* scene);
	static void ComputeLightsForDrawCall(DrawCall& drawCall, RenderScene* scene, const Vector3& position);

};
