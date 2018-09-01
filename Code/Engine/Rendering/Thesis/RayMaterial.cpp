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
	Vector3 target = record.position + record.normal + GetRandomPointOnSphere();
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
	out_scatteredRay = Ray(record.position, reflectedDirection + m_fuzziness * GetRandomPointOnSphere());
	out_attentuation = m_albedoColor;
	return (DotProduct(reflectedDirection, record.normal) > 0);
}
