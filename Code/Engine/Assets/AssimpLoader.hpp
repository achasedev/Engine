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
struct aiNodeAnim;
struct aiAnimation;
struct aiString;

class AssimpLoader
{
public:
	//-----Public Methods-----

	bool LoadFile(const std::string& filepath);

	// Accessors
	Renderable*		GetRenderable();


private:
	//-----Private Methods-----

	// Meshes (including bone vertex data) and materials
	void BuildMeshesAndMaterials_FromScene();
		void BuildMeshesAndMaterials_FromNode(aiNode* node, const Matrix44& parentTransform);
			void BuildMeshAndMaterial_FromAIMesh(aiMesh* mesh, const Matrix44& transformation);

	// Build skeletal structure
	void BuildBoneHierarchy();
		void ExtractBoneTransform(aiNode* ainode, const Matrix44& parentTransfrom, int parentBoneIndex);

	// Build Animations


private:
	//-----Private Data-----

	Renderable*		m_renderable = nullptr;
	const aiScene* m_scene = nullptr;

};
