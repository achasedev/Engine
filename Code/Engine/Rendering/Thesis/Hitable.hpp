/************************************************************************/
/* File: Hitable.hpp
/* Author: Andrew Chase
/* Date: September 4th, 2018
/* Description: Abstract Class to represent an object that can be hit by
				a ray trace during ray trace rendering
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"

class RayMaterial;

// Record of a hit
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
