#pragma once
#include "Engine/Math/Vector3.hpp"

struct HitRecord_t
{
	float t;
	Vector3 position;
	Vector3 normal;
};

class Ray;

class Hitable
{
public:
	//-----Public Methods-----

	virtual bool Hit(const Ray* ray, float tMin, float tMax, HitRecord_t& out_record) = 0;

};