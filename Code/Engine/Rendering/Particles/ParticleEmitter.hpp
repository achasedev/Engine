/************************************************************************/
/* File: ParticleEmitter.hpp
/* Author: Andrew Chase
/* Date: May 6th, 2018
/* Description: Class to represent a single emitter of particles
/************************************************************************/
#pragma once
#include <vector>
#include "Engine/Math/IntRange.hpp"
#include "Engine/Rendering/Particles/Particle.hpp"

class Clock;
class Stopwatch;
class Renderable;

// Typedefs for spawning callbacks
typedef Vector3 (*SpawnVelocity_cb)();
typedef Vector3 (*SpawnAngularVelocity_cb)();
typedef Vector3 (*SpawnScale_cb)();
typedef float (*SpawnLifetime_cb)();

// Default callbacks, in case one isn't set explicitly
Vector3 DefaultSpawnVelocity();
Vector3 DefaultSpawnAngularVelocity();
float	DefaultSpawnLifetime();
Vector3 DefaultSpawnScale();


class ParticleEmitter
{
public:
	//-----Public Methods-----

	ParticleEmitter(Clock* referenceClock);
	~ParticleEmitter();

	void SetTransform(const Vector3& position, const Vector3& rotation, const Vector3& scale);
	void SetRenderable(Renderable* renderable);

	void Update();
	
	// Spawning
	void SpawnParticle();
	void SpawnBurst();
	void SpawnBurst(unsigned int numToSpawn);

	void SetSpawnRate(unsigned int particlesPerSecond);
	void SetBurst(int minAmount, int maxAmount = -1);

	void SetKillWhenDone(bool killWhenDone);
	void SetParticlesParented(bool shouldParent);

	// Setting callbacks used when particles are spawned
	void SetSpawnVelocityFunction(SpawnVelocity_cb callback);
	void SetSpawnAngularVelocityFunction(SpawnAngularVelocity_cb callback);
	void SetSpawnLifetimeFunction(SpawnLifetime_cb callback);
	void SetSpawnScaleFunction(SpawnScale_cb callback);

	// Accessors
	bool IsFinished() const;
	int GetParticleCount() const;
	Renderable* GetRenderable() const;


public:
	//-----Public Data-----

	Transform transform;


private:
	//-----Private Data-----

	Renderable* m_renderable;

	std::vector<Particle> m_particles;

	bool m_spawnsOverTime = false;
	Stopwatch* m_stopwatch;

	bool m_killWhenDone = false;
	IntRange m_burstRange;

	Vector3 m_force = Vector3(0.f, -9.8f, 0.f);

	bool m_areParticlesParented = false;

	// Callbacks for spawning
	SpawnVelocity_cb m_spawnVelocityCallback = DefaultSpawnVelocity;
	SpawnAngularVelocity_cb m_spawnAngularVelocityCallback = DefaultSpawnAngularVelocity;
	SpawnLifetime_cb m_spawnLifetimeCallback = DefaultSpawnLifetime;
	SpawnScale_cb m_spawnScaleCallback = DefaultSpawnScale;
};
