#pragma once
#include "Engine/Rendering/Thesis/Hitable.hpp"

class HitableList : public Hitable
{
public:
	//-----Public methods-----

	HitableList(Hitable** list, int count);

	virtual bool Hit(const Ray& ray, float tMin, float tMax, HitRecord_t& out_record) override;


private:
	//-----Private Data-----

	Hitable** m_list = nullptr;
	int m_size;
};