/************************************************************************/
/* File: ForwardRenderingPath.cpp
/* Author: Andrew Chase
/* Date: May 2nd, 2018
/* Description: Implementation of the ForwardRenderingPath static class
/************************************************************************/
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Rendering/Core/Camera.hpp"
#include "Engine/Rendering/Core/DrawCall.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Rendering/Resources/Skybox.hpp"
#include "Engine/Rendering/Materials/Material.hpp"
#include "Engine/Rendering/Core/ForwardRenderingPath.hpp"

//-----------------------------------------------------------------------------------------------
// Renders the given scene
//
void ForwardRenderingPath::Render(RenderScene* scene)
{
	scene->SortCameras();
	
	int numCameras = (int) scene->m_cameras.size();
	for (int index = 0; index < numCameras; ++index)
	{
		// Render shadow textures
		CreateShadowTexturesForCamera(scene, scene->m_cameras[index]);

		RenderSceneForCamera(scene->m_cameras[index], scene);
	}
}


//-----------------------------------------------------------------------------------------------
// Constructs the shadow textures to be used for shadow casting
//
void ForwardRenderingPath::CreateShadowTexturesForCamera(RenderScene* scene, Camera* camera)
{
	int numLights = (int) scene->m_lights.size();

	for (int lightIndex = 0; lightIndex < numLights; ++lightIndex)
	{
		Light* light = scene->m_lights[lightIndex];
		if (light->IsShadowCasting())
		{
			Camera* shadowCamera = new Camera();
			//Texture* color = new Texture();
			//color->CreateRenderTarget(2048, 2048, TEXTURE_FORMAT_RGBA8);

			Vector3 cameraPos = camera->GetPosition();

			shadowCamera->SetCameraMatrix(Matrix44::MakeLookAt(cameraPos - 100.f * light->GetLightData().m_lightDirection, cameraPos));
			shadowCamera->SetProjectionOrtho(200.f, 200.f, -100.f, 1000.f);

			// Set the view projection to be used for the shadow test
			LightData data = light->GetLightData();
			data.m_shadowVP = shadowCamera->GetProjectionMatrix() * shadowCamera->GetViewMatrix();
			light->SetLightData(data);

			//shadowCamera->SetColorTarget(color);
			shadowCamera->SetDepthTarget(light->GetShadowTexture());

			RenderSceneForCamera(shadowCamera, scene);

			//delete color;
			delete shadowCamera;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Constructs all the draw calls necessary for a single renderable, and adds them to the given vector
//
void ForwardRenderingPath::ConstructDrawCallsForRenderable(Renderable* renderable, RenderScene* scene, std::vector<DrawCall>& drawCalls)
{
	int drawCount = renderable->GetDrawCountPerInstance();

	for (int dcIndex = 0; dcIndex < drawCount; ++dcIndex)
	{
		DrawCall dc;

		// Compute which lights contribute the most to this renderable
		Material* material = renderable->GetMaterialForRender(dcIndex);
		if (material->IsUsingLights())
		{
			TODO("Figure out how to find the 8 lights for each instance(?)");
			ComputeLightsForDrawCall(dc, scene, renderable->GetInstancePosition(0));
		}

		bool hasModels = dc.SetDataFromRenderable(renderable, dcIndex);
	
		// Add the draw call to the list to render
		if (hasModels)
		{
			drawCalls.push_back(dc);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Sorts the draw calls given for a camera draw
//
void ForwardRenderingPath::SortDrawCalls(std::vector<DrawCall>& drawCalls)
{
	TODO("Sort alpha");
	int numDrawCalls = (int) drawCalls.size();

	bool done = false;

	while (!done)
	{
		done = true;
		for (int index = 0; index < numDrawCalls - 1; ++index)
		{
			// Find one pair out of order, swap and continue iterating
			if (drawCalls[index].GetSortOrder() > drawCalls[index + 1].GetSortOrder())
			{
				DrawCall temp = drawCalls[index];
				drawCalls[index] = drawCalls[index + 1];
				drawCalls[index + 1] = temp;

				done = false;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the given scene using the given camera
//
void ForwardRenderingPath::RenderSceneForCamera(Camera* camera, RenderScene* scene)
{
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(camera);
	renderer->ClearDepth(1.0f);

	Skybox* skybox = scene->GetSkybox();

	if (skybox != nullptr)
	{
		skybox->Render();
	}

	std::vector<DrawCall> drawCalls;

	// Create draw calls for all renderables
	int numRenderables = (int) scene->m_renderables.size();
	for (int index = 0; index < numRenderables; ++index)
	{	
		Renderable* currRenderable = scene->m_renderables[index];

		// Only construct draw calls if instances exist to draw in the renderable
		if (currRenderable->GetInstanceCount() > 0)
		{
			ConstructDrawCallsForRenderable(currRenderable, scene, drawCalls);
		}
	}

	// Sort the draw calls by their shader's layer and queue order
	SortDrawCalls(drawCalls);

	// Iterate over all draw calls and draw them
	for (int drawIndex = 0; drawIndex < (int) drawCalls.size(); ++drawIndex)
	{
		DrawCall& dc = drawCalls[drawIndex];
		renderer->Draw(dc);
	} 
}


//-----------------------------------------------------------------------------------------------
// Iterates over all lights in the scene and finds the 8 most contributing lights for the draw call,
// and stores it in the draw call
//
void ForwardRenderingPath::ComputeLightsForDrawCall(DrawCall& drawCall, RenderScene* scene, const Vector3& position)
{
	// Set the ambience
	drawCall.SetAmbience(scene->GetAmbience());

	// Calculate all other lights
	int totalLights = (int) scene->m_lights.size();

	// Calculate all intensities, store in an array parallel to the light array
	std::vector<float> intensities;
	for (int lightIndex = 0; lightIndex < totalLights; ++lightIndex)
	{
		float currIntensity = scene->m_lights[lightIndex]->CalculateIntensityForPosition(position);
		intensities.push_back(currIntensity);
	}

	// Sort the two parallel arrays by decreasing intensity
	for (int i = 0; i < totalLights - 1; ++i)
	{
		float maxValue = intensities[i];
		int maxIndex = i;

		for (int j = i + 1; j < totalLights; ++j)
		{
			if (intensities[j] > intensities[maxIndex])
			{
				maxIndex = j;
				maxValue = intensities[j];
			}
		}

		// If we find a max in the remainder of the list that isn't at the start,
		// swap it to the front
		if (maxIndex != i)
		{
			float tempValue = intensities[i];
			Light* tempLight = scene->m_lights[i];

			intensities[i] = intensities[maxIndex];
			scene->m_lights[i] = scene->m_lights[maxIndex];

			intensities[maxIndex] = tempValue;
			scene->m_lights[maxIndex] = tempLight;
		}
	}

	// Light list is sorted in decending order by intensity for this position
	// Just need to add the correct number of lights into the drawCall
	int numLightsToUse = MinInt(totalLights, MAX_NUMBER_OF_LIGHTS);

	for (int lightIndex = 0; lightIndex < numLightsToUse; ++lightIndex)
	{
		drawCall.SetLight(lightIndex, scene->m_lights[lightIndex]);
	}
	drawCall.SetNumLightsInUse(numLightsToUse);
}
