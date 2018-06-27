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

	bool LoadFile(const std::string& filepath);

	// Accessors
	Renderable*		GetRenderable();
	AnimationClip*	GetAnimationClip(unsigned int index);
	const SkeletonBase*	GetSkeletonBase() const;


private:
	//-----Private Methods-----

	// Skeletal Bone order
	void InitializeSkeleton();
		void GetBoneNamesFromNode(aiNode* node, std::vector<std::string>& out_names);
		void CreateBoneMappingsFromNode(aiNode* node, const std::vector<std::string>& boneNames);

	// Meshes (including bone vertex data) and materials
	void BuildMeshesAndMaterials_FromScene();
		void BuildMeshesAndMaterials_FromNode(aiNode* node, const Matrix44& parentTransform);
			void BuildMeshAndMaterial_FromAIMesh(aiMesh* mesh, const Matrix44& transformation);

	// Build skeletal structure
	void BuildBoneHierarchy();
		void ExtractBoneTransform(aiNode* ainode, const Matrix44& parentTransfrom, int parentBoneIndex, const std::string& concatenations);

	// Build Animations
	void BuildAnimations();
		void BuildAnimation(unsigned int animationIndex);
			void FillPoseForTime(Pose* out_pose, aiAnimation* aianimation, float time);
				aiNodeAnim* GetChannelForBone(const std::string& boneName, aiAnimation* animation) const;
				Matrix44	GetLocalTransfromAtTime(aiNodeAnim* channel, float time);
					aiVector3D		GetWorldTranslationAtTime(aiNodeAnim* channel, float time) const;
					aiQuaternion	GetWorldRotationAtTime(aiNodeAnim* channel, float time) const;
					aiVector3D		GetWorldScaleAtTime(aiNodeAnim* channel, float time) const;
				Matrix44	ConstructMatrixFromSeparatedChannels(const std::string& boneName, aiAnimation* animation, float time) const;


private:
	//-----Private Data-----

	Renderable*		m_renderable = nullptr;
	SkeletonBase*	m_skeleton = nullptr;
	const aiScene*	m_scene = nullptr;

	std::vector<AnimationClip*> m_animationClips;

};
