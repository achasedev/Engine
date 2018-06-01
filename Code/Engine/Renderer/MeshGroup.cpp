/************************************************************************/
/* File: MeshGroup.cpp
/* Author: Andrew Chase
/* Date: May 6th, 2018
/* Description: Implementation of the MeshGroup class
/************************************************************************/
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Renderer/MeshGroup.hpp"


//-----------------------------------------------------------------------------------------------
// Destructor - delete the meshes, since MeshGroups owns the meshes
//
MeshGroup::~MeshGroup()
{
	int numMeshes = (int) m_meshes.size();

	for (int meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
	{
		delete m_meshes[meshIndex];
	}

	m_meshes.clear();
}


//-----------------------------------------------------------------------------------------------
// Adds the given mesh to the group, avoiding duplicates
//
void MeshGroup::AddMeshUnique(Mesh* mesh)
{
	RemoveMesh(mesh);
	m_meshes.push_back(mesh);
}


//-----------------------------------------------------------------------------------------------
// Removes the given mesh from the group
//
void MeshGroup::RemoveMesh(Mesh* mesh)
{
	int numMeshes = (int) m_meshes.size();

	for (int meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
	{
		if (m_meshes[meshIndex] == mesh)
		{
			m_meshes.erase(m_meshes.begin() + meshIndex);
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Removes the mesh located at index from the mesh group
//
void MeshGroup::RemoveMesh(int index)
{
	m_meshes.erase(m_meshes.begin() + index);
}


//-----------------------------------------------------------------------------------------------
// Returns the mesh located at the given index
//
Mesh* MeshGroup::GetMesh(int index) const
{
	return m_meshes[index];
}


//-----------------------------------------------------------------------------------------------
// Returns the number of meshes in the group
//
int MeshGroup::GetMeshCount() const
{
	return (int) m_meshes.size();
}
