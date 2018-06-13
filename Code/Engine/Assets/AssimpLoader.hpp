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
	SkeletonBase*	GetSkeleton();


private:
	//-----Private Methods-----

	// Loading Order
	void ParseMeshAndModelDataFromScene();
	//void ParseSkeletonDataFromScene();

	// Mesh and Material Loading
	void					ExtractMeshAndMaterialsFromNode(aiNode* ainode);
	void					BuildMeshAndMaterialsFromMesh(aiMesh* aimesh, const Matrix44& transform);

	// Skeletal and Animation Loading
	//void ExtractSkeletonFromNode(aiNode* ainode);
	//void ExtractSkeletonFromNode_NoMesh(aiNode* ainode);
	//void ExtractSkeletonFromNode_WithMesh(aiNode* ainode);

	// Assimp Utility
	static Matrix44	ExtractTransformationFromNode(aiNode* ainode);


private:
	//-----Private Data-----

	Renderable*		m_renderable = nullptr;
	SkeletonBase*	m_skeleton = nullptr;

	const aiScene* m_scene = nullptr;

};
