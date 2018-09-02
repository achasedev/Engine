#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Rendering/Thesis/Ray.hpp"
#include "Engine/Rendering/Core/Camera.hpp"
#include "Engine/Core/Time/ProfileScoped.hpp"
#include "Engine/Rendering/Thesis/RaySphere.hpp"
#include "Engine/Rendering/Thesis/HitableList.hpp"
#include "Engine/Rendering/Thesis/RayMaterial.hpp"
#include "Engine/Rendering/Thesis/RayTraceCamera.hpp"
#include "Engine/Rendering/Thesis/RayTraceRenderer.hpp"
#include "Engine/Core/Threading/Threading.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"

#include "ThirdParty/stb/stb_image_write.h"
#include <cstdlib>

#define MAX_BOUNCES 50
// Singleton instance
RayTraceRenderer* RayTraceRenderer::s_instance = nullptr;

RayTraceRenderer::RayTraceRenderer()
	: m_pixelDimensions(1920, 1080)
{
	m_colorData = (Rgba*)malloc(sizeof(Rgba) * 1920 * 1080);
}

void RayTraceRenderer::RegisterConsoleCommands()
{
}

RayTraceRenderer::~RayTraceRenderer()
{

}


float HitSphere(const Vector3& center, float radius, const Ray& r)
{
	// From ray origin to sphere center
	Vector3 oc = r.GetPosition() - center;
	float a = DotProduct(r.GetDirection(), r.GetDirection());
	float b = 2.0f * DotProduct(oc, r.GetDirection());
	float c = DotProduct(oc, oc) - radius * radius;

	float discriminant = b * b - 4 * a * c;

	// No solution (imaginary) == didn't hit the sphere
	if (discriminant < 0.f)
	{
		return -1.0f;
	}
	else
	{
		// Solve for a solution, returning the - solution, for the t close to the camera
		return (-b - Sqrt(discriminant)) / (2.f * a);
	}
}

Vector3 GetColorForRay(const Ray& r, Hitable* hitable, int depth)
{
	HitRecord_t record;
	if (hitable->Hit(r, 0.001f, 100000.f, record))
	{
		Ray scatteredRay;
		Vector3 attentuation;

		if (depth < MAX_BOUNCES && record.rayMaterial->Scatter(r, record, attentuation, scatteredRay))
		{
			Vector3 recursiveColor = GetColorForRay(scatteredRay, hitable, depth + 1);
			Vector3 finalResult;
			finalResult.x = attentuation.x * recursiveColor.x;
			finalResult.y = attentuation.y * recursiveColor.y;
			finalResult.z = attentuation.z * recursiveColor.z;

			return finalResult;
		}
		{
			return Vector3::ZERO;
		}
	}
	else
	{
		// For now just return a color between blue and white
		Vector3 unitDirection = r.GetDirection().GetNormalized();

		float blend = RangeMapFloat(unitDirection.y, -1.f, 1.0f, 0.f, 1.f);
		return Interpolate(Vector3::ONES, Vector3(0.5f, 0.7f, 1.f) , blend);
	}
}

Ray GetRayForUV(float u, float v, Camera* camera)
{
	// Set up values - everything defined in CAMERA space
	Vector3 origin = Vector3::ZERO;
	Vector3 bl = (Matrix44::GetInverse(camera->GetProjectionMatrix()) * Vector4(-1.f, -1.f, -1.f, 1.f)).xyz();
	Vector3 tr = (Matrix44::GetInverse(camera->GetProjectionMatrix()) * Vector4(1.f, 1.f, 1.f, 1.f)).xyz();

	// Make the vectors the full length of the projection plane, for uv coordinates
	Vector3 up = Vector3(0.f, tr.y - bl.y, 0.f);
	Vector3 right = Vector3(tr.x - bl.x, 0.f, 0.f);

	Vector3 direction = bl + u * right + v * up;
	Ray r(origin, direction);

	return r;
}

Hitable* GenerateRandomScene()
{
	int n = 500;
	Hitable** list = new Hitable*[n + 1];
	list[0] = new RaySphere(Vector3(0.f, -1000.f, 0.f), 1000.f, new RayMaterial_Diffuse(Vector3(0.5f)));

	// Little Spheres
	int i = 1;
	for (int a = -11; a < 11; a++)
	{
		for (int b = -11; b < 11; ++b)
		{
			float chance = GetRandomFloatZeroToOne();
			Vector3 center(a + 0.9f * GetRandomFloatZeroToOne(), 0.2f, b + 0.9f * GetRandomFloatZeroToOne());
			if ((center - Vector3(4.f, 0.2f, 0.f)).GetLength() > 0.9)
			{
				if (chance < 0.8f)
				{
					list[i++] = new RaySphere(center, 0.2f, 
						new RayMaterial_Diffuse(Vector3(
							GetRandomFloatZeroToOne() * GetRandomFloatZeroToOne(), 
							GetRandomFloatZeroToOne() * GetRandomFloatZeroToOne(), 
							GetRandomFloatZeroToOne() * GetRandomFloatZeroToOne())));
				}
				else if (chance < 0.95f)
				{
					list[i++] = new RaySphere(center, 0.2f, 
						new RayMaterial_Metal(0.5f * Vector3(
							GetRandomFloatZeroToOne() + 1.f, 
							GetRandomFloatZeroToOne() + 1.f, 
							GetRandomFloatZeroToOne() + 1.f), 
							0.5f * GetRandomFloatZeroToOne()));
				}
				else
				{
					list[i++] = new RaySphere(center, 0.2f, new RayMaterial_Dielectric(1.5f));
				}
			}
		}
	}

	// Big Spheres
	list[i++] = new RaySphere(Vector3(0.f, 1.f, 0.f), 1.f, new RayMaterial_Dielectric(1.5f));
	list[i++] = new RaySphere(Vector3(-4.f, 1.f, 0.f), 1.f, new RayMaterial_Diffuse(Vector3(0.4f, 0.2f, 0.1f)));
	list[i++] = new RaySphere(Vector3(4.f, 1.f, 0.f), 1.f, new RayMaterial_Metal(Vector3(0.7f, 0.6f, 0.5f)));

	return new HitableList(list, i);
}

struct DrawParams
{
	RayTraceCamera* camera;
	Hitable* scene;
	Rgba* colorData;
	int minY;
	int rowsToRender;
};

void ThreadWork_Draw(void* params)
{
	DrawParams* data = (DrawParams*)params;

	for (int y = data->minY; y < data->minY + data->rowsToRender; ++y)
	{
		for (int x = 0; x < 1920; ++x)
		{
			Vector3 colorValues = Vector3::ZERO;

			int numSamples = 500;
			for (int sampleNumber = 0; sampleNumber < numSamples; ++sampleNumber)
			{
				float u = ((float)x + GetRandomFloatZeroToOne()) / 1920.f;
				float v = ((float)y + GetRandomFloatZeroToOne()) / 1080.f;

				//Ray ray = GetRayForUV(u, v, camera);
				Ray ray = data->camera->GetRay(u, v);
				colorValues += GetColorForRay(ray, data->scene, 0);
			}

			colorValues /= numSamples;
			colorValues = Vector3(Sqrt(colorValues.x), Sqrt(colorValues.y), Sqrt(colorValues.z));
			Rgba finalColor(colorValues.x, colorValues.y, colorValues.z, 1.0f);

			int index = y * 1920 + x;
			data->colorData[index] = finalColor;
		}
	}

}

void RayTraceRenderer::Draw()
{
	ProfileScoped test("RayTraceRenderer::Draw"); UNUSED(test);

	// Make the camera
	Vector3 lookFrom = Vector3(3.f, 3.f, -5.f);
	Vector3 lookAt = Vector3(0.f, 1.f, 0.f);
	float focusDistance = (lookAt - lookFrom).GetLength();

	RayTraceCamera camera(lookFrom, lookAt, Vector3::DIRECTION_UP, 90.f, ((float) m_pixelDimensions.x / (float) m_pixelDimensions.y), 0.2f, focusDistance);

	// Make a few spheres
	//Hitable *hitables[4];

	// Mine
	//hitables[0] = new RaySphere(Vector3(-10.f, 0.f, 50.f), 5.f, new RayMaterial_Diffuse(Vector3(0.f, 0.f, 1.f)));
	//hitables[1] = new RaySphere(Vector3(10.f, 0.f, 50.f), 2.f, new RayMaterial_Diffuse(Vector3(1.f, 0.f, 0.f)));
	//hitables[2] = new RaySphere(Vector3(0.f, 0.f, 100.f), 20.f, new RayMaterial_Metal(Vector3(0.8f, 0.8f, 0.8f)));
	//hitables[3] = new RaySphere(Vector3(0.f, -100.f, 50.f), 84.5f, new RayMaterial_Diffuse(Vector3(0.f, 1.f, 0.f)));

	// His
	//hitables[0] = new RaySphere(Vector3(0.f, 0.f, 5.f), 0.5f, new RayMaterial_Diffuse(Vector3(0.8f, 0.3f, 0.3f)));
	//hitables[1] = new RaySphere(Vector3(0.f, -100.5f, 5.f), 100.f, new RayMaterial_Diffuse(Vector3(0.8f, 0.8f, 0.f)));
	//hitables[2] = new RaySphere(Vector3(1.f, 0.f, 5.f), 0.5f, new RayMaterial_Metal(Vector3(0.8f, 0.6f, 0.2f)));
	//hitables[3] = new RaySphere(Vector3(-1.f, 0.f, 5.f), 0.5f, new RayMaterial_Dielectric(1.5f));

	//Hitable* collection = new HitableList(hitables, 4);
	Hitable* scene = GenerateRandomScene();

	// Positions of the sphere and camera in camera space, so origin should always be (0,0,0)
	//Vector3 spherePosition = (camera->GetViewMatrix() * Vector4(0.f, 0.f, 0.f, 1.f)).xyz();

	DrawParams params[10];

	for (int i = 0; i < 10; ++i)
	{
		params[i].camera = &camera;
		params[i].colorData = m_colorData;
		params[i].scene = scene;
		params[i].minY = i * (1080 / 10);
		params[i].rowsToRender = (1080 / 10);
	}


	// Spin up threads
	ThreadHandle_t threads[10];
	for (int i = 0; i < 10; ++i)
	{
		threads[i] = Thread::Create(ThreadWork_Draw, &params[i]);
	}

	// Wait for threads to finish
	for (int i = 0; i < 10; ++i)
	{
		threads[i]->join();
		ConsolePrintf(Rgba::GREEN, "Thread %i joined", i);
	}


	// Pixels are drawn from top left to bottom right, but (0,0) is the bottom left
	//for (int y = m_pixelDimensions.y - 1; y >= 0; --y)
	//{
	//	for (int x = 0; x < m_pixelDimensions.x; ++x)
	//	{
	//		Vector3 colorValues = Vector3::ZERO;
	//
	//		int numSamples = 100;
	//		for (int sampleNumber = 0; sampleNumber < numSamples; ++sampleNumber)
	//		{
	//			float u = ((float)x + GetRandomFloatZeroToOne()) / (float)m_pixelDimensions.x;
	//			float v = ((float)y + GetRandomFloatZeroToOne()) / (float)m_pixelDimensions.y;
	//
	//			//Ray ray = GetRayForUV(u, v, camera);
	//			Ray ray = camera.GetRay(u, v);
	//			colorValues += GetColorForRay(ray, scene, 0);
	//		}
	//
	//		colorValues /= numSamples;
	//		colorValues = Vector3(Sqrt(colorValues.x), Sqrt(colorValues.y), Sqrt(colorValues.z));
	//		Rgba finalColor(colorValues.x, colorValues.y, colorValues.z, 1.0f);
	//
	//		int index = y * m_pixelDimensions.x + x;
	//		m_colorData[index] = finalColor;
	//	}
	//}
}

void RayTraceRenderer::WriteToFile(const char* filename)
{
	ProfileScoped test("RayTraceRenderer::WriteToFile"); UNUSED(test);
	stbi_flip_vertically_on_write(1);
	stbi_write_png(filename, m_pixelDimensions.x, m_pixelDimensions.y, 4, m_colorData, 0);
}

void RayTraceRenderer::Initialize()
{
	s_instance = new RayTraceRenderer();
	RegisterConsoleCommands();
}

void RayTraceRenderer::ShutDown()
{
	delete s_instance;
	s_instance = nullptr;
}

RayTraceRenderer* RayTraceRenderer::GetInstance()
{
	return s_instance;
}
