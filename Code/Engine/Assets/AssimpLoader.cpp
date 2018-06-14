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

	m_renderable = new Renderable();
	m_renderable->SetSkeleton(new SkeletonBase());

	// Load Mesh, Material, and Skeletal data
	BuildAllMeshesAndMaterials();

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
// Parses the aiScene loaded from the file for all Mesh and Material data, and assembles a renderable
//
void AssimpLoader::BuildAllMeshesAndMaterials()
{
	// Iterate across all meshes in the scene object
	unsigned int numMeshes = m_scene->mNumMeshes;
	for (unsigned int meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
	{
		BuildMeshAndMaterialForMesh(m_scene->mMeshes[meshIndex]);
	}
}


//-----------------------------------------------------------------------------------------------
// Constructs all Mesh and Material data from the given node
//
void AssimpLoader::ProcessNodeAndChildren(aiNode* ainode)
{
	Matrix44 nodeTransform = ConvertAiMatrixToMyMatrix(ainode->mTransformation);

	// Process meshes  
	for (unsigned int meshIndex = 0; meshIndex < ainode->mNumMeshes; ++meshIndex)
	{
		aiMesh* aimesh =  m_scene->mMeshes[ainode->mMeshes[meshIndex]];

		// NOTE: Don't set the draw matrix, since all vertices are now in model ("world") space, no need to transform again
		BuildMeshAndMaterialForMesh(aimesh);
	}

	// Recursively process the child nodes
	for (unsigned int nodeIndex = 0; nodeIndex < ainode->mNumChildren; ++nodeIndex)
	{
		ProcessNodeAndChildren(ainode->mChildren[nodeIndex]);
	}
}


//-----------------------------------------------------------------------------------------------
// Builds the mesh and the materials from the given aiMesh
//
void AssimpLoader::BuildMeshAndMaterialForMesh(aiMesh* aimesh)
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

	// Bone information
	unsigned int numBones = aimesh->mNumBones;
	for (unsigned int boneIndex = 0; boneIndex < numBones; ++boneIndex)
	{
		aiBone* currBone = aimesh->mBones[boneIndex];
		std::string boneName = currBone->mName.C_Str();

		SkeletonBase* skeleton = m_renderable->GetSkeletonBase();
		unsigned int mappingIndex = skeleton->CreateOrGetBoneMapping(boneName);
		skeleton->SetOffsetMatrix(mappingIndex, ConvertAiMatrixToMyMatrix(currBone->mOffsetMatrix));

		// Iterate across the weights for this bone
		for (unsigned int weightIndex = 0; weightIndex < currBone->mNumWeights; ++weightIndex)
		{
			unsigned int vertexIndex = currBone->mWeights[weightIndex].mVertexId;
			float weightValue = currBone->mWeights[weightIndex].mWeight;

			mb.AddBoneData(vertexIndex, mappingIndex, weightValue);
		}
	}

	mb.FinishBuilding();

	Mesh* mesh;
	
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

		// Make the material
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

	RenderableDraw_t draw;
	draw.sharedMaterial = material;
	draw.mesh = mesh;

	m_renderable->AddDraw(draw);
}


//-----------------------------------------------------------------------------------------------
// Builds the skeleton transforms by traversing the assimp node hierarchy
//
void AssimpLoader::BuildSkeletonHierarchy()
{

}


//-----------------------------------------------------------------------------------------------
// Extracts the transformation matrix from the aiNode
//
Matrix44 ConvertAiMatrixToMyMatrix(aiMatrix4x4 aimatrix)
{
	aiVector3D aiPosition, aiRotation, aiScale;
	aimatrix.Decompose(aiScale, aiRotation, aiPosition);

	Vector3 position, rotation, scale;
	position.x = aiPosition.x;
	position.y = aiPosition.y;
	position.z = aiPosition.z;

	rotation.x = ConvertRadiansToDegrees(aiRotation.x);
	rotation.y = ConvertRadiansToDegrees(aiRotation.y);
	rotation.z = ConvertRadiansToDegrees(aiRotation.z);

	scale.x = aiScale.x;
	scale.y = aiScale.y;
	scale.z = aiScale.z;

	Matrix44 transform = Matrix44::MakeModelMatrix(position, rotation, scale);

	return transform;
}
