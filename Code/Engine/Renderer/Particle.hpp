/************************************************************************/
/* File: Particle.hpp
/* Author: Andrew Chase
/* Date: May 6th, 2018
/* Description: Class to represent a single particle within a ParticleEmitter
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Transform.hpp"

class Particle
{
	friend class ParticleEmitter;

public:
	//-----Public Methods-----

	void Update(float deltaTime);

	float GetNormalizedTime(float currentTime) const;
	bool IsDead(float currentTime) const;


private:
	//-----Private Data-----

	Transform m_transform;

	// Linear physics
	Vector3 m_velocity;
	Vector3 m_force;

	// Angular physics
	Vector3 m_angularVelocity;
	Vector3 m_torque;

	float m_mass;

	float m_timeCreated;
	float m_timeToDestroy;

};
