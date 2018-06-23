/************************************************************************/
/* File: GameObject.hpp
/* Author: Andrew Chase
/* Date: May 9th, 2018
/* Description: Class to represent any given object in the game scene
/************************************************************************/
#pragma once
#include "Engine/Math/Transform.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"

class GameObject
{
public:
	//-----Public Methods-----

	GameObject();
	virtual ~GameObject();

	virtual void Update(float deltaTime);

	void SetPhysicsRadius(float radius);
	void SetMarkedForDelete(bool markedForDelete);
	void AddTagUnique(const std::string& tag);
	bool RemoveTag(const std::string& tag);

	Renderable* GetRenderable();
	bool		IsMarkedForDelete() const;
	bool		HasTag(const std::string& tag);

	float		GetPhysicsRadius() const;

public:
	//-----Public Data-----

	Transform transform;


protected:
	//-----Protected Data-----

	Renderable*		m_renderable;		// For rendering
	bool			m_markedForDelete;	// For end-of-frame deletion

	float			m_physicsRadius;	// For collision detection, sphere collision for now

	std::vector<std::string> m_tags;
};
