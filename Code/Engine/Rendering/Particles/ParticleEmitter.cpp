/************************************************************************/
/* File: ParticleEmitter.cpp
/* Author: Andrew Chase
/* Date: May 6th, 2018
/* Description: Implementation of the ParticleEmitter class
/************************************************************************/
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Particles/ParticleEmitter.hpp"

#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor - takes a clock for timing, all particles of this emitter will use this clock
//
ParticleEmitter::ParticleEmitter(Clock* referenceClock)
{
	m_stopwatch = new Stopwatch(referenceClock);
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
ParticleEmitter::~ParticleEmitter()
{
	delete m_stopwatch;
	m_stopwatch = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Sets the emitter's transform to match the parameters specified                              
//
void ParticleEmitter::SetTransform(const Vector3& position, const Vector3& rotation, const Vector3& scale)
{
	transform.position = position;
	transform.rotation = rotation;
	transform.scale = scale;
}


//-----------------------------------------------------------------------------------------------
// Sets the renderable for this particle emitter
// Also clears the passed renderable's instance list, to ensure we don't render anything other than
// particle instances
//
void ParticleEmitter::SetRenderable(Renderable* renderable)
{
	m_renderable = renderable;

	// Ensure that there aren't any instances on the renderable already
	m_renderable->ClearInstances();
}


//-----------------------------------------------------------------------------------------------
// Sets the renderable for this particle emitter
// Also clears the passed renderable's instance list, to ensure we don't render anything other than
// particle instances
//
void ParticleEmitter::Update()
{
	if (m_spawnsOverTime)
	{
		unsigned int numParticles = m_stopwatch->DecrementByIntervalAll();
		SpawnBurst(numParticles);
	}

	// Iterate across the particles
	int numParticles = (int) m_particles.size();
	for (int particleIndex = numParticles - 1; particleIndex >= 0; --particleIndex)
	{
		Particle& p = m_particles[particleIndex];
		p.m_force = m_force;
		p.Update(m_stopwatch->GetDeltaSeconds());
		m_renderable->SetInstanceMatrix(particleIndex, p.m_transform.GetWorldMatrix());

		if (p.IsDead(m_stopwatch->GetTotalSeconds()))
		{
			m_particles.erase(m_particles.begin() + particleIndex);
			m_renderable->RemoveInstanceMatrix(particleIndex);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Spawns a single particle, given the emitter's current spawn state
//
void ParticleEmitter::SpawnParticle()
{
	Particle particle;
	particle.m_velocity = m_spawnVelocityCallback();
	particle.m_angularVelocity = m_spawnAngularVelocityCallback();
	particle.m_mass = 1.0f;

	particle.m_force = m_force;
	particle.m_torque = Vector3::ZERO;

	particle.m_timeCreated = m_stopwatch->GetTotalSeconds();
	particle.m_timeToDestroy = particle.m_timeCreated + m_spawnLifetimeCallback();

	particle.m_transform.Scale(m_spawnScaleCallback());

	if (m_areParticlesParented)
	{
		particle.m_transform.SetParentTransform(&transform);
	}
	else
	{
		particle.m_transform.position = transform.position;
	}

	m_renderable->AddInstanceMatrix(particle.m_transform.GetWorldMatrix());
	m_particles.push_back(particle);
}


//-----------------------------------------------------------------------------------------------
// Spawns a set of particles, equal to a value in the burst range of the emitter
//
void ParticleEmitter::SpawnBurst()
{
	int spawnCount = m_burstRange.GetRandomInRange();
	SpawnBurst(spawnCount);
}


//-----------------------------------------------------------------------------------------------
// Spawns a set of particles, equal to the value passed
//
void ParticleEmitter::SpawnBurst(unsigned int numToSpawn)
{
	for (int particleIndex = 0; particleIndex < (int) numToSpawn; ++particleIndex)
	{
		SpawnParticle();
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the emitter to spawn the given number of particles per second
//
void ParticleEmitter::SetSpawnRate(unsigned int particlesPerSecond)
{
	if (particlesPerSecond == 0)
	{
		m_spawnsOverTime = false;
	}
	else
	{
		m_spawnsOverTime = true;
		m_stopwatch->SetInterval(1.0f / (float) particlesPerSecond);
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the burst range to the range specified
//
void ParticleEmitter::SetBurst(int minAmount, int maxAmount /*= -1*/)
{
	if (maxAmount == -1)
	{
		maxAmount = minAmount;
	}

	m_burstRange = IntRange(minAmount, maxAmount);
}


//-----------------------------------------------------------------------------------------------
// Sets the flag to indicate whether this emitter should be deleted if done emitting particles
//
void ParticleEmitter::SetKillWhenDone(bool killWhenDone)
{
	m_killWhenDone = killWhenDone;
}


//-----------------------------------------------------------------------------------------------
// Sets whether the particles' transforms should be parented to the emitter's transform
//
void ParticleEmitter::SetParticlesParented(bool shouldParent)
{
	m_areParticlesParented = shouldParent;
}


//-----------------------------------------------------------------------------------------------
// Sets the spawn velocity callback function to the one specified
//
void ParticleEmitter::SetSpawnVelocityFunction(SpawnVelocity_cb callback)
{
	m_spawnVelocityCallback = callback;
}


//-----------------------------------------------------------------------------------------------
// Sets the spawn angular velocity callback function to the one specified
//
void ParticleEmitter::SetSpawnAngularVelocityFunction(SpawnAngularVelocity_cb callback)
{
	m_spawnAngularVelocityCallback = callback;
}


//-----------------------------------------------------------------------------------------------
// Sets the spawn lifetime callback function to the one specified
//
void ParticleEmitter::SetSpawnLifetimeFunction(SpawnLifetime_cb callback)
{
	m_spawnLifetimeCallback = callback;
}


//-----------------------------------------------------------------------------------------------
// Sets the spawn scale callback function to the one specified
//
void ParticleEmitter::SetSpawnScaleFunction(SpawnScale_cb callback)
{
	m_spawnScaleCallback = callback;
}


//-----------------------------------------------------------------------------------------------
// Returns true if this emitter is finished spawning, has no particles, and should be deleted when done
//
bool ParticleEmitter::IsFinished() const
{
	return (m_killWhenDone && m_spawnsOverTime == false && GetParticleCount() == 0);
}


//-----------------------------------------------------------------------------------------------
// Returns the number of particles currently spawned on the emitter
//
int ParticleEmitter::GetParticleCount() const
{
	return (int) m_particles.size();
}


//-----------------------------------------------------------------------------------------------
// Returns the number of particles currently spawned on the emitter
//
Renderable* ParticleEmitter::GetRenderable() const
{
	return m_renderable;
}


//---------- Default Spawn Callbacks ----------

//-----------------------------------------------------------------------------------------------
// Just returns (0,0,0)
//
Vector3 DefaultSpawnVelocity()
{
	return Vector3::ZERO;
}


//-----------------------------------------------------------------------------------------------
// Just returns (0,0,0)
//
Vector3 DefaultSpawnAngularVelocity()
{
	return Vector3::ZERO;
}


//-----------------------------------------------------------------------------------------------
// Returns 1.0f, indicating each particle lives for 1 second
//
float DefaultSpawnLifetime()
{
	return 1.0f;
}


//-----------------------------------------------------------------------------------------------
// Returns a default scale of (1,1,1)
//
Vector3 DefaultSpawnScale()
{
	return Vector3::ONES;
}
