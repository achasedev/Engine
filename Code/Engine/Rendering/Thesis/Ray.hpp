#pragma once

#include "Engine/Math/Vector3.hpp"

class Ray
{
public:
	//-----Public Methods-----

	Ray();
	Ray(const Vector3& position, const Vector3& direction);

	Vector3 GetPosition() const;
	Vector3 GetDirection() const;
	Vector3 GetPointAtParameter(float t) const;

private:
	//-----Private Data-----

	Vector3 m_position;
	Vector3 m_direction;

};
