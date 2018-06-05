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

	void SetMarkedForDelete(bool markedForDelete);

	Renderable& GetRenderable();
	bool		IsMarkedForDelete() const;


public:
	//-----Public Data-----

	Transform transform;


protected:
	//-----Protected Data-----

	Renderable  m_renderable;		// For rendering
	bool		m_markedForDelete;	// For end-of-frame deletion

};
