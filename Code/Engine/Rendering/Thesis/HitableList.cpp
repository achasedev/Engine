/************************************************************************/
/* File: HitableList.cpp
/* Author: Andrew Chase
/* Date: September 4th, 2018
/* Description: Implementation of the hitable list class
/************************************************************************/
#include "Engine/Rendering/Thesis/HitableList.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
HitableList::HitableList(Hitable** list, int count)
	: m_list(list)
	, m_size(count)
{
}


//-----------------------------------------------------------------------------------------------
// Checks for a hit by the given ray
// Returns true if hit, false otherwise
//
bool HitableList::Hit(const Ray& ray, float tMin, float tMax, HitRecord_t& out_record)
{
	HitRecord_t tempRecord;
	bool hitAnything = false;
	float closestSoFar = tMax;

	for (int i = 0; i < m_size; ++i)
	{
		if (m_list[i]->Hit(ray, tMin, closestSoFar, tempRecord))
		{
			hitAnything = true;
			closestSoFar = tempRecord.t;
			out_record = tempRecord;
		}
	}

	return hitAnything;
}
