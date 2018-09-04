/************************************************************************/
/* File: RaySphere.hpp
/* Author: Andrew Chase
/* Date: September 4th, 2018
/* Description: Class to represent a hitable sphere raytrace object
/************************************************************************/
#pragma once
#include "Engine/Rendering/Thesis/Hitable.hpp"

class RaySphere : public Hitable
{
public:
	//-----Public Methods-----

	RaySphere();
	RaySphere(const Vector3& center, float radius, RayMaterial* rayMaterial);

	virtual bool Hit(const Ray& ray, float tMin, float tMax, HitRecord_t& out_record) override;


private:
	//-----Private Data-----

	Vector3			m_center;
	float			m_radius;
	RayMaterial*	m_rayMaterial;

};
