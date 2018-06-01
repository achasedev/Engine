/************************************************************************/
/* File: Particle.cpp
/* Author: Andrew Chase
/* Date: May 6th, 2018
/* Description: Implementation of the Particle class
/************************************************************************/
#include "Engine/Renderer/Particle.hpp"


//-----------------------------------------------------------------------------------------------
// Updates the particle using a forward euler method
//
void Particle::Update(float deltaTime)
{
	// Forward Euler
	Vector3 acceleration = (m_force / m_mass);
	m_velocity += acceleration * deltaTime;
	m_transform.TranslateWorld(m_velocity * deltaTime);

	Vector3 angularAcceleration = (m_torque / m_mass);
	m_angularVelocity += angularAcceleration * deltaTime;
	m_transform.Rotate(m_angularVelocity * deltaTime);

	// Remove the force
	m_force = Vector3::ZERO;
	m_torque = Vector3::ZERO;
}


//-----------------------------------------------------------------------------------------------
// Returns a normalized parameter for time through the particle's life
//
float Particle::GetNormalizedTime(float currentTime) const
{
	return (currentTime - m_timeCreated) / (m_timeToDestroy - m_timeCreated);
}


//-----------------------------------------------------------------------------------------------
// Returns true if the particle's lifetime is over
//
bool Particle::IsDead(float currentTime) const
{
	return (currentTime >= m_timeToDestroy);
}
