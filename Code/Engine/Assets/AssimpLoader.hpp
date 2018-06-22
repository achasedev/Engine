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
	void BuildAnimations();
		void BuildAnimation(unsigned int animationIndex);
			void FillPoseForTime(Pose* out_pose, aiAnimation* aianimation, float time);
				aiNodeAnim* GetChannelForBone(const std::string& boneName, aiAnimation* animation) const;
				Matrix44	GetLocalTransfromAtTime(aiNodeAnim* channel, float time);
					Vector3		GetWorldTranslationAtTime(aiNodeAnim* channel, float time);
					Quaternion	GetWorldRotationAtTime(aiNodeAnim* channel, float time);
					Vector3		GetWorldScaleAtTime(aiNodeAnim* channel, float time);


private:
	//-----Private Data-----

	Renderable*		m_renderable = nullptr;
	const aiScene* m_scene = nullptr;

	std::vector<AnimationClip*> m_animationClips;

};
