#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/Thesis/Ray.hpp"
#include "Engine/Rendering/Thesis/Hitable.hpp"
#include "Engine/Rendering/Thesis/RayMaterial.hpp"

RayMaterial_Diffuse::RayMaterial_Diffuse(const Vector3& albedo)
	: m_albedoColor(albedo)
{
}


bool RayMaterial_Diffuse::Scatter(const Ray& incomingRay, const HitRecord_t& record, Vector3& out_attentuation, Ray& out_scatteredRay)
{
	Vector3 target = record.position + record.normal + GetRandomPointWithinSphere();
	out_scatteredRay = Ray(record.position, target - record.position);
	out_attentuation = m_albedoColor;
	return true;
}

RayMaterial_Metal::RayMaterial_Metal(const Vector3& albedo, float fuzziness /*= 0.f*/)
	: m_albedoColor(albedo)
	, m_fuzziness(fuzziness)
{
}

bool RayMaterial_Metal::Scatter(const Ray& incomingRay, const HitRecord_t& record, Vector3& out_attentuation, Ray& out_scatteredRay)
{
	Vector3 reflectedDirection = Reflect(incomingRay.GetDirection().GetNormalized(), record.normal);
	out_scatteredRay = Ray(record.position, reflectedDirection + m_fuzziness * GetRandomPointWithinSphere());
	out_attentuation = m_albedoColor;
	return (DotProduct(reflectedDirection, record.normal) > 0);
}

RayMaterial_Dielectric::RayMaterial_Dielectric(float indexOfRefraction)
	: m_indexOfRefraction(indexOfRefraction)
{
}

bool RayMaterial_Dielectric::Scatter(const Ray& incomingRay, const HitRecord_t& record, Vector3& out_attentuation, Ray& out_scatteredRay)
{
	Vector3 outwardNormal;
	Vector3 reflected = Reflect(incomingRay.GetDirection(), record.normal);

	float niOverNt;
	out_attentuation = Vector3(1.f, 1.f, 1.f);
	Vector3 refracted;

	if (DotProduct(incomingRay.GetDirection(), record.normal) > 0)
	{
		outwardNormal = -1.f * record.normal;
		niOverNt = m_indexOfRefraction;
	}
	else
	{
		outwardNormal = record.normal;
		niOverNt = 1.0f / m_indexOfRefraction;
	}

	// Check for reflect or refract
	if (Refract(incomingRay.GetDirection(), outwardNormal, niOverNt, refracted))
	{
		out_scatteredRay = Ray(record.position, refracted);
	}
	else
	{
		out_scatteredRay = Ray(record.position, reflected);
	}

	return true;
}
