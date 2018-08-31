#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Rendering/Thesis/Ray.hpp"
#include "Engine/Rendering/Core/Camera.hpp"
#include "Engine/Core/Time/ProfileScoped.hpp"
#include "Engine/Rendering/Thesis/RaySphere.hpp"
#include "Engine/Rendering/Thesis/HitableList.hpp"
#include "Engine/Rendering/Thesis/RayTraceRenderer.hpp"
#include "ThirdParty/stb/stb_image_write.h"
#include <cstdlib>

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

Vector3 GetColorForRay(const Ray& r, Hitable* hitable)
{
	HitRecord_t record;
	if (hitable->Hit(&r, 0.f, 1000.f, record))
	{
		return 0.5f * (record.normal + Vector3::ONES);
	}
	else
	{
		// For now just return a color between blue and white
		Vector3 unitDirection = r.GetDirection().GetNormalized();

		float blend = RangeMapFloat(unitDirection.y, -1.f, 1.0f, 0.f, 1.f);
		return Interpolate(Vector3::ONES, Vector3(0.5f, 0.7f, 1.0f), blend);
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


void RayTraceRenderer::Draw(Camera* camera)
{
	UNUSED(camera);
	ProfileScoped test("RayTraceRenderer::Draw"); UNUSED(test);

	// Make a few spheres
	Hitable *hitables[3];

	hitables[0] = new RaySphere(Vector3(-10.f, 0.f, 50.f), 5.f);
	hitables[1] = new RaySphere(Vector3(10.f, 0.f, 50.f), 2.f);
	hitables[2] = new RaySphere(Vector3(0.f, 0.f, 100.f), 20.f);

	Hitable* collection = new HitableList(hitables, 3);

	// Positions of the sphere and camera in camera space, so origin should always be (0,0,0)
	//Vector3 spherePosition = (camera->GetViewMatrix() * Vector4(0.f, 0.f, 0.f, 1.f)).xyz();

	// Pixels are drawn from top left to bottom right, but (0,0) is the bottom left
	for (int y = m_pixelDimensions.y - 1; y >= 0; --y)
	{
		for (int x = 0; x < m_pixelDimensions.x; ++x)
		{
			Vector3 colorValues = Vector3::ZERO;

			int numSamples = 1;
			for (int sampleNumber = 0; sampleNumber < numSamples; ++sampleNumber)
			{
				float u = ((float)x + GetRandomFloatZeroToOne()) / (float)m_pixelDimensions.x;
				float v = ((float)y + GetRandomFloatZeroToOne()) / (float)m_pixelDimensions.y;

				Ray ray = GetRayForUV(u, v, camera);
				colorValues += GetColorForRay(ray, collection);
			}

			colorValues /= numSamples;
			Rgba finalColor(colorValues.x, colorValues.y, colorValues.z, 1.0f);

			int index = y * m_pixelDimensions.x + x;
			m_colorData[index] = finalColor;
		}
	}
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
