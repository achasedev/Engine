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
#include "Engine/Rendering/Resources/Sampler.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Rendering/Materials/Material.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
#include "Engine/Rendering/Animation/SkeletonBase.hpp"
#include "Engine/Rendering/Animation/AnimationClip.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

// Assimp
Assimp::Importer g_importer;


// C functions
Matrix44 ConvertAiMatrixToMyMatrix(aiMatrix4x4 aimatrix);
Quaternion ConvertAiQuaternionToMyQuaternion(aiQuaternion aiQuat);

void DebugPrintAnimation(aiAnimation* anim);
void DebugPrintAITree(aiNode* node, const std::string& indent);


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

		std::string fullPath	= "Data/Models/" + std::string(texturePath.C_Str());
		Texture* texture		= AssetDB::CreateOrGetTexture(fullPath.c_str(), true);

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


void AssimpLoader::OpenFile(const std::string& filepath)
{
	if (m_scene == nullptr)
	{
		m_scene = g_importer.ReadFile(filepath.c_str(), aiProcessPreset_TargetRealtime_Quality | aiProcess_MakeLeftHanded);
	}

	// Ensure the file loads
	if (m_scene == nullptr || m_scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || m_scene->mRootNode == nullptr)
	{
		ERROR_AND_DIE(Stringf("Error: AssimpLoader::OpenFile ran into error \"%s\" while loading file \"%s\"", g_importer.GetErrorString(), filepath.c_str()));
	}
}

void AssimpLoader::CloseFile()
{
	g_importer.FreeScene();
	m_scene = nullptr;
}

Renderable* AssimpLoader::ImportMesh(SkeletonBase* skeleton /*= nullptr*/)
{
	//DebugPrintAITree(m_scene->mRootNode, "");

	Renderable* renderable = new Renderable();
	BuildMeshesAndMaterials_FromScene(renderable, skeleton);

	return renderable;
}

SkeletonBase* AssimpLoader::ImportSkeleton()
{
	DebugPrintAITree(m_scene->mRootNode, "");

	SkeletonBase* skeleton = new SkeletonBase();
	InitializeSkeleton(skeleton);

	return skeleton;
}

std::vector<AnimationClip*> AssimpLoader::ImportAnimation(SkeletonBase* skeleton, int firstFrame /*= 0*/)
{
	DebugPrintAITree(m_scene->mRootNode, "");

	std::vector<AnimationClip*> animations;
	BuildAnimations(skeleton, animations, firstFrame);

	return animations;
}


void AssimpLoader::InitializeSkeleton(SkeletonBase* skeleton)
{
	// 1. Get all the bone names in the tree
	std::vector<std::string> boneNames;
	GetBoneNamesFromNode(m_scene->mRootNode, boneNames);

	// 2. Create all the mappings for the bones in the correct order
	CreateBoneMappingsFromNode(m_scene->mRootNode, boneNames, skeleton);

	// 3. Set the bone data for the bones in the skeleton
	SetBoneOffsetData(m_scene->mRootNode, skeleton);

	// 4. Build the hierarchy
	BuildBoneHierarchy(skeleton);

	// 5. Construct local matrices
	for (int i = 0; i < skeleton->GetBoneCount(); ++i)
	{
		BoneData_t currBone = skeleton->GetBoneData(i);
		Matrix44 oldLocal = currBone.localTransform;
		int parentIndex = currBone.parentIndex;

		if (parentIndex >= 0)
		{
			BoneData_t parentBone = skeleton->GetBoneData(parentIndex);
			Matrix44 parentWorldInverse = parentBone.worldTransform;
			parentWorldInverse.Invert();

			Matrix44 newLocalTransform = parentWorldInverse * currBone.worldTransform;
			skeleton->SetLocalTransform(i, newLocalTransform);

			if (!AreMostlyEqual(oldLocal, newLocalTransform))
			{
				DebuggerPrintf("");
			}
		}
		else
		{
			skeleton->SetLocalTransform(i, currBone.worldTransform);
		}
	}
}

void AssimpLoader::GetBoneNamesFromNode(aiNode* node, std::vector<std::string>& out_names)
{
	int numMeshes = node->mNumMeshes;

	for (int meshIndex = 0; meshIndex < numMeshes; ++ meshIndex)
	{
		aiMesh* currMesh = m_scene->mMeshes[node->mMeshes[meshIndex]];

		int numBones = currMesh->mNumBones;

		for (int boneIndex = 0; boneIndex < numBones; ++boneIndex)
		{
			aiBone* currBone = currMesh->mBones[boneIndex];

			std::string boneName = std::string(currBone->mName.C_Str());

			if (std::find(out_names.begin(), out_names.end(), boneName) == out_names.end())
			{
				out_names.push_back(boneName);
			}
		}
	}

	int numChildren = node->mNumChildren;
	 
	for (int childIndex = 0; childIndex < numChildren; ++childIndex)
	{
		GetBoneNamesFromNode(node->mChildren[childIndex], out_names);
	}
}

void AssimpLoader::CreateBoneMappingsFromNode(aiNode* node, const std::vector<std::string>& boneNames, SkeletonBase* skeleton)
{
	std::string nodeName = node->mName.C_Str();

	if (std::find(boneNames.begin(), boneNames.end(), nodeName) != boneNames.end())
	{
		skeleton->CreateOrGetBoneMapping(nodeName);
	}

	int numChildren = node->mNumChildren;

	for (int childIndex = 0; childIndex < numChildren; ++childIndex)
	{
		CreateBoneMappingsFromNode(node->mChildren[childIndex], boneNames, skeleton);
	}
}

Matrix44 GetNodeWorldTransform(aiNode* node)
{
	if (node == nullptr)
	{
		return Matrix44::IDENTITY;
	}

	Matrix44 parentTransform = GetNodeWorldTransform(node->mParent);

	return parentTransform * ConvertAiMatrixToMyMatrix(node->mTransformation);
}

void AssimpLoader::SetBoneOffsetData(aiNode* node, SkeletonBase* skeleton)
{
	int numMeshes = (int) node->mNumMeshes;

	for (int meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
	{
		aiMesh* currMesh = m_scene->mMeshes[node->mMeshes[meshIndex]];

		int numBones = currMesh->mNumBones;
		for (int boneIndex = 0; boneIndex < numBones; ++boneIndex)
		{
			aiBone* currBone = currMesh->mBones[boneIndex];
			int boneMapping = skeleton->GetBoneMapping(currBone->mName.C_Str());

			ASSERT_OR_DIE(boneMapping >= 0, Stringf("Error: AssimpLoader::SetBoneOffsetData couldn't find mapping for bone \"%s\"", currBone->mName.C_Str()));

			Matrix44 offset = ConvertAiMatrixToMyMatrix(currBone->mOffsetMatrix);
			Matrix44 nodeTransform = GetNodeWorldTransform(node);

			skeleton->SetOffsetMatrix(boneMapping, offset * Matrix44::GetInverse(nodeTransform));
		}
	}

	int numChildren = node->mNumChildren;
	for (int childIndex = 0; childIndex < numChildren; ++childIndex)
	{
		SetBoneOffsetData(node->mChildren[childIndex], skeleton);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the renderable created from the loaded file
//
void AssimpLoader::BuildMeshesAndMaterials_FromScene(Renderable* renderable, SkeletonBase* skeleton)
{
	BuildMeshesAndMaterials_FromNode(m_scene->mRootNode, Matrix44::IDENTITY, renderable, skeleton);
}


//-----------------------------------------------------------------------------------------------
// Builds all meshes and materials from the data on the given node
//
void AssimpLoader::BuildMeshesAndMaterials_FromNode(aiNode* node, const Matrix44& parentTransform, Renderable* renderable, SkeletonBase* skeleton)
{
	 Matrix44 currTransform = ConvertAiMatrixToMyMatrix(node->mTransformation);
	 currTransform = parentTransform * currTransform;

	 int numMeshes = (int) node->mNumMeshes;

	 for (int meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
	 {
		 BuildMeshAndMaterial_FromAIMesh(m_scene->mMeshes[node->mMeshes[meshIndex]], currTransform, renderable, skeleton);
	 }

	 int numChildren = node->mNumChildren;
	 for (int childIndex = 0; childIndex < numChildren; ++childIndex)
	 {
		 BuildMeshesAndMaterials_FromNode(node->mChildren[childIndex], currTransform, renderable, skeleton);
	 }
}


//-----------------------------------------------------------------------------------------------
// Constructs the mesh and material data from the given aiMesh structure
// The transformation passed is the space the current mesh exists in, and is used to convert
// all mesh vertices into "model" space
//
void AssimpLoader::BuildMeshAndMaterial_FromAIMesh(aiMesh* aimesh, const Matrix44& transformation, Renderable* renderable, SkeletonBase* skeleton)
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

		normal = transformation.TransformVector(normal).xyz();
		tangent = transformation.TransformVector(tangent).xyz();
		position = transformation.TransformPoint(position).xyz();

		normal.NormalizeAndGetLength();
		tangent.NormalizeAndGetLength();

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

		if (face.mNumIndices != 3) { continue; }

		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			mb.PushIndex(face.mIndices[j]);
		}
	}

	// Bone Vertex Information
	if (skeleton != nullptr)
	{
		unsigned int numBones = aimesh->mNumBones;
		for (unsigned int boneIndex = 0; boneIndex < numBones; ++boneIndex)
		{
			aiBone* currBone = aimesh->mBones[boneIndex];
			std::string boneName = currBone->mName.C_Str();

			unsigned int mappingIndex = skeleton->GetBoneMapping(boneName);

			ASSERT_OR_DIE(mappingIndex >= 0, Stringf("Error: Mesh built with a bone name without a registered slot."));

			// Iterate across the weights for this bone
			for (unsigned int weightIndex = 0; weightIndex < currBone->mNumWeights; ++weightIndex)
			{
				unsigned int vertexIndex = currBone->mWeights[weightIndex].mVertexId;
				float weightValue = currBone->mWeights[weightIndex].mWeight;

				// Set the index and weight data in the vertex buffer
				mb.AddBoneData(vertexIndex, mappingIndex, weightValue);
			}
		}
	}
	

	mb.FinishBuilding();

	Mesh* mesh;
	
	// Only build with skinned vertices if bones are present, slight optimization
	if (skeleton != nullptr)
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

		if (skeleton != nullptr)
		{
			material->SetShader(AssetDB::CreateOrGetShader("Data/Shaders/Skinning.shader"));
		}
		else
		{
			material->SetShader(AssetDB::CreateOrGetShader("Phong_Opaque"));
		}

		Sampler* sampler = new Sampler();
		sampler->Initialize(SAMPLER_FILTER_LINEAR_MIPMAP_LINEAR, EDGE_SAMPLING_REPEAT);
		material->SetSampler(0, sampler);
		material->SetProperty("SPECULAR_AMOUNT", 0.3f);
		material->SetProperty("SPECULAR_POWER", 10.f);
	}

	// Add the draw!
	RenderableDraw_t draw;
	draw.sharedMaterial = material;
	draw.mesh = mesh;

    renderable->AddDraw(draw);
}


//-----------------------------------------------------------------------------------------------
// Constructs all Mesh and Material data from the given node
//
void AssimpLoader::ExtractBoneTransform(aiNode* ainode, const Matrix44& accumulatedTransform, int parentBoneIndex, SkeletonBase* skeleton)
{
	std::string nodeName = ainode->mName.data;

	Matrix44 thisNodeLocalTransform = ConvertAiMatrixToMyMatrix(ainode->mTransformation);
	Matrix44 thisNodeWorldTransform = accumulatedTransform * thisNodeLocalTransform;

	// Check if this name is a bone by searching for it's name mapping in the hierarchy
	int thisBoneIndex = skeleton->GetBoneMapping(nodeName);

	// If it is a bone, update the matrix in the skeleton (might be unnecessary for bind pose, as you get the same result!)
	if (thisBoneIndex >= 0)
	{
		Matrix44 offsetMatrix = skeleton->GetBoneData(thisBoneIndex).offsetMatrix;
		Matrix44 finalTransformation = thisNodeWorldTransform * skeleton->GetBoneData(thisBoneIndex).boneToMeshMatrix;

		skeleton->SetFinalTransformation(thisBoneIndex, finalTransformation);
		skeleton->SetWorldTransform(thisBoneIndex, thisNodeWorldTransform);
		skeleton->SetParentBoneIndex(thisBoneIndex, parentBoneIndex);

		skeleton->SetMeshToBoneMatrix(thisBoneIndex, offsetMatrix);
		skeleton->SetBoneToMeshMatrix(thisBoneIndex, Matrix44::GetInverse(offsetMatrix));
	}

	// Grab PreRotations
	int suffixStart = nodeName.find("_$AssimpFbx$_PreRotation");
	if (suffixStart != std::string::npos)
	{
		std::string baseName = std::string(nodeName, 0, suffixStart);
		int boneMapping = skeleton->GetBoneMapping(baseName);

		if (boneMapping >= 0)
		{
			skeleton->SetBonePreRotation(boneMapping, thisNodeLocalTransform);
		}
		else if (baseName == "BoneRoot") // Edge case - if fake root node exists
		{
			skeleton->SetBonePreRotation(0, thisNodeLocalTransform);
		}
	}

	// Determine what the parent index of the children bones are (either us if we're a bone, or our last bone ancestor)
	int childParentIndex = (thisBoneIndex >= 0 ? thisBoneIndex : parentBoneIndex);

	// Recursively process the child nodes for bone information (even if this one wasn't a bone)
	for (unsigned int nodeIndex = 0; nodeIndex < ainode->mNumChildren; ++nodeIndex)
	{
		ExtractBoneTransform(ainode->mChildren[nodeIndex], thisNodeWorldTransform, childParentIndex, skeleton);
	}
}


//-----------------------------------------------------------------------------------------------
// Builds all animations from the assimp tree and stores them on the loader
//
void AssimpLoader::BuildAnimations(SkeletonBase* skeleton, std::vector<AnimationClip*>& animations, int firstFrameIndex)
{
	unsigned int animationCount = m_scene->mNumAnimations;

	for (unsigned int animationIndex = 0; animationIndex < animationCount; ++animationIndex)
	{
		AnimationClip* clip = BuildAnimation(animationIndex, skeleton, firstFrameIndex);
		animations.push_back(clip);
	}
}


//-----------------------------------------------------------------------------------------------
// Builds a single animation at the given animation index
//
AnimationClip* AssimpLoader::BuildAnimation(unsigned int animationIndex, SkeletonBase* skeleton, int tickOffset)
{
	aiAnimation* aianimation = m_scene->mAnimations[animationIndex];

	// Get general animation data
	float numTicks = ((float) aianimation->mDuration) - (float) tickOffset;
	float durationSeconds = (float) numTicks / (float) aianimation->mTicksPerSecond;

	float framesPerSecond = 60.f;	// Hard coding for now
	float secondsPerFrame = 1.f / framesPerSecond;

	int numFramesToGenerate = Ceiling(durationSeconds * framesPerSecond);

	// Create the animation clip
	AnimationClip* animation = new AnimationClip();
	animation->Initialize(numFramesToGenerate, skeleton, framesPerSecond);
	animation->SetName(aianimation->mName.C_Str());

	// Iterate across the frames, and build a pose for each
	for (int frameIndex = 0; frameIndex < numFramesToGenerate; ++frameIndex)
	{
		Pose* pose = animation->GetPoseAtIndex(frameIndex);

		// Pass our time in number of ticks, since channels store times as number of ticks
		float time = (frameIndex * secondsPerFrame * (float) aianimation->mTicksPerSecond);
		FillPoseForTime(pose, aianimation, time, skeleton, tickOffset);
	}

	return animation;
}


//-----------------------------------------------------------------------------------------------
// Fills in the pose given for the animation at time time
//
void AssimpLoader::FillPoseForTime(Pose* out_pose, aiAnimation* aianimation, float time, SkeletonBase* skeleton, int tickOffset)
{
	// Iterate across all the bones in the pose
		// Find the channel corresponding to the bone
		// If the channel exists, make the transform and store it in the pose
		// Else store identity

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
			Matrix44 boneTransform = GetLocalTransfromAtTime(channel, time, skeleton->GetBoneData(boneDataIndex).preRotation, tickOffset);

			out_pose->SetBoneTransform(boneDataIndex, boneTransform);
		}
		else
		{
			Matrix44 boneTransform;
			bool channelsExist = ConstructMatrixFromSeparatedChannels(currBoneName, aianimation, time, skeleton, boneTransform, tickOffset);

			if (channelsExist)
			{
				out_pose->SetBoneTransform(boneDataIndex, boneTransform);
			}
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
Matrix44 AssimpLoader::GetLocalTransfromAtTime(aiNodeAnim* channel, float time, const Matrix44& preRotation, int tickOffset)
{
	// Assumes the start time for all nodes is mTime == 0

	// Translation
	aiVector3D position = GetWorldTranslationAtTime(channel, time, tickOffset);

	Vector3 myPosition = Vector3(position.x, position.y, position.z);

	// Rotation
	aiQuaternion rotation = GetWorldRotationAtTime(channel, time, tickOffset);

	Matrix44 rotationMat = preRotation * Matrix44::MakeRotation(ConvertAiQuaternionToMyQuaternion(rotation));

	// Scale
	aiVector3D scale = GetWorldScaleAtTime(channel, time, tickOffset);

	// Make the model
	Matrix44 translationMat = Matrix44::MakeTranslation(Vector3(position.x, position.y, position.z));
	Matrix44 scaleMat = Matrix44::MakeScale(Vector3(scale.x, scale.y, scale.z));
	Matrix44 transform = translationMat * rotationMat * scaleMat;

	return transform;
}


aiVector3D AssimpLoader::GetWorldTranslationAtTime(const aiNodeAnim* channel, float time, int firstFrameIndex) const
{
	if (firstFrameIndex >= channel->mNumPositionKeys)
	{
		return channel->mPositionKeys[0].mValue;
	}
	int positionKeyIndex = firstFrameIndex;
	aiVector3D finalPosition = channel->mPositionKeys[firstFrameIndex].mValue;
	float timeOffset = (float) channel->mPositionKeys[firstFrameIndex].mTime;

	// Only bother interpolating if there's more than one key
	if (channel->mNumPositionKeys > firstFrameIndex)
	{
		bool found = false;
		for (unsigned int translationKeyIndex = firstFrameIndex; translationKeyIndex < channel->mNumPositionKeys - 1; ++translationKeyIndex)
		{
			if (channel->mPositionKeys[translationKeyIndex + 1].mTime > time + timeOffset)
			{
				positionKeyIndex = translationKeyIndex;
				found = true;
				break;
			}
		}

		// Clamp to the end of the channel if our current time is pass the end
		if (!found)
		{
			return channel->mPositionKeys[channel->mNumPositionKeys - 1].mValue;
		}

		// Clamp to front
		if (channel->mPositionKeys[positionKeyIndex].mTime > time + timeOffset)
		{
			return finalPosition;
		}

		float firstTime = (float) channel->mPositionKeys[positionKeyIndex].mTime;
		float secondTime = (float) channel->mPositionKeys[positionKeyIndex + 1].mTime;

		float deltaTime = secondTime - firstTime;
		float interpolationFactor = (time + timeOffset - firstTime) / deltaTime;

		ASSERT_OR_DIE(interpolationFactor >= 0.f && interpolationFactor <= 1.0f, Stringf("Error: AssimpLoader::GetWorldTranslationAtTime calculated interpolation factor out of range, factor was %f", interpolationFactor));

		aiVector3D aifirstPosition = channel->mPositionKeys[positionKeyIndex].mValue;
		aiVector3D aisecondPosition = channel->mPositionKeys[positionKeyIndex + 1].mValue;

		Vector3 firstPosition = Vector3(aifirstPosition.x, aifirstPosition.y, aifirstPosition.z);
		Vector3 secondPosition = Vector3(aisecondPosition.x, aisecondPosition.y, aisecondPosition.z);
		
		Vector3 interpolation = Interpolate(firstPosition, secondPosition, interpolationFactor);
		finalPosition = aiVector3D(interpolation.x, interpolation.y, interpolation.z);
	}

	return finalPosition;
}

aiQuaternion AssimpLoader::GetWorldRotationAtTime(aiNodeAnim* channel, float time, int tickOffset) const
{
	if (tickOffset >= channel->mNumRotationKeys)
	{
		return channel->mRotationKeys[0].mValue;
	}

	int rotationKeyIndex = tickOffset;
	aiQuaternion aiFinalRotation = channel->mRotationKeys[tickOffset].mValue;
	float timeOffset = (float) channel->mRotationKeys[tickOffset].mTime;

	if (channel->mNumRotationKeys > tickOffset)
	{
		bool found = false;
		for (unsigned int index = tickOffset; index < channel->mNumRotationKeys - 1; ++index)
		{
			if (channel->mRotationKeys[index + 1].mTime > time + timeOffset)
			{
				rotationKeyIndex = index;
				found = true;
				break;
			}
		}

		// Clamp to front
		if (channel->mRotationKeys[rotationKeyIndex].mTime > time + timeOffset)
		{
			return channel->mRotationKeys[tickOffset].mValue;
		}

		// Clamp to end
		if (!found)
		{
			return channel->mRotationKeys[channel->mNumRotationKeys - 1].mValue;
		}
	


		float firstTime = (float) channel->mRotationKeys[rotationKeyIndex].mTime;
		float secondTime = (float) channel->mRotationKeys[rotationKeyIndex + 1].mTime;

		float deltaTime = secondTime - firstTime;
		float interpolationFactor = (time + timeOffset - firstTime) / deltaTime;

		ASSERT_OR_DIE(interpolationFactor >= 0.f && interpolationFactor <= 1.0f, Stringf("Error: AssimpLoader::GetWorldRotationAtTime calculated interpolation factor out of range, factor was %f", interpolationFactor));

		aiQuaternion aiFirstRotation = channel->mRotationKeys[rotationKeyIndex].mValue;
		aiQuaternion aiSecondRotation = channel->mRotationKeys[rotationKeyIndex + 1].mValue;

		aiQuaternion::Interpolate(aiFinalRotation, aiFirstRotation, aiSecondRotation, interpolationFactor);
	}

	aiFinalRotation.Normalize();
	return aiFinalRotation;
}

aiVector3D AssimpLoader::GetWorldScaleAtTime(aiNodeAnim* channel, float time, int tickOffset) const
{
	if (tickOffset >= channel->mNumScalingKeys)
	{
		return channel->mScalingKeys[0].mValue;
	}

	int firstKeyIndex = tickOffset;
	aiVector3D finalScale = channel->mScalingKeys[tickOffset].mValue;
	float timeOffset = (float) channel->mScalingKeys[tickOffset].mTime;

	// Only bother interpolating if there's more than one key
	if (channel->mNumScalingKeys > tickOffset)
	{
		bool found = false;
		for (unsigned int scaleKeyIndex = tickOffset; scaleKeyIndex < channel->mNumScalingKeys - 1; ++scaleKeyIndex)
		{
			if (channel->mScalingKeys[scaleKeyIndex + 1].mTime > time + timeOffset)
			{
				firstKeyIndex = scaleKeyIndex;
				found = true;
				break;
			}
		}

		// Clamp to the end of the channel if our current time is pass the end
		if (!found)
		{
			return channel->mScalingKeys[channel->mNumScalingKeys - 1].mValue;
		}

		// Clamp to front
		if (channel->mScalingKeys[firstKeyIndex].mTime > time + timeOffset)
		{
			return finalScale;
		}

		float firstTime = (float) channel->mScalingKeys[firstKeyIndex].mTime;
		float secondTime = (float) channel->mScalingKeys[firstKeyIndex + 1].mTime;

		float deltaTime = secondTime - firstTime;
		float interpolationFactor = (time + timeOffset - firstTime) / deltaTime;

		ASSERT_OR_DIE(interpolationFactor >= 0.f && interpolationFactor <= 1.0f, Stringf("Error: AssimpLoader::GetWorldScaleAtTime calculated interpolation factor out of range, factor was %f", interpolationFactor));

		aiVector3D aiFirstScale = channel->mScalingKeys[firstKeyIndex].mValue;
		aiVector3D aiSecondScale = channel->mScalingKeys[firstKeyIndex + 1].mValue;

		Vector3 firstScale = Vector3(aiFirstScale.x, aiFirstScale.y, aiFirstScale.z);
		Vector3 secondScale = Vector3(aiSecondScale.x, aiSecondScale.y, aiSecondScale.z);

		Vector3 interpolation = Interpolate(firstScale, secondScale, interpolationFactor);
		finalScale = aiVector3D(interpolation.x, interpolation.y, interpolation.z);
	}

	return finalScale;
}

bool AssimpLoader::ConstructMatrixFromSeparatedChannels(const std::string& boneName, aiAnimation* animation, float time, SkeletonBase* skeleton, Matrix44& out_transform, int firstFrameIndex) const
{
	bool channelFound = false;
	// Translation
	std::string translationChannelName = boneName + "_$AssimpFbx$_Translation";
	aiVector3D translation = aiVector3D(0.f, 0.f, 0.f);

	aiNodeAnim* translationChannel = GetChannelForBone(translationChannelName, animation);
	
	if (translationChannel != nullptr)
	{
		translation = GetWorldTranslationAtTime(translationChannel, time, firstFrameIndex);
		channelFound = true;
	}
	else 
	{
		Matrix44 localTransform = skeleton->GetBoneData(skeleton->GetBoneMapping(boneName)).localTransform;
		Vector3 positionOld = Matrix44::ExtractTranslation(localTransform);
		translation = aiVector3D(positionOld.x, positionOld.y, positionOld.z);
	}

	// Prerotation
	int boneMapping = skeleton->GetBoneMapping(boneName);
	BoneData_t boneData = skeleton->GetBoneData(boneMapping);
	Matrix44 preRotation = boneData.preRotation;

	// Rotation
	std::string rotationChannelName = boneName + "_$AssimpFbx$_Rotation";
	aiQuaternion rotation;

	aiNodeAnim* rotationChannel = GetChannelForBone(rotationChannelName, animation);

	if (rotationChannel != nullptr)
	{
		rotation = GetWorldRotationAtTime(rotationChannel, time, firstFrameIndex);
		channelFound = true;
	}

	Matrix44 rotationMat = ConvertAiMatrixToMyMatrix(aiMatrix4x4(aiVector3D(1.f, 1.f, 1.f), rotation, aiVector3D(0.f, 0.f, 0.f)));

	Matrix44 finalRotation = preRotation * rotationMat;

	// Scale
	std::string scaleChannelName = boneName + "_$AssimpFbx$_Scale";
	aiVector3D scale = aiVector3D(1.f, 1.f, 1.f);

	aiNodeAnim* scaleChannel = GetChannelForBone(scaleChannelName, animation);

	if (scaleChannel != nullptr)
	{
		scale = GetWorldScaleAtTime(scaleChannel, time, firstFrameIndex);
		channelFound = true;
	}
	else 
	{
		Matrix44 localTransform = skeleton->GetBoneData(skeleton->GetBoneMapping(boneName)).localTransform;
		Vector3 scaleOld = Matrix44::ExtractScale(localTransform);
		scale = aiVector3D(scaleOld.x, scaleOld.y, scaleOld.z);
	}

	Matrix44 scaleMat = Matrix44::MakeScale(Vector3(scale.x, scale.y, scale.z));
	Matrix44 translationMat = Matrix44::MakeTranslation(Vector3(translation.x, translation.y, translation.z));


	out_transform = translationMat * finalRotation * scaleMat;

	return channelFound;
}


//-----------------------------------------------------------------------------------------------
// Builds the skeleton transforms by traversing the assimp node hierarchy
//
void AssimpLoader::BuildBoneHierarchy(SkeletonBase* skeleton)
{
	// Recursively traverse the tree to assemble the bone transformations
	ExtractBoneTransform(m_scene->mRootNode, Matrix44::IDENTITY, -1, skeleton);
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

Quaternion ConvertAiQuaternionToMyQuaternion(aiQuaternion aiQuat)
{
	Quaternion result;

	result.s = aiQuat.w;
	result.v.x = aiQuat.x;
	result.v.y = aiQuat.y;
	result.v.z = aiQuat.z;

	return result;
}


void DebugPrintAnimation(aiAnimation* anim)
{
	DebuggerPrintf("---------------------ANIMATION NAME: %s -- NUMBER OF CHANNELS: %i\n", anim->mName.C_Str(), anim->mNumChannels);

	float tps = (float) anim->mTicksPerSecond;

	for (unsigned int channelIndex = 0; channelIndex < anim->mNumChannels; ++channelIndex)
	{
		aiNodeAnim* channel = anim->mChannels[channelIndex];

		aiString channelName = channel->mNodeName;

		int numPos = channel->mNumPositionKeys;
		int numRot = channel->mNumRotationKeys;
		int numSca = channel->mNumScalingKeys;

		DebuggerPrintf("CHANNEL %i: %s\n -- POSITIONS: %i -- POSITION TIME START: %f -- POSITION TIME END: %f\n -- ROTATIONS: %i -- ROTATION TIME START: %f -- ROTATION TIME END: %f\n -- SCALES: %i -- SCALE TIME START: %f -- SCALE TIME END: %f\n", 
			channelIndex, channel->mNodeName.C_Str(), numPos, channel->mPositionKeys[0].mTime / tps, channel->mPositionKeys[numPos - 1].mTime / tps, numRot, channel->mRotationKeys[0].mTime / tps, channel->mRotationKeys[numRot - 1].mTime / tps, numSca, channel->mScalingKeys[0].mTime / tps, channel->mScalingKeys[numSca - 1].mTime / tps);
	}
}

void DebugPrintAITree(aiNode* node, const std::string& indent)
{
	aiMatrix4x4 aiTransform = node->mTransformation;
	Matrix44 t = ConvertAiMatrixToMyMatrix(aiTransform);

	std::string text = "\n" + indent + "NODE: %s\n";
	DebuggerPrintf(text.c_str(), node->mName.C_Str());

	text = indent + "-----------------------------------------\n";
	DebuggerPrintf(text.c_str());
	text = "|%f %f %f %f|\n|%f %f %f %f|\n|%f %f %f %f|\n|%f %f %f %f|\n";

	DebuggerPrintf(text.c_str(), t.Ix, t.Jx, t.Kx, t.Tx, t.Iy, t.Jy, t.Ky, t.Ty, t.Iz, t.Jz, t.Kz, t.Tz, t.Iw, t.Jw, t.Kw, t.Tw);

	text = indent + "-----------------------------------------\n";
	DebuggerPrintf(text.c_str());

	for (unsigned int childIndex = 0; childIndex < node->mNumChildren; ++childIndex)
	{
		DebugPrintAITree(node->mChildren[childIndex], indent + "-");
		//DebuggerPrintf("-%s\n", node->mChildren[childIndex]->mName.C_Str());
	}
}
