#include "Engine/Rendering/Thesis/Ray.hpp"

Ray::Ray()
	: m_position(Vector3::ZERO), m_direction(Vector3::ZERO)
{
}

Ray::Ray(const Vector3& position, const Vector3& direction)
	: m_position(position), m_direction(direction)
{
}

Vector3 Ray::GetPosition() const
{
	return m_position;
}

Vector3 Ray::GetDirection() const
{
	return m_direction;
}

Vector3 Ray::GetPointAtParameter(float t) const
{
	return m_position + m_direction * t;
}

