/************************************************************************/
/* File: AssimpLoader.hpp
/* Author: Andrew Chase
/* Date: June 13th, 2018
/* Description: Class used for loading models and animation using the Assimp library
/************************************************************************/
#pragma once
#include <string>
#include <vector>
#include "Engine/Math/Matrix44.hpp"

#include "ThirdParty/assimp/include/assimp/scene.h"
#include "ThirdParty/assimp/include/assimp/cimport.h"
#include "ThirdParty/assimp/include/assimp/Importer.hpp"
#include "ThirdParty/assimp/include/assimp/postprocess.h"

// Predeclares
class Texture;
class Renderable;
class Skeleton;
class AnimationClip;
class Pose;

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;
struct aiNodeAnim;
struct aiAnimation;
struct aiString;
struct aiAnimation;


class AssimpLoader
{
public:
	//-----Public Methods-----

	// File I/O
	void OpenFile(const std::string& filepath);
	void CloseFile();

	// Parsing the Assimp scene for information
	Renderable*						ImportMesh(Skeleton* skeleton = nullptr);
	Skeleton*						ImportSkeleton();
	std::vector<AnimationClip*>		ImportAnimation(Skeleton* skeleton, int firstFrame = 0);


private:
	//-----Private Methods-----

	// Skeleton loading
	void InitializeSkeleton(Skeleton* skeleton);
		void GetBoneNamesFromNode(aiNode* node, std::vector<std::string>& out_names);
		void CreateBoneMappingsFromNode(aiNode* node, const std::vector<std::string>& boneNames, Skeleton* skeleton);
		void SetBoneOffsetData(aiNode* node, Skeleton* skeleton);
		void BuildBoneHierarchy(Skeleton* skeleton);
		void ExtractBoneTransform(aiNode* ainode, const Matrix44& accumulatedTransform, int parentBoneIndex, Skeleton* skeleton);
		void BuildLocalMatrices(Skeleton* skeleton);


	// Meshes and materials
	void BuildMeshesAndMaterials_FromScene(Renderable* renderable, Skeleton* skeleton);
		void BuildMeshesAndMaterials_FromNode(aiNode* node, const Matrix44& parentTransform, Renderable* renderable, Skeleton* skeleton);
			void BuildMeshAndMaterial_FromAIMesh(aiMesh* mesh, const Matrix44& transformation, Renderable* renderable, Skeleton* skeleton);


	// Animation
	void BuildAnimations(Skeleton* skeleton, std::vector<AnimationClip*>& animations, int firstFrameIndex);
		AnimationClip* BuildAnimation(unsigned int animationIndex, Skeleton* skeleton,  int firstFrameIndex);
			void FillPoseForTime(Pose* out_pose, aiAnimation* aianimation, float time, Skeleton* skeleton,  int firstFrameIndex);
				aiNodeAnim* GetChannelForBone(const std::string& boneName, aiAnimation* animation) const;
				Matrix44	GetLocalTransfromAtTime(aiNodeAnim* channel, float time, const Matrix44& preRotation,  int firstFrameIndex);
					aiVector3D		GetAnimationTranslationAtTime(const aiNodeAnim* channel, float time,  int firstFrameIndex) const;
					aiQuaternion	GetAnimationRotationAtTime(aiNodeAnim* channel, float time,  int firstFrameIndex) const;
					aiVector3D		GetAnimationScaleAtTime(aiNodeAnim* channel, float time,  int firstFrameIndex) const;
				bool	ConstructTransformFromSeparatedChannels(const std::string& boneName, aiAnimation* animation, float time, Skeleton* skeleton, Matrix44& out_transform,  int firstFrameIndex) const;


private:
	//-----Private Data-----

	// The Assimp scene that hold the file data in a tree
	const aiScene*	m_scene = nullptr;

};
