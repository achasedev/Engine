#pragma once
#include "Engine/Math/Vector3.hpp"

class Ray;
struct HitRecord_t;

class RayMaterial
{
public:
	//-----Public Methods-----

	virtual bool Scatter(const Ray& incomingRay, const HitRecord_t& record, Vector3& out_attentuation, Ray& out_scatteredRay) = 0;

private:
	//-----Private Data-----


};

class RayMaterial_Diffuse : public RayMaterial
{
public:
	//-----Public Methods-----

	RayMaterial_Diffuse(const Vector3& albedo);
	virtual bool Scatter(const Ray& incomingRay, const HitRecord_t& record, Vector3& out_attentuation, Ray& out_scatteredRay) override;


private:
	//-----Private Data-----

	Vector3 m_albedoColor;

};

class RayMaterial_Metal : public RayMaterial
{
public:
	//-----Public Methods-----

	RayMaterial_Metal(const Vector3& albedo, float fuzziness = 0.f);
	virtual bool Scatter(const Ray& incomingRay, const HitRecord_t& record, Vector3& out_attentuation, Ray& out_scatteredRay) override;

private:
	//-----Private Data-----

	Vector3 m_albedoColor;
	float m_fuzziness; // 0 to 1
};
