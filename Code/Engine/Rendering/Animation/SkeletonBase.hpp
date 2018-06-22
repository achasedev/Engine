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
	Matrix44	worldTransform;				// Transform of the bone in world space
	Matrix44	finalTransformation;		// Full transformation: From Node local -> bone space -> World -> Model Local
	Matrix44	offsetMatrix;				// Matrix that converts vertices from the local space into the bone space
	int			parentIndex = -1;			// Index of the parent of this bone, -1 indicates no parent (root)
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

	std::vector<std::string> GetAllBoneNames() const;

	// Mutators
	void SetOffsetMatrix(unsigned int boneIndex, const Matrix44& offsetMatrix);
	void SetWorldTransform(unsigned int boneIndex, const Matrix44& worldTransform);
	void SetParentBoneIndex(unsigned int boneIndex, int parentBoneIndex);
	void SetFinalTransformation(unsigned int boneIndex, const Matrix44& toWorldMatrix);
	void SetGlobalInverseTransform(const Matrix44& inverseTransform);


private:
	//-----Private Data-----

	std::map<std::string, unsigned int> m_boneNameMappings;	// Registry that maps bone names to element positions in the m_boneData array
	std::vector<BoneData_t>				m_boneData;			// Collection of bone information (transforms, parent indices)

	Matrix44 m_globalInverseTransform;	// Inverse transform of the Root node for the entire Assimp tree
										// Used to transform a vertex back into "model" space after the bone transformation
};
