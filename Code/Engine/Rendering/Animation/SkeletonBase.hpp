/************************************************************************/
/* File: SkeletonBase.hpp
/* Author: Andrew Chase
/* Date: June 15th, 2018
/* Description: Class to represent an animation skeleton resource (data, not state)
/************************************************************************/
#pragma once
#include <map>
#include <vector>
#include "Engine/Math/Matrix44.hpp"

#define MAX_BONES_PER_VERTEX (4) // Only support up to 4 bone weights per vertex

// Structure to represent a single bone in the skeleton
struct BoneData_t
{
	Matrix44	localTransform;				// Transform of the bone local to it's parent bone
	Matrix44	worldTransform;				// Transform of the bone in world space
	Matrix44	finalTransformation;		
	Matrix44	boneToMeshMatrix;				// Matrix that converts vertices from bone to mesh space
	int			parentIndex = -1;			// Index of the parent of this bone, -1 indicates no parent (root)
	Matrix44	meshToBoneMatrix;
	Matrix44	offsetMatrix;
};

class SkeletonBase
{
public:
	//-----Public Methods-----

	// Accessors
	BoneData_t	GetBoneData(unsigned int boneIndex) const;
	int			GetBoneMapping(const std::string name) const;
	int			CreateOrGetBoneMapping(const std::string& boneName);
	
	unsigned int	GetBoneCount() const;
	Matrix44		GetGlobalInverseTransform() const;
	Matrix44		GetRootBoneOffset() const;
	std::string		GetRootBoneName() const;

	std::vector<std::string> GetAllBoneNames() const;

	// Mutators
	void SetBoneToMeshMatrix(unsigned int boneIndex, const Matrix44& offsetMatrix);
	void SetLocalTransform(unsigned int boneIndex, const Matrix44& localTransform);
	void SetWorldTransform(unsigned int boneIndex, const Matrix44& worldTransform);
	void SetParentBoneIndex(unsigned int boneIndex, int parentBoneIndex);
	void SetFinalTransformation(unsigned int boneIndex, const Matrix44& toWorldMatrix);
	void SetGlobalInverseTransform(const Matrix44& inverseTransform);
	void SetRootBoneOffset(const Matrix44& transform);
	void SetMeshToBoneMatrix(unsigned int boneIndex, const Matrix44& bindPoseTransform);
	void SetOffsetMatrix(unsigned int boneIndex, const Matrix44& offsetTransform);


private:
	//-----Private Data-----

	std::map<std::string, unsigned int> m_boneNameMappings;	// Registry that maps bone names to element positions in the m_boneData array
	std::vector<BoneData_t>				m_boneData;			// Collection of bone information (transforms, parent indices)

	Matrix44 m_globalInverseTransform;	// Inverse transform of the Root node for the entire Assimp tree
										// Used to transform a vertex back into "model" space after the bone transformation

	Matrix44 m_rootBoneOffset;
};
