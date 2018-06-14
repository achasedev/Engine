/************************************************************************/
/* File: AssimpLoader.hpp
/* Author: Andrew Chase
/* Date: June 13th, 2018
/* Description: Class used for loading files using the Assimp library
/************************************************************************/
#pragma once
#include <string>
#include <vector>
#include "Engine/Math/Matrix44.hpp"

class Renderable;
class SkeletonBase;
class Texture;

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;

class AssimpLoader
{
public:
	//-----Public Methods-----

	bool LoadFile(const std::string& filepath);

	// Accessors
	Renderable*		GetRenderable();


private:
	//-----Private Methods-----

	// Loading Order
	void BuildAllMeshesAndMaterials();
	void BuildMeshAndMaterialForMesh(aiMesh* aimesh);

	void BuildSkeletonHierarchy();

	// Mesh and Material Loading
	void ProcessNodeAndChildren(aiNode* ainode);

	// Skeletal and Animation Loading
	//void ExtractSkeletonFromNode(aiNode* ainode);


private:
	//-----Private Data-----

	Renderable*		m_renderable = nullptr;

	const aiScene* m_scene = nullptr;

};
