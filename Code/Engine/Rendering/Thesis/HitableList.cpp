#include "Engine/Rendering/Thesis/HitableList.hpp"



HitableList::HitableList(Hitable** list, int count)
	: m_list(list)
	, m_size(count)
{
}

bool HitableList::Hit(const Ray* ray, float tMin, float tMax, HitRecord_t& out_record)
{
	HitRecord_t tempRecord;
	bool hitAnything = false;
	double closestSoFar = tMax;

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

