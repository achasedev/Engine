/************************************************************************/
/* File: GameObject.cpp
/* Author: Andrew Chase
/* Date: May 9th, 2018
/* Description: Implementation of the GameObject class
/************************************************************************/
#include "Game/Framework/GameCommon.hpp"
#include "Engine/Core/GameObject.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
GameObject::GameObject()
	: m_markedForDelete(false)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
GameObject::~GameObject()
{
}


//-----------------------------------------------------------------------------------------------
// Update
//
void GameObject::Update(float deltaTime)
{
	UNUSED(deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Marks this GameObject for deletion at the end of the frame
//
void GameObject::SetMarkedForDelete(bool markedForDelete)
{
	m_markedForDelete = markedForDelete;
}


//-----------------------------------------------------------------------------------------------
// Returns the renderable of the GameObject
//
Renderable* GameObject::GetRenderable()
{
	return m_renderable;
}


//-----------------------------------------------------------------------------------------------
// Returns true if this object is marked for deletion, false otherwise
//
bool GameObject::IsMarkedForDelete() const
{
	return m_markedForDelete;
}
