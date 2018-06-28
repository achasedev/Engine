/************************************************************************/
/* File: AssimpLoader.cpp
/* Author: Andrew Chase
/* Date: June 13th, 2018
/* Description: Implementaton of the Assimp class
/************************************************************************/
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Assets/AssimpLoader.hpp"
#include "Engine/Core/Time/ScopedProfiler.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Rendering/Materials/Material.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
#include "Engine/Rendering/Animation/SkeletonBase.hpp"

#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

// Assimp
#include "ThirdParty/assimp/include/assimp/scene.h"
#include "ThirdParty/assimp/include/assimp/cimport.h"
#include "ThirdParty/assimp/include/assimp/Importer.hpp"
#include "ThirdParty/assimp/include/assimp/postprocess.h"

// C functions
Matrix44 ConvertAiMatrixToMyMatrix(aiMatrix4x4 aimatrix);


//-----------------------------------------------------------------------------------------------
// C function - Find the textures used by the given aiMaterial and attempts to load and return them
//
std::vector<Texture*> LoadAssimpMaterialTextures(aiMaterial* aimaterial, aiTextureType type)
{
	std::vector<Texture*> textures;

	for (unsigned int textureIndex = 0; textureIndex < aimaterial->GetTextureCount(type); ++textureIndex)
	{
		aiString texturePath;
		aimaterial->GetTexture(type, textureIndex, &texturePath);

		Texture* texture = AssetDB::CreateOrGetTexture(texturePath.C_Str());

		if (texture == nullptr)
		{
			// Default the texture to some default
			switch(type)
			{
			case aiTextureType_DIFFUSE:
				texture = AssetDB::GetTexture("White");
				break;
			case aiTextureType_NORMALS:
				texture = AssetDB::GetTexture("Flat");
				break;
			case aiTextureType_EMISSIVE:
				texture = AssetDB::GetTexture("Black");
				break;
			}
		}

		textures.push_back(texture);
	}

	return textures;
}


//-----------------------------------------------------------------------------------------------
// Loads the model file using Assimp, and attempts to construct a renderable and SkeletonBase from
// the information
//
bool AssimpLoader::LoadFile(const std::string& filepath)
{
	ScopedProfiler sp = ScopedProfiler(Stringf("LoadFile: \"%s\"", filepath.c_str()));
	UNUSED(sp);

	Assimp::Importer importer;
	m_scene = importer.ReadFile(filepath.c_str(), aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_MakeLeftHanded);

	// Ensure the file loads
	if (m_scene == nullptr || m_scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || m_scene->mRootNode == nullptr)
	{
		ERROR_AND_DIE(Stringf("Error: AssimpLoader::LoadFile ran into error \"%s\" while loading file \"%s\"", importer.GetErrorString(), filepath.c_str()));
	}

	// Debugging

	m_renderable = new Renderable();
	SkeletonBase* skeleton = new SkeletonBase();
	aiMatrix4x4 inverseGlobal = m_scene->mRootNode->mTransformation;
	inverseGlobal.Inverse();

	skeleton->SetGlobalInverseTransform(ConvertAiMatrixToMyMatrix(inverseGlobal));
	m_renderable->SetSkeleton(skeleton);

	// Create mesh and material data
	BuildMeshesAndMaterials_FromScene();

	// Load the skeletal hierarchy
	BuildBoneHierarchy();

	// Load animations

	importer.FreeScene();

	return true;
}


//-----------------------------------------------------------------------------------------------
// Returns the renderable created from the loaded file
//
Renderable* AssimpLoader::GetRenderable()
{
	return m_renderable;
}


//-----------------------------------------------------------------------------------------------
// Returns the renderable created from the loaded file
//
void AssimpLoader::BuildMeshesAndMaterials_FromScene()
{
	BuildMeshesAndMaterials_FromNode(m_scene->mRootNode, Matrix44::IDENTITY);
}


//-----------------------------------------------------------------------------------------------
// Builds all meshes and materials from the data on the given node
//
void AssimpLoader::BuildMeshesAndMaterials_FromNode(aiNode* node, const Matrix44& parentTransform)
{
	 Matrix44 currTransform = ConvertAiMatrixToMyMatrix(node->mTransformation);
	 currTransform = parentTransform * currTransform;

	 int numMeshes = (int) node->mNumMeshes;

	 for (int meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
	 {
		 BuildMeshAndMaterial_FromAIMesh(m_scene->mMeshes[node->mMeshes[meshIndex]], currTransform);
	 }

	 int numChildren = node->mNumChildren;
	 for (int childIndex = 0; childIndex < numChildren; ++childIndex)
	 {
		 BuildMeshesAndMaterials_FromNode(node->mChildren[childIndex], currTransform);
	 }
}


//-----------------------------------------------------------------------------------------------
// Constructs the mesh and material data from the given aiMesh structure
// The transformation passed is the space the current mesh exists in, and is used to convert
// all mesh vertices into "model" space
//
void AssimpLoader::BuildMeshAndMaterial_FromAIMesh(aiMesh* aimesh, const Matrix44& transformation)
{
	MeshBuilder mb;
	mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);

	for (unsigned int vertexIndex = 0; vertexIndex < aimesh->mNumVertices; ++vertexIndex)
	{
		// Get position (should always have a position)
		Vector3 position;

		position.x = aimesh->mVertices[vertexIndex].x;
		position.y = aimesh->mVertices[vertexIndex].y;
		position.z = aimesh->mVertices[vertexIndex].z;

		// Get normal, if it exists
		Vector3 normal = Vector3::ZERO;
		if (aimesh->HasNormals())
		{
			normal.x = aimesh->mNormals[vertexIndex].x;
			normal.y = aimesh->mNormals[vertexIndex].y;
			normal.z = aimesh->mNormals[vertexIndex].z;
		}

		// Get tangent, if it exists
		Vector3 tangent = Vector3::ZERO;
		if (aimesh->HasTangentsAndBitangents())
		{
			tangent.x = aimesh->mTangents[vertexIndex].x;
			tangent.y = aimesh->mTangents[vertexIndex].y;
			tangent.z = aimesh->mTangents[vertexIndex].z;
		}


		// Get uvs, if they exist
		Vector2 uvs = Vector2::ZERO;
		if (aimesh->HasTextureCoords(0))
		{
			// Only one texture coordinate per vertex, so take the 0th one
			uvs.x = aimesh->mTextureCoords[0][vertexIndex].x;
			uvs.y = aimesh->mTextureCoords[0][vertexIndex].y;
		}

		//Matrix44 toModelTransform = m_renderable->GetSkeletonBase()->GetGlobalInverseTransform() * transformation;
		//
		//normal = toModelTransform.TransformVector(normal).xyz();
		//tangent = toModelTransform.TransformVector(tangent).xyz();
		//position = toModelTransform.TransformPoint(position).xyz();

		mb.SetNormal(normal);
		mb.SetTangent(Vector4(tangent, 1.0f));
		mb.SetUVs(uvs);

		// Push the vertex into the MeshBuilder
		mb.PushVertex(position);
	}

	// Mesh indices
	for (unsigned int i = 0; i < aimesh->mNumFaces; ++i)
	{
		aiFace face = aimesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			mb.PushIndex(face.mIndices[j]);
		}
	}

	// Bone Vertex Information
	unsigned int numBones = aimesh->mNumBones;
	for (unsigned int boneIndex = 0; boneIndex < numBones; ++boneIndex)
	{
// 		aiBone* currBone = aimesh->mBones[boneIndex];
// 		std::string boneName = currBone->mName.C_Str();
// 
// 		SkeletonBase* skeleton = m_renderable->GetSkeletonBase();
// 		unsigned int mappingIndex = skeleton->CreateOrGetBoneMapping(boneName);
// 		skeleton->SetOffsetMatrix(mappingIndex, ConvertAiMatrixToMyMatrix(currBone->mOffsetMatrix));
// 
// 		// Iterate across the weights for this bone
// 		for (unsigned int weightIndex = 0; weightIndex < currBone->mNumWeights; ++weightIndex)
// 		{
// 			unsigned int vertexIndex = currBone->mWeights[weightIndex].mVertexId;
// 			float weightValue = currBone->mWeights[weightIndex].mWeight;
// 
// 			// Set the index and weight data in the vertex buffer
// 			mb.AddBoneData(vertexIndex, mappingIndex, weightValue);
// 		}
	}

	mb.FinishBuilding();

	Mesh* mesh;
	
	// Only build with skinned vertices if bones are present, slight optimization
	if (numBones > 0)
	{
		mesh = mb.CreateMesh<VertexSkinned>();
	}
	else
	{
		mesh = mb.CreateMesh<VertexLit>();
	}

	// Build the material
	Material* material = AssetDB::GetSharedMaterial("Default_Opaque");
	if (aimesh->mMaterialIndex >= 0)
	{
		aiMaterial* aimaterial = m_scene->mMaterials[aimesh->mMaterialIndex];
		std::vector<Texture*> diffuse, normal, emissive;

		diffuse		= LoadAssimpMaterialTextures(aimaterial,	aiTextureType_DIFFUSE);
		normal		= LoadAssimpMaterialTextures(aimaterial,	aiTextureType_NORMALS);
		emissive	= LoadAssimpMaterialTextures(aimaterial,	aiTextureType_EMISSIVE);

		// In case the model has multiple textures of the same type on a single mesh
		if (diffuse.size() > 1)
		{
			ConsoleWarningf("Warning: multiple diffuse textures for a single mesh detected.");
		}

		if (normal.size() > 1)
		{
			ConsoleWarningf("Warning: multiple normal textures for a single mesh detected.");
		}

		if (emissive.size() > 1)
		{
			ConsoleWarningf("Warning: multiple emissive textures for a single mesh detected.");
		}

		// Make the material, defaulting missing textures to built-in engine textures
		material = new Material();
		if (diffuse.size() > 0)
		{
			material->SetDiffuse(diffuse[0]);
		}
		else
		{
			material->SetDiffuse(AssetDB::GetTexture("Default"));
		}

		if (normal.size() > 0)
		{
			material->SetNormal(normal[0]);
		}
		else
		{
			material->SetNormal(AssetDB::GetTexture("Flat"));
		}

		// Setting emissive, even though we don't support them yet
		if (emissive.size() > 0)
		{
			material->SetEmissive(emissive[0]);
		}
		else
		{
			material->SetEmissive(AssetDB::GetTexture("Black"));
		}

		material->SetShader(AssetDB::GetShader("Phong_Opaque"));
		material->SetProperty("SPECULAR_AMOUNT", 0.3f);
		material->SetProperty("SPECULAR_POWER", 10.f);
	}

	// Add the draw!
	RenderableDraw_t draw;
	draw.sharedMaterial = material;
	draw.mesh = mesh;

	m_renderable->AddDraw(draw);
}


//-----------------------------------------------------------------------------------------------
// Constructs all Mesh and Material data from the given node
//
void AssimpLoader::ExtractBoneTransform(aiNode* ainode, const Matrix44& parentTransform, int parentBoneIndex)
{
// 	std::string nodeName = ainode->mName.data;
// 
// 	Matrix44 nodeTransform = ConvertAiMatrixToMyMatrix(ainode->mTransformation);
// 
// 	Matrix44 currentTransform = parentTransform * nodeTransform;
// 
// 	// Check if this name is a bone by searching for it's name mapping in the hierarchy
// 	SkeletonBase* skeleton = m_renderable->GetSkeletonBase();
// 	int thisBoneIndex = skeleton->GetBoneMapping(nodeName);
// 
// 	// If it is a bone, update the matrix in the skeleton (might be unnecessary for bind pose, as you get the same result!)
// 	if (thisBoneIndex >= 0)
// 	{
// 		Matrix44 offset = skeleton->GetBoneData(thisBoneIndex).offsetMatrix;
// 		Matrix44 finalTransformation = skeleton->GetGlobalInverseTransform() * currentTransform * skeleton->GetBoneData(thisBoneIndex).offsetMatrix;
// 
// 		skeleton->SetFinalTransformation(thisBoneIndex, finalTransformation);
// 
// 		// For debugging
// 		skeleton->SetWorldTransform(thisBoneIndex, currentTransform);
// 		skeleton->SetParentBoneIndex(thisBoneIndex, parentBoneIndex);
// 	}
// 	
// 	// Determine what the parent index of the children bones are (either us if we're a bone, or our last bone ancestor)
// 	int childParentIndex = (thisBoneIndex >= 0 ? thisBoneIndex : parentBoneIndex);
// 
// 	// Recursively process the child nodes for bone information (even if this one wasn't a bone)
// 	for (unsigned int nodeIndex = 0; nodeIndex < ainode->mNumChildren; ++nodeIndex)
// 	{
// 		ExtractBoneTransform(ainode->mChildren[nodeIndex], currentTransform, childParentIndex);
// 	}
}


//-----------------------------------------------------------------------------------------------
// Builds the skeleton transforms by traversing the assimp node hierarchy
//
void AssimpLoader::BuildBoneHierarchy()
{
// 	// Set up the skeleton global inverse first, to convert vertices from world to local model space
// 	aiMatrix4x4 globalInverse = m_scene->mRootNode->mTransformation;
// 	globalInverse.Inverse();
// 
// 	SkeletonBase* skeleton = m_renderable->GetSkeletonBase();
// 	skeleton->SetGlobalInverseTransform(ConvertAiMatrixToMyMatrix(globalInverse));
// 
// 	// Recursively traverse the tree to assemble the bone transformations
// 	ExtractBoneTransform(m_scene->mRootNode, Matrix44::IDENTITY, -1);
}


//-----------------------------------------------------------------------------------------------
// Constructs a Matrix44 object from the aiMatrix4x4 by pulling rows as columns (row major to column major)
//
Matrix44 ConvertAiMatrixToMyMatrix(aiMatrix4x4 aimatrix)
{
	Matrix44 result;

	result.Ix = aimatrix.a1;
	result.Iy = aimatrix.b1;
	result.Iz = aimatrix.c1;
	result.Iw = aimatrix.d1;

	result.Jx = aimatrix.a2;
	result.Jy = aimatrix.b2;
	result.Jz = aimatrix.c2;
	result.Jw = aimatrix.d2;

	result.Kx = aimatrix.a3;
	result.Ky = aimatrix.b3;
	result.Kz = aimatrix.c3;
	result.Kw = aimatrix.d3;

	result.Tx = aimatrix.a4;
	result.Ty = aimatrix.b4;
	result.Tz = aimatrix.c4;
	result.Tw = aimatrix.d4;

	return result;
}
