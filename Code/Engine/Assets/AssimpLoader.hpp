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

#include "ThirdParty/assimp/include/assimp/scene.h"
#include "ThirdParty/assimp/include/assimp/cimport.h"
#include "ThirdParty/assimp/include/assimp/Importer.hpp"
#include "ThirdParty/assimp/include/assimp/postprocess.h"

class Texture;
class Renderable;
class SkeletonBase;
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

	void OpenFile(const std::string& filepath);
	void CloseFile();

	Renderable*						ImportMesh(SkeletonBase* skeleton = nullptr);
	SkeletonBase*					ImportSkeleton();
	std::vector<AnimationClip*>		ImportAnimation(SkeletonBase* skeleton, int firstFrame = 0);


private:
	//-----Private Methods-----

	// Skeletal
	void InitializeSkeleton(SkeletonBase* skeleton);
		void GetBoneNamesFromNode(aiNode* node, std::vector<std::string>& out_names);
		void CreateBoneMappingsFromNode(aiNode* node, const std::vector<std::string>& boneNames, SkeletonBase* skeleton);
		void SetBoneOffsetData(aiNode* node, SkeletonBase* skeleton);
		void BuildBoneHierarchy(SkeletonBase* skeleton);
		void ExtractBoneTransform(aiNode* ainode, const Matrix44& accumulatedTransform, int parentBoneIndex, SkeletonBase* skeleton);

	// Meshes (including bone vertex data) and materials
	void BuildMeshesAndMaterials_FromScene(Renderable* renderable, SkeletonBase* skeleton);
		void BuildMeshesAndMaterials_FromNode(aiNode* node, const Matrix44& parentTransform, Renderable* renderable, SkeletonBase* skeleton);
			void BuildMeshAndMaterial_FromAIMesh(aiMesh* mesh, const Matrix44& transformation, Renderable* renderable, SkeletonBase* skeleton);



	// Build Animations
	void BuildAnimations(SkeletonBase* skeleton, std::vector<AnimationClip*>& animations, int firstFrameIndex);
		AnimationClip* BuildAnimation(unsigned int animationIndex, SkeletonBase* skeleton,  int firstFrameIndex);
			void FillPoseForTime(Pose* out_pose, aiAnimation* aianimation, float time, SkeletonBase* skeleton,  int firstFrameIndex);
				aiNodeAnim* GetChannelForBone(const std::string& boneName, aiAnimation* animation) const;
				Matrix44	GetLocalTransfromAtTime(aiNodeAnim* channel, float time, const Matrix44& preRotation,  int firstFrameIndex);
					aiVector3D		GetWorldTranslationAtTime(const aiNodeAnim* channel, float time,  int firstFrameIndex) const;
					aiQuaternion	GetWorldRotationAtTime(aiNodeAnim* channel, float time,  int firstFrameIndex) const;
					aiVector3D		GetWorldScaleAtTime(aiNodeAnim* channel, float time,  int firstFrameIndex) const;
				bool	ConstructMatrixFromSeparatedChannels(const std::string& boneName, aiAnimation* animation, float time, SkeletonBase* skeleton, Matrix44& out_transform,  int firstFrameIndex) const;


private:
	//-----Private Data-----


	const aiScene*	m_scene = nullptr;

};
