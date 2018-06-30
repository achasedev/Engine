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
#include "Engine/Core/Time/ProfileScoped.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Rendering/Materials/Material.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
#include "Engine/Rendering/Animation/SkeletonBase.hpp"
#include "Engine/Rendering/Animation/AnimationClip.hpp"

#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

// Assimp


// C functions
Matrix44 ConvertAiMatrixToMyMatrix(aiMatrix4x4 aimatrix);
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
	ProfileScoped sp = ProfileScoped(Stringf("LoadFile: \"%s\"", filepath.c_str()));
	UNUSED(sp);

	Assimp::Importer importer;
	m_scene = importer.ReadFile(filepath.c_str(), aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_MakeLeftHanded);

	// Ensure the file loads
	if (m_scene == nullptr || m_scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || m_scene->mRootNode == nullptr)
	{
		ERROR_AND_DIE(Stringf("Error: AssimpLoader::LoadFile ran into error \"%s\" while loading file \"%s\"", importer.GetErrorString(), filepath.c_str()));
	}

	// Debugging
	DebugPrintAITree(m_scene->mRootNode, "");

	InitializeSkeleton();

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

const SkeletonBase* AssimpLoader::GetSkeletonBase() const
{
	return m_skeleton;
}

void AssimpLoader::InitializeSkeleton()
{
	// 1. Get all the bone names in the tree
	std::vector<std::string> boneNames;
	GetBoneNamesFromNode(m_scene->mRootNode, boneNames);

	// 2. Initialize the Skeleton
	m_skeleton = new SkeletonBase();
	aiMatrix4x4 inverseGlobal = m_scene->mRootNode->mTransformation;
	inverseGlobal.Inverse();
	m_skeleton->SetGlobalInverseTransform(ConvertAiMatrixToMyMatrix(inverseGlobal));

	CreateBoneMappingsFromNode(m_scene->mRootNode, boneNames);

	std::vector<std::string> names = m_skeleton->GetAllBoneNames();
	for (int i = 0; i < (int) names.size(); ++i)
	{
		unsigned int index = m_skeleton->GetBoneMapping(names[i]);

		if (index == 0)
		{
			ConsolePrintf("Root bone is %s", names[i].c_str());
			break;
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

void AssimpLoader::CreateBoneMappingsFromNode(aiNode* node, const std::vector<std::string>& boneNames)
{
	std::string nodeName = node->mName.C_Str();

	if (std::find(boneNames.begin(), boneNames.end(), nodeName) != boneNames.end())
	{
		m_skeleton->CreateOrGetBoneMapping(nodeName);
	}

	int numChildren = node->mNumChildren;

	for (int childIndex = 0; childIndex < numChildren; ++childIndex)
	{
		CreateBoneMappingsFromNode(node->mChildren[childIndex], boneNames);
	}
}

//-----------------------------------------------------------------------------------------------
// Returns the renderable created from the loaded file
//
void AssimpLoader::BuildMeshesAndMaterials_FromScene()
{
	m_renderable = new Renderable();
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

		Matrix44 toModelTransform = m_skeleton->GetGlobalInverseTransform() * transformation;

		normal = toModelTransform.TransformVector(normal).xyz();
		tangent = toModelTransform.TransformVector(tangent).xyz();
		position = toModelTransform.TransformPoint(position).xyz();

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

		unsigned int mappingIndex = m_skeleton->GetBoneMapping(boneName);

		ASSERT_OR_DIE(mappingIndex >= 0, Stringf("Error: Mesh built with a bone name without a registered slot."));

		m_skeleton->SetOffsetMatrix(mappingIndex, ConvertAiMatrixToMyMatrix(currBone->mOffsetMatrix));

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

		material->SetShader(AssetDB::GetShader("Vertex_Normal"));
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
void AssimpLoader::ExtractBoneTransform(aiNode* ainode, const Matrix44& parentTransform, int parentBoneIndex, const std::string& concatenations)
{
	std::string nodeName = ainode->mName.data;
	std::string nextConcatenations = concatenations + " | " + nodeName;
	//DebuggerPrintf("%s\n", nextConcatenations.c_str());

	Matrix44 nodeTransform = ConvertAiMatrixToMyMatrix(ainode->mTransformation);

	Matrix44 currentTransform = parentTransform * nodeTransform;

	// Check if this name is a bone by searching for it's name mapping in the hierarchy
	int thisBoneIndex = m_skeleton->GetBoneMapping(nodeName);

	// If it is a bone, update the matrix in the skeleton (might be unnecessary for bind pose, as you get the same result!)
	if (thisBoneIndex >= 0)
	{
		Matrix44 offset = m_skeleton->GetBoneData(thisBoneIndex).offsetMatrix;
		Matrix44 finalTransformation = m_skeleton->GetGlobalInverseTransform() * currentTransform * m_skeleton->GetBoneData(thisBoneIndex).offsetMatrix;

		m_skeleton->SetFinalTransformation(thisBoneIndex, finalTransformation);
		m_skeleton->SetWorldTransform(thisBoneIndex, currentTransform);
		m_skeleton->SetParentBoneIndex(thisBoneIndex, parentBoneIndex);
		m_skeleton->SetLocalTransform(thisBoneIndex, nodeTransform);

		offset.Invert();
		m_skeleton->SetBindPose(thisBoneIndex, m_skeleton->GetRootBoneOffset() * offset);
	}
	
	// Catch the bone root offset when we find it
	DebuggerPrintf("NODE NAME: %s\n", nodeName.c_str());

	if (nodeName == "BoneRoot" || nodeName == m_skeleton->GetRootBoneName() + "_$AssimpFbx$_PreRotation")
	{
		m_skeleton->SetRootBoneOffset(currentTransform);
	}

	// Determine what the parent index of the children bones are (either us if we're a bone, or our last bone ancestor)
	int childParentIndex = (thisBoneIndex >= 0 ? thisBoneIndex : parentBoneIndex);

	// Recursively process the child nodes for bone information (even if this one wasn't a bone)
	for (unsigned int nodeIndex = 0; nodeIndex < ainode->mNumChildren; ++nodeIndex)
	{
		ExtractBoneTransform(ainode->mChildren[nodeIndex], currentTransform, childParentIndex, nextConcatenations);
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
		for (int i = 0; i < m_scene->mAnimations[0]->mNumChannels; ++i)
		{
			DebuggerPrintf("Channel: %s\n", m_scene->mAnimations[0]->mChannels[i]->mNodeName.C_Str());
		}

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
	animation->Initialize(numFrames, m_skeleton);
	animation->SetName(aianimation->mName.C_Str());
	animation->SetFramesPerSecond(framesPerSecond);
	animation->SetDurationSeconds(durationSeconds);

	// Iterate across the frames, and build a pose for each
	for (int frameIndex = 0; frameIndex < numFrames; ++frameIndex)
	{
		Pose* pose = animation->GetPoseAtIndex(frameIndex);

		// Pass our time in number of ticks, since channels store times as number of ticks
		float time = frameIndex * secondsPerFrame * (float) aianimation->mTicksPerSecond;
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

	std::vector<std::string> boneNames = m_skeleton->GetAllBoneNames();
	out_pose->Initialize(m_skeleton);
	
	
	int numBones = (int) boneNames.size();

	for (int boneNameIndex = 0; boneNameIndex < numBones; ++boneNameIndex)
	{
		std::string currBoneName = boneNames[boneNameIndex];
		int boneDataIndex = m_skeleton->GetBoneMapping(currBoneName);

		aiNodeAnim* channel = GetChannelForBone(currBoneName, aianimation);

		if (channel != nullptr)
		{
			Matrix44 boneTransform = GetLocalTransfromAtTime(channel, time);

			out_pose->SetBoneTransform(boneDataIndex, boneTransform);
		}
		else
		{
			Matrix44 boneTransform = ConstructMatrixFromSeparatedChannels(currBoneName, aianimation, time);

			out_pose->SetBoneTransform(boneDataIndex, boneTransform);
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
	aiVector3D position = GetWorldTranslationAtTime(channel, time);

	// Rotation
	aiQuaternion rotation = GetWorldRotationAtTime(channel, time);

	// Scale
	aiVector3D scale = GetWorldScaleAtTime(channel, time);

	// Make the model
	aiMatrix4x4 aiTransform = aiMatrix4x4(scale, rotation, position);

	Matrix44 transform = ConvertAiMatrixToMyMatrix(aiTransform);

	return transform;
}


aiVector3D AssimpLoader::GetWorldTranslationAtTime(aiNodeAnim* channel, float time) const
{
	int positionKeyIndex = 0;
	aiVector3D finalPosition = channel->mPositionKeys[0].mValue;

	// Only bother interpolating if there's more than one key
	if (channel->mNumPositionKeys > 1)
	{
		bool found = false;
		for (unsigned int translationKeyIndex = 0; translationKeyIndex < channel->mNumPositionKeys - 1; ++translationKeyIndex)
		{
			if (channel->mPositionKeys[translationKeyIndex + 1].mTime > time)
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
		if (channel->mPositionKeys[positionKeyIndex].mTime > time)
		{
			return finalPosition;
		}

		float firstTime = (float) channel->mPositionKeys[positionKeyIndex].mTime;
		float secondTime = (float) channel->mPositionKeys[positionKeyIndex + 1].mTime;

		float deltaTime = secondTime - firstTime;
		float interpolationFactor = (time - firstTime) / deltaTime;

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

aiQuaternion AssimpLoader::GetWorldRotationAtTime(aiNodeAnim* channel, float time) const
{
	int rotationKeyIndex = 0;
	aiQuaternion aiFinalRotation = channel->mRotationKeys[0].mValue;

	if (channel->mNumRotationKeys > 1)
	{
		bool found = false;
		for (unsigned int index = 0; index < channel->mNumRotationKeys - 1; ++index)
		{
			if (channel->mRotationKeys[index + 1].mTime > time)
			{
				rotationKeyIndex = index;
				found = true;
				break;
			}
		}

		// Clamp to front
		if (channel->mRotationKeys[rotationKeyIndex].mTime > time)
		{
			return channel->mRotationKeys[0].mValue;
		}

		// Clamp to end
		if (!found)
		{
			return channel->mRotationKeys[channel->mNumRotationKeys - 1].mValue;
		}
	


		float firstTime = (float) channel->mRotationKeys[rotationKeyIndex].mTime;
		float secondTime = (float) channel->mRotationKeys[rotationKeyIndex + 1].mTime;

		float deltaTime = secondTime - firstTime;
		float interpolationFactor = (time - firstTime) / deltaTime;

		ASSERT_OR_DIE(interpolationFactor >= 0.f && interpolationFactor <= 1.0f, Stringf("Error: AssimpLoader::GetWorldRotationAtTime calculated interpolation factor out of range, factor was %f", interpolationFactor));

		aiQuaternion aiFirstRotation = channel->mRotationKeys[rotationKeyIndex].mValue;
		aiQuaternion aiSecondRotation = channel->mRotationKeys[rotationKeyIndex + 1].mValue;

		aiQuaternion::Interpolate(aiFinalRotation, aiFirstRotation, aiSecondRotation, interpolationFactor);
	}

	aiFinalRotation.Normalize();
	return aiFinalRotation;
}

aiVector3D AssimpLoader::GetWorldScaleAtTime(aiNodeAnim* channel, float time) const
{
	int firstKeyIndex = 0;
	aiVector3D finalScale = channel->mScalingKeys[0].mValue;

	// Only bother interpolating if there's more than one key
	if (channel->mNumScalingKeys > 1)
	{
		bool found = false;
		for (unsigned int scaleKeyIndex = 0; scaleKeyIndex < channel->mNumScalingKeys - 1; ++scaleKeyIndex)
		{
			if (channel->mScalingKeys[scaleKeyIndex + 1].mTime > time)
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
		if (channel->mScalingKeys[firstKeyIndex].mTime > time)
		{
			return finalScale;
		}

		float firstTime = (float) channel->mScalingKeys[firstKeyIndex].mTime;
		float secondTime = (float) channel->mScalingKeys[firstKeyIndex + 1].mTime;

		float deltaTime = secondTime - firstTime;
		float interpolationFactor = (time - firstTime) / deltaTime;

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

Matrix44 AssimpLoader::ConstructMatrixFromSeparatedChannels(const std::string& boneName, aiAnimation* animation, float time) const
{
	// Translation
	std::string translationChannelName = boneName + "_$AssimpFbx$_Translation";
	aiVector3D translation = aiVector3D(0.f, 0.f, 0.f);

	aiNodeAnim* translationChannel = GetChannelForBone(translationChannelName, animation);
	
	if (translationChannel != nullptr)
	{
		translation = GetWorldTranslationAtTime(translationChannel, time);
	}

	// Rotation
	std::string rotationChannelName = boneName + "_$AssimpFbx$_Rotation";
	aiQuaternion rotation;

	aiNodeAnim* rotationChannel = GetChannelForBone(rotationChannelName, animation);

	if (rotationChannel != nullptr)
	{
		rotation = GetWorldRotationAtTime(rotationChannel, time);
	}

	// Scale
	std::string scaleChannelName = boneName + "_$AssimpFbx$_Scale";
	aiVector3D scale = aiVector3D(1.f, 1.f, 1.f);

	aiNodeAnim* scaleChannel = GetChannelForBone(scaleChannelName, animation);

	if (scaleChannel != nullptr)
	{
		scale = GetWorldScaleAtTime(scaleChannel, time);
	}

	aiMatrix4x4 aiTransform = aiMatrix4x4(scale, rotation, translation);
	Matrix44 transform = ConvertAiMatrixToMyMatrix(aiTransform);

	return transform;
}


//-----------------------------------------------------------------------------------------------
// Builds the skeleton transforms by traversing the assimp node hierarchy
//
void AssimpLoader::BuildBoneHierarchy()
{
	// Recursively traverse the tree to assemble the bone transformations
	ExtractBoneTransform(m_scene->mRootNode, Matrix44::IDENTITY, -1, "");
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
