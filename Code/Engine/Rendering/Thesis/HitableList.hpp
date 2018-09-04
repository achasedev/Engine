/************************************************************************/
/* File: HitableList.hpp
/* Author: Andrew Chase
/* Date: September 4th, 2018
/* Description: Class to represent a collection of hitable objects
/************************************************************************/
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
