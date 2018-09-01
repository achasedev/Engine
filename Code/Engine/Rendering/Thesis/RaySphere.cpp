#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/Thesis/Ray.hpp"
#include "Engine/Rendering/Thesis/RaySphere.hpp"

RaySphere::RaySphere()
	: m_center(Vector3::ZERO)
	, m_radius(1.f)
{
}

RaySphere::RaySphere(const Vector3& center, float radius, RayMaterial* rayMaterial)
	: m_center(center)
	, m_radius(radius)
	, m_rayMaterial(rayMaterial)
{
}

bool RaySphere::Hit(const Ray& ray, float tMin, float tMax, HitRecord_t& out_record)
{
	// From ray origin to sphere center
	Vector3 oc = ray.GetPosition() - m_center;
	float a = DotProduct(ray.GetDirection(), ray.GetDirection());
	float b = 2.0f * DotProduct(oc, ray.GetDirection());
	float c = DotProduct(oc, oc) - m_radius * m_radius;

	float discriminant = b * b - 4 * a * c;

	// No solution (imaginary) == didn't hit the sphere
	if (discriminant > 0.f)
	{
		// Solve for a solution, returning the - solution, for the t close to the camera
		float temp = (-b - Sqrt(discriminant)) / (2.f * a);

		if (temp < tMax && temp > tMin)
		{
			out_record.t = temp;
			out_record.position = ray.GetPointAtParameter(temp);
			out_record.normal = (out_record.position - m_center) / m_radius;
			out_record.rayMaterial = m_rayMaterial;
			return true;
		}

		// Check the other solution
		temp = (-b + Sqrt(discriminant)) / (2.f * a);
		if (temp < tMax && temp > tMin)
		{
			out_record.t = temp;
			out_record.position = ray.GetPointAtParameter(temp);
			out_record.normal = (out_record.position - m_center) / m_radius;
			out_record.rayMaterial = m_rayMaterial;
			return true;
		}
	}
	return false;
}

