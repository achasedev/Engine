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
// Adds the given tag to this object's list of tags, checking for duplicates
//
void GameObject::AddTagUnique(const std::string& tag)
{
	RemoveTag(tag);
	m_tags.push_back(tag);
}


//-----------------------------------------------------------------------------------------------
// Removes the tag from this object's list of tags
// Returns true if it was found and removed, false otherwise
//
bool GameObject::RemoveTag(const std::string& tag)
{
	for (int tagIndex = 0; tagIndex < (int) m_tags.size(); ++tagIndex)
	{
		if (m_tags[tagIndex] == tag)
		{
			// Quick delete, since order doesn't matter
			int lastIndex = (int) m_tags.size() - 1;

			m_tags[tagIndex] = m_tags[lastIndex];
			m_tags.erase(m_tags.begin() + lastIndex);

			return true;
		}
	}

	return false;
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


//-----------------------------------------------------------------------------------------------
// Returns true if the tag if the tag exists on this object, false otherwise
//
bool GameObject::HasTag(const std::string& tag)
{
	bool tagExists = std::find(m_tags.begin(), m_tags.end(), tag) != m_tags.end();

	return tagExists;
}
