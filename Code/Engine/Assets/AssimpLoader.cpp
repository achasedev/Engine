/************************************************************************/
/* File: AssimpLoader.cpp
/* Author: Andrew Chase
/* Date: June 13th, 2018
/* Description: Implementaton of the Assimp class
/************************************************************************/
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/Quaternion.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Assets/AssimpLoader.hpp"
#include "Engine/Rendering/Animation/Pose.hpp"
#include "Engine/Core/Time/ScopedProfiler.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Rendering/Materials/Material.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
#include "Engine/Rendering/Animation/SkeletonBase.hpp"
#include "Engine/Rendering/Animation/AnimationClip.hpp"

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
	BuildAnimations();

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


AnimationClip* AssimpLoader::GetAnimationClip(unsigned int index)
{
	return m_animationClips[index];
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

		Matrix44 toModelTransform = m_renderable->GetSkeletonBase()->GetGlobalInverseTransform() * transformation;

		normal = toModelTransform.TransformVector(normal).xyz();
		tangent = toModelTransform.TransformVector(tangent).xyz();
		position = toModelTransform.TransformPoint(position).xyz();

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

			// Set the index and weight data in the vertex buffer
			mb.AddBoneData(vertexIndex, mappingIndex, weightValue);
		}
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
	std::string nodeName = ainode->mName.data;

	Matrix44 nodeTransform = ConvertAiMatrixToMyMatrix(ainode->mTransformation);

	Matrix44 currentTransform = parentTransform * nodeTransform;

	// Check if this name is a bone by searching for it's name mapping in the hierarchy
	SkeletonBase* skeleton = m_renderable->GetSkeletonBase();
	int thisBoneIndex = skeleton->GetBoneMapping(nodeName);

	// If it is a bone, update the matrix in the skeleton (might be unnecessary for bind pose, as you get the same result!)
	if (thisBoneIndex >= 0)
	{
		Matrix44 offset = skeleton->GetBoneData(thisBoneIndex).offsetMatrix;
		Matrix44 finalTransformation = skeleton->GetGlobalInverseTransform() * currentTransform * skeleton->GetBoneData(thisBoneIndex).offsetMatrix;

		skeleton->SetFinalTransformation(thisBoneIndex, finalTransformation);

		// For debugging
		skeleton->SetWorldTransform(thisBoneIndex, currentTransform);
		skeleton->SetParentBoneIndex(thisBoneIndex, parentBoneIndex);
	}
	
	// Determine what the parent index of the children bones are (either us if we're a bone, or our last bone ancestor)
	int childParentIndex = (thisBoneIndex >= 0 ? thisBoneIndex : parentBoneIndex);

	// Recursively process the child nodes for bone information (even if this one wasn't a bone)
	for (unsigned int nodeIndex = 0; nodeIndex < ainode->mNumChildren; ++nodeIndex)
	{
		ExtractBoneTransform(ainode->mChildren[nodeIndex], currentTransform, childParentIndex);
	}
}


//-----------------------------------------------------------------------------------------------
// Builds all animations from the assimp tree and stores them on the loader
//
void AssimpLoader::BuildAnimations()
{
	unsigned int animationCount = m_scene->mNumAnimations;

	for (unsigned int animationIndex = 0; animationIndex < animationCount; ++animationIndex)
	{
		BuildAnimation(animationIndex);
	}
}


//-----------------------------------------------------------------------------------------------
// Builds a single animation at the given animation index
//
void AssimpLoader::BuildAnimation(unsigned int animationIndex)
{
	aiAnimation* aianimation = m_scene->mAnimations[animationIndex];
	
	// Get general animation data
	float numTicks = (float) aianimation->mDuration;
	float durationSeconds = (float) numTicks / (float) aianimation->mTicksPerSecond;

	float framesPerSecond = 30.f;	// Hard coding for now
	float secondsPerFrame = 1.f / framesPerSecond;

	int numFrames = Ceiling(durationSeconds * framesPerSecond);

	// Create the animation clip
	AnimationClip* animation = new AnimationClip();
	animation->Initialize(numFrames, m_renderable->GetSkeletonBase());
	animation->SetName(aianimation->mName.C_Str());
	animation->SetFramesPerSecond(framesPerSecond);
	animation->SetDurationSeconds(durationSeconds);

	// Iterate across the frames, and build a pose for each
	for (int frameIndex = 0; frameIndex < numFrames; ++frameIndex)
	{
		Pose* pose = animation->GetPoseAtIndex(frameIndex);

		float time = frameIndex * secondsPerFrame;
		FillPoseForTime(pose, aianimation, time);
	}

	m_animationClips.push_back(animation);
}


//-----------------------------------------------------------------------------------------------
// Fills in the pose given for the animation at time time
//
void AssimpLoader::FillPoseForTime(Pose* out_pose, aiAnimation* aianimation, float time)
{
	// Iterate across all the bones in the pose
		// Find the channel corresponding to the bone
		// If the channel exists, make the transform and store it in the pose
		// Else store identity

	SkeletonBase* skeleton = m_renderable->GetSkeletonBase();
	std::vector<std::string> boneNames = skeleton->GetAllBoneNames();
	out_pose->Initialize(skeleton);
	
	
	int numBones = (int) boneNames.size();

	for (int boneNameIndex = 0; boneNameIndex < numBones; ++boneNameIndex)
	{
		std::string currBoneName = boneNames[boneNameIndex];
		int boneDataIndex = skeleton->GetBoneMapping(currBoneName);

		aiNodeAnim* channel = GetChannelForBone(currBoneName, aianimation);

		if (channel != nullptr)
		{
			Matrix44 boneTransform = GetLocalTransfromAtTime(channel, time);
			out_pose->SetBoneTransform(boneDataIndex, boneTransform);
		}
		else
		{
			out_pose->SetBoneTransform(boneDataIndex, Matrix44::IDENTITY);
		}
	}

	// All the pose matrices are in their local space, so need to concatenate parents in order to finish it, as well as apply offsets matrices and the global inverse
	out_pose->ConstructGlobalMatrices();
}


//-----------------------------------------------------------------------------------------------
// Find the channel corresponding to the given bone name, returning nullptr if it doesn't exist
//
aiNodeAnim* AssimpLoader::GetChannelForBone(const std::string& boneName, aiAnimation* animation) const
{
	int numChannels = animation->mNumChannels;

	for (int channelIndex = 0; channelIndex < numChannels; ++channelIndex)
	{
		aiNodeAnim* currChannel = animation->mChannels[channelIndex];

		std::string nodeName = currChannel->mNodeName.C_Str();
		if (nodeName == boneName)
		{
			return currChannel;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Determines the transform to use from the channel at the given time
//
Matrix44 AssimpLoader::GetLocalTransfromAtTime(aiNodeAnim* channel, float time)
{
	// Assumes the start time for all nodes is mTime == 0

	// Translation
	Vector3 position = GetWorldTranslationAtTime(channel, time);

	// Rotation
	Quaternion rotation = GetWorldRotationAtTime(channel, time);

	// Scale
	Vector3 scale = GetWorldScaleAtTime(channel, time);

	// Make the model
	Matrix44 transform = Matrix44::MakeModelMatrix(position, rotation.GetAsEulerAngles(), scale);

	return transform;
}


Vector3 AssimpLoader::GetWorldTranslationAtTime(aiNodeAnim* channel, float time)
{
	int positionKeyIndex = 0;
	Vector3 finalPosition = Vector3(channel->mPositionKeys[0].mValue.x, channel->mPositionKeys[0].mValue.y, channel->mPositionKeys[0].mValue.z);

	// Only bother interpolating if there's more than one key
	if (channel->mNumPositionKeys > 1)
	{
		for (unsigned int translationKeyIndex = 0; translationKeyIndex < channel->mNumPositionKeys - 1; ++translationKeyIndex)
		{
			if (channel->mPositionKeys[translationKeyIndex + 1].mTime > time)
			{
				positionKeyIndex = translationKeyIndex;
				break;
			}
		}

		float firstTime = (float) channel->mPositionKeys[positionKeyIndex].mTime;
		float secondTime = (float) channel->mPositionKeys[positionKeyIndex + 1].mTime;

		float deltaTime = secondTime - firstTime;
		float interpolationFactor = (time - firstTime) / deltaTime;

		ASSERT_OR_DIE(interpolationFactor >= 0.f && interpolationFactor <= 1.0f, Stringf("Error: AssimpLoader::GetWorldPositionAtTime calculated interpolation factor out of range, factor was %f", interpolationFactor));

		aiVector3D aifirstPosition = channel->mPositionKeys[positionKeyIndex].mValue;
		aiVector3D aisecondPosition = channel->mPositionKeys[positionKeyIndex + 1].mValue;

		Vector3 firstPosition = Vector3(aifirstPosition.x, aifirstPosition.y, aifirstPosition.z);
		Vector3 secondPosition = Vector3(aisecondPosition.x, aisecondPosition.y, aisecondPosition.z);

		finalPosition = Interpolate(firstPosition, secondPosition, interpolationFactor);
	}

	return finalPosition;
}

Quaternion AssimpLoader::GetWorldRotationAtTime(aiNodeAnim* channel, float time)
{
	int rotationKeyIndex = 0;

	if (channel->mNumRotationKeys > 1)
	{
		for (unsigned int index = 0; index < channel->mNumRotationKeys - 1; ++index)
		{
			if (channel->mRotationKeys[index + 1].mTime > time)
			{
				rotationKeyIndex = index;
				break;
			}
		}
	}


	float firstTime = (float) channel->mRotationKeys[rotationKeyIndex].mTime;
	float secondTime = (float) channel->mRotationKeys[rotationKeyIndex + 1].mTime;

	float deltaTime = secondTime - firstTime;
	float interpolationFactor = (time - firstTime) / deltaTime;

	ASSERT_OR_DIE(interpolationFactor >= 0.f && interpolationFactor <= 1.0f, Stringf("Error: AssimpLoader::GetWorldRotationAtTime calculated interpolation factor out of range, factor was %f", interpolationFactor));

	aiQuaternion aiFirstRotation = channel->mRotationKeys[rotationKeyIndex].mValue;
	aiQuaternion aiSecondRotation = channel->mRotationKeys[rotationKeyIndex + 1].mValue;
	aiQuaternion aifinalRotation;
	aiQuaternion::Interpolate(aifinalRotation, aiFirstRotation, aiSecondRotation, interpolationFactor);
	aifinalRotation.Normalize();

	Quaternion finalRotation;

	finalRotation.s = aifinalRotation.w;
	finalRotation.v.x = aifinalRotation.x;
	finalRotation.v.y = aifinalRotation.y;
	finalRotation.v.z = aifinalRotation.z;

	return finalRotation;
}

Vector3 AssimpLoader::GetWorldScaleAtTime(aiNodeAnim* channel, float time)
{
	int scaleKeyIndex = 0;

	if (channel->mNumScalingKeys > 1)
	{
		for (unsigned int index = 0; index < channel->mNumScalingKeys - 1; ++index)
		{
			if (channel->mScalingKeys[index + 1].mTime > time)
			{
				scaleKeyIndex = index;
				break;
			}
		}
	}


	float firstTime = (float) channel->mScalingKeys[scaleKeyIndex].mTime;
	float secondTime = (float) channel->mScalingKeys[scaleKeyIndex + 1].mTime;

	float deltaTime = secondTime - firstTime;
	float interpolationFactor = (time - firstTime) / deltaTime;

	ASSERT_OR_DIE(interpolationFactor >= 0.f && interpolationFactor <= 1.0f, Stringf("Error: AssimpLoader::GetWorldPositionAtTime calculated interpolation factor out of range, factor was %f", interpolationFactor));

	aiVector3D aiFirstScale = channel->mScalingKeys[scaleKeyIndex].mValue;
	aiVector3D aiSecondScale = channel->mScalingKeys[scaleKeyIndex + 1].mValue;

	Vector3 firstScale = Vector3(aiFirstScale.x, aiFirstScale.y, aiFirstScale.z);
	Vector3 secondScale = Vector3(aiSecondScale.x, aiSecondScale.y, aiSecondScale.z);

	Vector3 finalScale = Interpolate(firstScale, secondScale, interpolationFactor);

	return finalScale;
}

//-----------------------------------------------------------------------------------------------
// Builds the skeleton transforms by traversing the assimp node hierarchy
//
void AssimpLoader::BuildBoneHierarchy()
{
	// Set up the skeleton global inverse first, to convert vertices from world to local model space
	aiMatrix4x4 globalInverse = m_scene->mRootNode->mTransformation;
	globalInverse.Inverse();

	SkeletonBase* skeleton = m_renderable->GetSkeletonBase();
	skeleton->SetGlobalInverseTransform(ConvertAiMatrixToMyMatrix(globalInverse));

	// Recursively traverse the tree to assemble the bone transformations
	ExtractBoneTransform(m_scene->mRootNode, Matrix44::IDENTITY, -1);
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
