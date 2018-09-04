/************************************************************************/
/* File: RayMaterial.hpp
/* Author: Andrew Chase
/* Date: September 4th, 2018
/* Description: Classes to represent a ray trace material for shading
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"

class Ray;
struct HitRecord_t;

// Base abstract class for all ray materials
class RayMaterial
{
public:
	//-----Public Methods-----

	virtual bool Scatter(const Ray& incomingRay, const HitRecord_t& record, Vector3& out_attentuation, Ray& out_scatteredRay) = 0;

private:
	//-----Private Data-----


};


// Class for diffuse shading with a single albedo color
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


// Class for reflective shading, using an albedo color and reflection fuzziness factor
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


// Class for refraction/reflection shading (glass)
class RayMaterial_Dielectric : public RayMaterial
{
public:
	//-----Public Methods-----

	RayMaterial_Dielectric(float indexOfRefraction);

	virtual bool Scatter(const Ray& incomingRay, const HitRecord_t& record, Vector3& out_attentuation, Ray& out_scatteredRay) override;


private:
	//-----Private Methods-----

	static float GetSchlickApproximation(float cosine, float indexOfRefraction); // For determining if the ray should refract or reflect


private:
	//-----Private Data-----

	float m_indexOfRefraction;
	
};
