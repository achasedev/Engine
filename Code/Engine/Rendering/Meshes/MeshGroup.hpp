/************************************************************************/
/* File: MeshGroup.hpp
/* Author: Andrew Chase
/* Date: May 6th, 2018
/* Description: Class to represent a collection of meshes
/************************************************************************/
#pragma once
#include <vector>

class Mesh;

class MeshGroup
{
public:
	//-----Public Methods-----

	~MeshGroup();

	void AddMeshUnique(Mesh* mesh);
	void RemoveMesh(Mesh* mesh);
	void RemoveMesh(int index);

	Mesh*	GetMesh(int index) const;
	int		GetMeshCount() const;


private:
	//-----Private Data-----

	std::vector<Mesh*> m_meshes;

};
