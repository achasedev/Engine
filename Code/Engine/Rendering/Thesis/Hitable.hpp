#pragma once
#include "Engine/Math/Vector3.hpp"

class RayMaterial;

struct HitRecord_t
{
	float t;
	Vector3 position;
	Vector3 normal;
	RayMaterial* rayMaterial;
};

class Ray;
class RayMaterial;

class Hitable
{
public:
	//-----Public Methods-----

	virtual bool Hit(const Ray& ray, float tMin, float tMax, HitRecord_t& out_record) = 0;

};