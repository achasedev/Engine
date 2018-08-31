#pragma once
#include "Engine/Rendering/Thesis/Hitable.hpp"

class RaySphere : public Hitable
{
public:
	//-----Public Methods-----

	RaySphere();
	RaySphere(const Vector3& center, float radius);

	virtual bool Hit(const Ray* ray, float tMin, float tMax, HitRecord_t& out_record) override;

private:
	//-----Private Data-----

	Vector3 m_center;
	float m_radius;
};