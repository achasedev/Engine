/************************************************************************/
/* File: AssimpLoader.cpp
/* Author: Andrew Chase
/* Date: June 13th, 2018
/* Description: Implementation of the AssimpLoader class
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
#include "Engine/Rendering/Animation/Skeleton.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
#include "Engine/Rendering/Animation/AnimationClip.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

// Assimp importer, so we don't need to pass it between open/close files
Assimp::Importer g_importer;

// C utility functions
std::vector<Texture*>	LoadAssimpMaterialTextures(aiMaterial* aimaterial, aiTextureType type);
Matrix44				GetNodeWorldTransform(aiNode* node);
Matrix44				ConvertAiMatrixToMyMatrix(aiMatrix4x4 aimatrix);
Quaternion				ConvertAiQuaternionToMyQuaternion(aiQuaternion aiQuat);
void					DebugPrintAnimation(aiAnimation* anim);
void					DebugPrintAITree(aiNode* node, const std::string& indent);



/////////////////////////////////////////////////////////////////////////////////////////////////
// File I/O
/////////////////////////////////////////////////////////////////////////////////////////////////



//-----------------------------------------------------------------------------------------------
// Opens the file specified by filepath, and reads the contents to assemble the Assimp tree
//
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


//-----------------------------------------------------------------------------------------------
// Frees the Assimp scene
//
void AssimpLoader::CloseFile()
{
	g_importer.FreeScene();
	m_scene = nullptr;
}



/////////////////////////////////////////////////////////////////////////////////////////////////
// Importing
/////////////////////////////////////////////////////////////////////////////////////////////////



//-----------------------------------------------------------------------------------------------
// Traverses the Assimp tree and builds the skeleton from the bone data
//
Skeleton* AssimpLoader::ImportSkeleton()
{
	Skeleton* skeleton = new Skeleton();
	InitializeSkeleton(skeleton);

	return skeleton;
}


//-----------------------------------------------------------------------------------------------
// Traverses the Assimp tree and builds the meshes from the data
// If a skeleton is specified, the bone data stored in the mesh is extracted and used with the skeleton
// so that the mesh can be skinned to it. Otherwise it just loads the mesh as static meshes.
//
Renderable* AssimpLoader::ImportMesh(Skeleton* skeleton /*= nullptr*/)
{
	Renderable* renderable = new Renderable();
	BuildMeshesAndMaterials_FromScene(renderable, skeleton);

	return renderable;
}


//-----------------------------------------------------------------------------------------------
// Traverses the Assimp root node to assemble animations
// If a tick offset is specified, it starts creating all animations starting from the offset - is
// used for avoiding bind poses set in the animation's first frame, which occurs if the loaded file
// is an animation file without meshes or a skeleton, and instead stores the bind pose there.
//
std::vector<AnimationClip*> AssimpLoader::ImportAnimation(Skeleton* skeleton, int tickOffset /*= 0*/)
{
	std::vector<AnimationClip*> animations;
	BuildAnimations(skeleton, animations, tickOffset);

	return animations;
}



/////////////////////////////////////////////////////////////////////////////////////////////////
// Skeleton
/////////////////////////////////////////////////////////////////////////////////////////////////



//-----------------------------------------------------------------------------------------------
// Initializes the skeleton given the data from the currently loaded scene
//
void AssimpLoader::InitializeSkeleton(Skeleton* skeleton)
{
	// 1. Get all the bone names in the tree, done by recursively walking the tree
	// and pulling out all bone names from the meshes.
	std::vector<std::string> boneNames;
	GetBoneNamesFromNode(m_scene->mRootNode, boneNames);

	// 2. Create all the mappings for the bones in the correct order, with the root bone
	// being at index 0 (parents come before children)
	CreateBoneMappingsFromNode(m_scene->mRootNode, boneNames, skeleton);

	// 3. Traverse all meshes in the scene and pull out the offset matrices from the bone structs within them
	SetBoneOffsetData(m_scene->mRootNode, skeleton);

	// 4. Set the bone data for the bones in the skeleton, including all used transform concatenations
	BuildBoneHierarchy(skeleton);

	// 5. Construct local matrices - since Assimp seperates node transforms into seperate translation, rotation, and scale
	// nodes, building these with recursion requires tracking traversal state, and is more easily faulty, so I build them here
	// from world matrices
	BuildLocalMatrices(skeleton);
}


//-----------------------------------------------------------------------------------------------
// Traverses the Assimp scene to pull all bone names from the meshes' bone structs, 
// and stores them uniquely in the out_names parameter.
// 
void AssimpLoader::GetBoneNamesFromNode(aiNode* node, std::vector<std::string>& out_names)
{
	// Iterate across all meshes in the scene
	for (unsigned int meshIndex = 0; meshIndex < m_scene->mNumMeshes; ++meshIndex)
	{
		aiMesh* currMesh = m_scene->mMeshes[meshIndex];

		// For each mesh, iterate across all bones affected by it
		for (unsigned int boneIndex = 0; boneIndex < currMesh->mNumBones; ++boneIndex)
		{
			aiBone* currBone = currMesh->mBones[boneIndex];
			std::string boneName = std::string(currBone->mName.C_Str());
		
			// If we don't already have the name in the list, store the name
			if (std::find(out_names.begin(), out_names.end(), boneName) == out_names.end())
			{
				out_names.push_back(boneName);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Creates the ordered bone mappings for the bones in the given boneNames list
// Essentially we want to assign an index for each bone to map into, but in an order where the
// parent bone always comes before it's children, for calculating to-world matrices. As a result, we
// Walk the tree to find the order they appear in, creating unique mappings for each as we go
// 
void AssimpLoader::CreateBoneMappingsFromNode(aiNode* node, const std::vector<std::string>& boneNames, Skeleton* skeleton)
{
	// If the name of this node is a bone name, create or get the mapping
	std::string nodeName = node->mName.C_Str();
	if (std::find(boneNames.begin(), boneNames.end(), nodeName) != boneNames.end())
	{
		skeleton->CreateOrGetBoneMapping(nodeName); // Creates a mapping if one doesn't exist, otherwise returns the existing one
	}

	// Recursively call on all children of this node
	int numChildren = node->mNumChildren;

	for (int childIndex = 0; childIndex < numChildren; ++childIndex)
	{
		CreateBoneMappingsFromNode(node->mChildren[childIndex], boneNames, skeleton);
	}
}


//-----------------------------------------------------------------------------------------------
// Recursively gets all offset matrices (inverse bind pose, transforms from mesh space to bone space)
// which are stored in the mesh objects. These matrices require the node transform that the mesh is defined
// in, which requires us to walk the tree as opposed to just walking the scene's list of meshes
// 
// 
void AssimpLoader::SetBoneOffsetData(aiNode* node, Skeleton* skeleton)
{
	// Iterate across meshes in this node
	int numMeshes = (int) node->mNumMeshes;

	for (int meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
	{
		aiMesh* currMesh = m_scene->mMeshes[node->mMeshes[meshIndex]];

		// Iterate across the bones in the mesh
		int numBones = currMesh->mNumBones;

		for (int boneIndex = 0; boneIndex < numBones; ++boneIndex)
		{
			aiBone* currBone = currMesh->mBones[boneIndex];
			int boneMapping = skeleton->GetBoneMapping(currBone->mName.C_Str());

			// Safety check - ensure our mapping is in order
			ASSERT_OR_DIE(boneMapping >= 0, Stringf("Error: AssimpLoader::SetBoneOffsetData couldn't find mapping for bone \"%s\"", currBone->mName.C_Str()));

			Matrix44 nodeTransform = GetNodeWorldTransform(node);					// Why we need to recursively walk the tree
			Matrix44 offset = ConvertAiMatrixToMyMatrix(currBone->mOffsetMatrix);	// Inverse bind pose

			Matrix44 finalWorldToBone = offset * Matrix44::GetInverse(nodeTransform);
			skeleton->SetOffsetMatrix(boneMapping, finalWorldToBone);
		}
	}

	// Recursively call on all children
	int numChildren = node->mNumChildren;
	for (int childIndex = 0; childIndex < numChildren; ++childIndex)
	{
		SetBoneOffsetData(node->mChildren[childIndex], skeleton);
	}
}


//-----------------------------------------------------------------------------------------------
// Builds the skeleton transforms that are used to define skeletal structure
// It recursively traverses the Assimp scene tree to concatenate node transforms, initializing
// the skeleton's bone transforms as it comes across them
//
void AssimpLoader::BuildBoneHierarchy(Skeleton* skeleton)
{
	// Start with the root
	//						Node					Parent Transform		Parent bone index/mapping	Skeleton
	ExtractBoneTransform(	m_scene->mRootNode,		Matrix44::IDENTITY,		-1,							skeleton);
}


//-----------------------------------------------------------------------------------------------
// Constructs the bone transforms from this node if this node is a bone in the skeleton
//
void AssimpLoader::ExtractBoneTransform(aiNode* ainode, const Matrix44& accumulatedTransform, int parentBoneIndex, Skeleton* skeleton)
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

		skeleton->SetWorldTransform(thisBoneIndex, thisNodeWorldTransform);
		skeleton->SetParentBoneIndex(thisBoneIndex, parentBoneIndex);

		skeleton->SetMeshToBoneMatrix(thisBoneIndex, offsetMatrix);
		skeleton->SetBoneToMeshMatrix(thisBoneIndex, Matrix44::GetInverse(offsetMatrix));
	}

	// Grab PreRotations, since animations don't have pre-rotation channels and need these prepended
	int suffixStart = nodeName.find("_$AssimpFbx$_PreRotation"); // Name format is <BONENAME>_$AssimpFbx$_PreRotation
	if (suffixStart != std::string::npos)
	{
		std::string baseName = std::string(nodeName, 0, suffixStart);
		int boneMapping = skeleton->GetBoneMapping(baseName);

		if (boneMapping >= 0)
		{
			skeleton->SetBonePreRotation(boneMapping, thisNodeLocalTransform);
		}
		else if (baseName == "BoneRoot") // Edge case - if "fake" root node exists, only occurs in one model of mine, where a node called "RootNode" exists above the real root node
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
// Builds all local matrices for the skeleton, given the already-set world matrices of each bone
//
void AssimpLoader::BuildLocalMatrices(Skeleton* skeleton)
{
	// For each bone
	for (int i = 0; i < skeleton->GetBoneCount(); ++i)
	{
		BoneData_t currBone = skeleton->GetBoneData(i);
		Matrix44 oldLocal = currBone.localTransform;
		int parentIndex = currBone.parentIndex;

		// If we have a parent (i.e. we aren't the root)
		if (parentIndex >= 0)
		{
			// Get the parent transform
			BoneData_t parentBone = skeleton->GetBoneData(parentIndex);
			Matrix44 parentWorldInverse = parentBone.worldTransform;
			parentWorldInverse.Invert();

			// Multiply our parent's inverse with our world to get our local
			Matrix44 localTransform = parentWorldInverse * currBone.worldTransform;

			// Set it
			skeleton->SetLocalTransform(i, localTransform);
		}
		else
		{
			// Root has it's local matrix be the world matrix
			skeleton->SetLocalTransform(i, currBone.worldTransform);
		}
	}
}



/////////////////////////////////////////////////////////////////////////////////////////////////
// Meshes and Materials
/////////////////////////////////////////////////////////////////////////////////////////////////



//-----------------------------------------------------------------------------------------------
// Initializes the renderable given with the mesh and material data in the current scene
// The skeleton is passed in case we want to skin the meshes, so we have a skeleton to reference
//
void AssimpLoader::BuildMeshesAndMaterials_FromScene(Renderable* renderable, Skeleton* skeleton)
{
	// Done recursively from the root
	//									Node					Parent Transform		Renderable		Skeleton
	BuildMeshesAndMaterials_FromNode(	m_scene->mRootNode,		Matrix44::IDENTITY,		renderable,		skeleton);
}


//-----------------------------------------------------------------------------------------------
// Builds all meshes and materials from the data on the given node
//
void AssimpLoader::BuildMeshesAndMaterials_FromNode(aiNode* node, const Matrix44& parentTransform, Renderable* renderable, Skeleton* skeleton)
{
	// Get the node transform
	 Matrix44 currTransform = ConvertAiMatrixToMyMatrix(node->mTransformation);
	 currTransform = parentTransform * currTransform;

	 // Build a mesh from each aiMesh struct used by this node
	 int numMeshes = (int) node->mNumMeshes;

	 for (int meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
	 {
		 BuildMeshAndMaterials_FromAIMesh(m_scene->mMeshes[node->mMeshes[meshIndex]], currTransform, renderable, skeleton);
	 }

	 // Recursively call on child nodes
	 int numChildren = node->mNumChildren;
	 for (int childIndex = 0; childIndex < numChildren; ++childIndex)
	 {
		 BuildMeshesAndMaterials_FromNode(node->mChildren[childIndex], currTransform, renderable, skeleton);
	 }
}


//-----------------------------------------------------------------------------------------------
// Constructs the mesh and material data from the given aiMesh structure
// The transformation passed is the space the current mesh exists in, and is used to convert
// all mesh vertices into "model/world" space
//
void AssimpLoader::BuildMeshAndMaterials_FromAIMesh(aiMesh* aimesh, const Matrix44& transformation, Renderable* renderable, Skeleton* skeleton)
{
	//-----Build the mesh from this aiMesh-----

	MeshBuilder mb;
	mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);

	// Iterate across vertices
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

		// Transform the vertices from the local mesh space to world space
		normal = transformation.TransformVector(normal).xyz();
		tangent = transformation.TransformVector(tangent).xyz();
		position = transformation.TransformPoint(position).xyz();

		// Renormalize
		normal.NormalizeAndGetLength();
		tangent.NormalizeAndGetLength();

		// Set the MeshBuilder state
		mb.SetNormal(normal);
		mb.SetTangent(Vector4(tangent, 1.0f));
		mb.SetUVs(uvs);

		// Push the vertex into the MeshBuilder
		mb.PushVertex(position);
	}

	// Mesh indices, for faces
	for (unsigned int i = 0; i < aimesh->mNumFaces; ++i)
	{
		aiFace face = aimesh->mFaces[i];

		if (face.mNumIndices != 3) { continue; } // Only support triangles!

		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			mb.PushIndex(face.mIndices[j]);
		}
	}

	// Bone Vertex Information - only if we're using a skeleton
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
	
	// Only build with skinned vertices if bones are present
	if (skeleton != nullptr)
	{
		mesh = mb.CreateMesh<VertexSkinned>();
	}
	else
	{
		mesh = mb.CreateMesh<VertexLit>();
	}


	//-----Build the material for this mesh-----

	Material* material = AssetDB::GetSharedMaterial("Default_Opaque");
	if (aimesh->mMaterialIndex >= 0)
	{
		aiMaterial* aimaterial = m_scene->mMaterials[aimesh->mMaterialIndex];
		std::vector<Texture*> diffuse, normal;

		diffuse		= LoadAssimpMaterialTextures(aimaterial,	aiTextureType_DIFFUSE);
		normal		= LoadAssimpMaterialTextures(aimaterial,	aiTextureType_NORMALS);

		// Make the material, defaulting missing textures to built-in engine textures
		material = new Material();
		if (diffuse.size() > 0)
		{
			material->SetDiffuse(diffuse[0]); // Only pull the first texture
		}
		else
		{
			material->SetDiffuse(AssetDB::GetTexture("Default"));
		}

		if (normal.size() > 0)
		{
			material->SetNormal(normal[0]); // Only pull the first texture
		}
		else
		{
			material->SetNormal(AssetDB::GetTexture("Flat"));
		}


		// If we have a skeleton, then use a skinning shader
		if (skeleton != nullptr)
		{
			material->SetShader(AssetDB::CreateOrGetShader("Data/Shaders/Skinning.shader"));
		}
		else
		{
			material->SetShader(AssetDB::CreateOrGetShader("Phong_Opaque"));
		}

		// Set up a linear sampler for looks
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



/////////////////////////////////////////////////////////////////////////////////////////////////
// Animation
/////////////////////////////////////////////////////////////////////////////////////////////////



//-----------------------------------------------------------------------------------------------
// Builds all animations from the Assimp tree and stores them in the vector provided
//
void AssimpLoader::BuildAnimations(Skeleton* skeleton, std::vector<AnimationClip*>& animations, int firstFrameIndex)
{
	unsigned int animationCount = m_scene->mNumAnimations;

	for (unsigned int animationIndex = 0; animationIndex < animationCount; ++animationIndex)
	{
		AnimationClip* clip = BuildAnimation(animationIndex, skeleton, firstFrameIndex);
		animations.push_back(clip);
	}
}


//-----------------------------------------------------------------------------------------------
// Builds a single animation, from the aiAnimation struct at the given index
//
AnimationClip* AssimpLoader::BuildAnimation(unsigned int animationIndex, Skeleton* skeleton, int tickOffset)
{
	aiAnimation* aianimation = m_scene->mAnimations[animationIndex];

	// Get general animation data
	float numTicks = ((float) aianimation->mDuration) - (float) tickOffset;
	float durationSeconds = (float) numTicks / (float) aianimation->mTicksPerSecond;

	float framesPerSecond = 30.f;	// Import at 30 fps
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
void AssimpLoader::FillPoseForTime(Pose* out_pose, aiAnimation* aianimation, float time, Skeleton* skeleton, int tickOffset)
{
	// Initialize the pose
	std::vector<std::string> boneNames = skeleton->GetAllBoneNames();
	out_pose->Initialize(skeleton);
	
	// Iterate across all bones
	int numBones = (int) boneNames.size();

	for (int boneNameIndex = 0; boneNameIndex < numBones; ++boneNameIndex)
	{
		std::string currBoneName = boneNames[boneNameIndex];
		int boneDataIndex = skeleton->GetBoneMapping(currBoneName);

		// Get the channel for the bone
		aiNodeAnim* channel = GetChannelForBone(currBoneName, aianimation);

		// If the channel exists, get the transform at the current time
		if (channel != nullptr)
		{
			Matrix44 boneTransform = GetLocalTransfromAtTime(channel, time, skeleton->GetBoneData(boneDataIndex).preRotation, tickOffset);
			out_pose->SetBoneTransform(boneDataIndex, boneTransform);
		}
		else
		{
			// Assimp may have separated the animation channel for this bone into 3 separate channels, so we look for them
			Matrix44 boneTransform;
			bool channelsExist = ConstructTransformFromSeparatedChannels(currBoneName, aianimation, time, skeleton, boneTransform, tickOffset);

			// If they existed as separate channels, then set the transform
			// Otherwise leave the transform alone, which was already initialized to the bind pose bone's local transform
			if (channelsExist)
			{
				out_pose->SetBoneTransform(boneDataIndex, boneTransform);
			}
		}
	}

	// All the pose matrices are in their local space, so need to concatenate parents in order to finish it, as well as apply offsets matrices and the global inverse
	out_pose->ConstructWorldMatrices();
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
	aiVector3D position = GetAnimationTranslationAtTime(channel, time, tickOffset);

	Vector3 myPosition = Vector3(position.x, position.y, position.z);

	// Rotation
	aiQuaternion rotation = GetAnimationRotationAtTime(channel, time, tickOffset);

	Matrix44 rotationMat = preRotation * Matrix44::MakeRotation(ConvertAiQuaternionToMyQuaternion(rotation)); // Append prerotation

	// Scale
	aiVector3D scale = GetAnimationScaleAtTime(channel, time, tickOffset);

	// Make the model
	Matrix44 translationMat = Matrix44::MakeTranslation(Vector3(position.x, position.y, position.z));
	Matrix44 scaleMat = Matrix44::MakeScale(Vector3(scale.x, scale.y, scale.z));
	Matrix44 transform = translationMat * rotationMat * scaleMat;

	return transform;
}


//-----------------------------------------------------------------------------------------------
// Gets the translation of the channel at the given time, accounting for the offset
//
aiVector3D AssimpLoader::GetAnimationTranslationAtTime(const aiNodeAnim* channel, float time, int firstFrameIndex) const
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


//-----------------------------------------------------------------------------------------------
// Gets the rotation of the channel at the given time, accounting for the offset
//
aiQuaternion AssimpLoader::GetAnimationRotationAtTime(aiNodeAnim* channel, float time, int tickOffset) const
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


//-----------------------------------------------------------------------------------------------
// Gets the scale of the channel at the given time, accounting for the offset
//
aiVector3D AssimpLoader::GetAnimationScaleAtTime(aiNodeAnim* channel, float time, int tickOffset) const
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


//-----------------------------------------------------------------------------------------------
// Constructs the animation for the given bone name from 3 separate channels:
//		boneName__$AssimpFbx$_Translation
//		boneName__$AssimpFbx$_Rotation
//		boneName__$AssimpFbx$_Scale
// It's not guarenteed that all three (or any at all) exist
// Returns true if at least one existed
//
bool AssimpLoader::ConstructTransformFromSeparatedChannels(const std::string& boneName, aiAnimation* animation, float time, Skeleton* skeleton, Matrix44& out_transform, int firstFrameIndex) const
{
	bool channelFound = false;

	// Translation
	std::string translationChannelName = boneName + "_$AssimpFbx$_Translation";
	aiVector3D translation = aiVector3D(0.f, 0.f, 0.f);

	aiNodeAnim* translationChannel = GetChannelForBone(translationChannelName, animation);
	
	if (translationChannel != nullptr)
	{
		translation = GetAnimationTranslationAtTime(translationChannel, time, firstFrameIndex);
		channelFound = true;
	}
	else 
	{
		Matrix44 localTransform = skeleton->GetBoneData(skeleton->GetBoneMapping(boneName)).localTransform;
		Vector3 positionOld = Matrix44::ExtractTranslation(localTransform);
		translation = aiVector3D(positionOld.x, positionOld.y, positionOld.z);
	}

	// Prerotation - from skeleton
	int boneMapping = skeleton->GetBoneMapping(boneName);
	BoneData_t boneData = skeleton->GetBoneData(boneMapping);
	Matrix44 preRotation = boneData.preRotation;

	// Rotation
	std::string rotationChannelName = boneName + "_$AssimpFbx$_Rotation";
	aiQuaternion rotation;

	aiNodeAnim* rotationChannel = GetChannelForBone(rotationChannelName, animation);

	if (rotationChannel != nullptr)
	{
		rotation = GetAnimationRotationAtTime(rotationChannel, time, firstFrameIndex);
		channelFound = true;
	}

	// Prepend prerotation
	Matrix44 rotationMat = ConvertAiMatrixToMyMatrix(aiMatrix4x4(aiVector3D(1.f, 1.f, 1.f), rotation, aiVector3D(0.f, 0.f, 0.f)));
	Matrix44 finalRotation = preRotation * rotationMat;

	// Scale
	std::string scaleChannelName = boneName + "_$AssimpFbx$_Scale";
	aiVector3D scale = aiVector3D(1.f, 1.f, 1.f);

	aiNodeAnim* scaleChannel = GetChannelForBone(scaleChannelName, animation);

	if (scaleChannel != nullptr)
	{
		scale = GetAnimationScaleAtTime(scaleChannel, time, firstFrameIndex);
		channelFound = true;
	}
	else 
	{
		Matrix44 localTransform = skeleton->GetBoneData(skeleton->GetBoneMapping(boneName)).localTransform;
		Vector3 scaleOld = Matrix44::ExtractScale(localTransform);
		scale = aiVector3D(scaleOld.x, scaleOld.y, scaleOld.z);
	}

	Matrix44 scaleMat = Matrix44::MakeScale(Vector3(scale.x, scale.y, scale.z));

	// Final transform
	Matrix44 translationMat = Matrix44::MakeTranslation(Vector3(translation.x, translation.y, translation.z));
	out_transform = translationMat * finalRotation * scaleMat;

	return channelFound;
}



//////////////////////////////////////////////////////////////////////////
// C Functions, for utility/debugging
//////////////////////////////////////////////////////////////////////////



//-----------------------------------------------------------------------------------------------
// Find the textures used by the given aiMaterial and attempts to load and return them
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


//-----------------------------------------------------------------------------------------------
// Returns the world transform of the node given
//
Matrix44 GetNodeWorldTransform(aiNode* node)
{
	if (node == nullptr)
	{
		return Matrix44::IDENTITY;
	}

	Matrix44 parentTransform = GetNodeWorldTransform(node->mParent);

	return parentTransform * ConvertAiMatrixToMyMatrix(node->mTransformation);
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


//-----------------------------------------------------------------------------------------------
// Constructs a Quaternion from an aiQuaternion
//
Quaternion ConvertAiQuaternionToMyQuaternion(aiQuaternion aiQuat)
{
	Quaternion result;

	result.s = aiQuat.w;
	result.v.x = aiQuat.x;
	result.v.y = aiQuat.y;
	result.v.z = aiQuat.z;

	return result;
}


//-----------------------------------------------------------------------------------------------
// Recursively prints out the AI tree, including node names and all matrix transforms for each node
//
void DebugPrintAITree(aiNode* node, const std::string& indent)
{
	aiMatrix4x4 aiTransform = node->mTransformation;
	Matrix44 t = ConvertAiMatrixToMyMatrix(aiTransform);

	// Name with indent
	std::string text = "\n" + indent + "NODE: %s\n";
	DebuggerPrintf(text.c_str(), node->mName.C_Str());

	// Line separator
	text = indent + "-----------------------------------------\n";
	DebuggerPrintf(text.c_str());

	// Matrix
	text = "|%f %f %f %f|\n|%f %f %f %f|\n|%f %f %f %f|\n|%f %f %f %f|\n";
	DebuggerPrintf(text.c_str(), t.Ix, t.Jx, t.Kx, t.Tx, t.Iy, t.Jy, t.Ky, t.Ty, t.Iz, t.Jz, t.Kz, t.Tz, t.Iw, t.Jw, t.Kw, t.Tw);

	// Line separator
	text = indent + "-----------------------------------------\n";
	DebuggerPrintf(text.c_str());

	// Recursive call on children
	for (unsigned int childIndex = 0; childIndex < node->mNumChildren; ++childIndex)
	{
		DebugPrintAITree(node->mChildren[childIndex], indent + "-");
	}
}


//-----------------------------------------------------------------------------------------------
// Prints out the given aiAnimation information, including all channels
//
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
