/************************************************************************/
/* File: SkeletonBase.cpp
/* Author: Andrew Chase
/* Date: June 15th, 2018
/* Description: Implementation of the SkeletonBase class
/************************************************************************/
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Rendering/Animation/Skeleton.hpp"


//-----------------------------------------------------------------------------------------------
// Returns the bone data structure for the bone at the given index
//
BoneData_t Skeleton::GetBoneData(unsigned int boneIndex) const
{
	ASSERT_OR_DIE(boneIndex < m_boneData.size(), Stringf("Error: SkeletonBase::SetOffsetMatrix received index out of bounds - size is %i, index is %i.", m_boneData.size(), boneIndex));

	return m_boneData[boneIndex];
}


//-----------------------------------------------------------------------------------------------
// Returns the index of the bone given by name in the mappings array
// Returns -1 if a bone of the given name doesn't exist
//
int Skeleton::GetBoneMapping(const std::string name) const
{
	bool nameExists = m_boneNameMappings.find(name) != m_boneNameMappings.end();

	if (nameExists)
	{
		return m_boneNameMappings.at(name);
	}

	return -1;
}


//-----------------------------------------------------------------------------------------------
// Returns the index of the bone mapped by the given name if it already exists
// If it doesn't exist, it creates a mapping and index for it and returns the newly created index
//
int Skeleton::CreateOrGetBoneMapping(const std::string& boneName)
{
	bool nameExists = m_boneNameMappings.find(boneName) != m_boneNameMappings.end();
	int boneIndex;

	if (nameExists)
	{
		boneIndex = m_boneNameMappings[boneName];
	}
	else
	{
		boneIndex = (unsigned int) m_boneData.size();
		m_boneData.push_back(BoneData_t());
		m_boneNameMappings[boneName] = boneIndex;

		// Also add the name to the name's list
		m_boneNames.push_back(boneName);
	}

	return boneIndex;
}


//-----------------------------------------------------------------------------------------------
// Returns the name of the root bone for this skeleton
//
std::string Skeleton::GetRootBoneName() const
{
	return m_boneNames[0];
}


//-----------------------------------------------------------------------------------------------
// Returns the list of bone names for this skeleton
//
std::vector<std::string> Skeleton::GetAllBoneNames() const
{
	return m_boneNames;
}


//-----------------------------------------------------------------------------------------------
// Returns the number of bones in the skeleton
//
unsigned int Skeleton::GetBoneCount() const
{
	return (unsigned int) m_boneData.size();
}


//-----------------------------------------------------------------------------------------------
// Sets the offset matrix of the bone given by boneIndex
// The offset matrix is the matrix that converts the vertex from model space into bone space
//
void Skeleton::SetBoneToMeshMatrix(unsigned int boneIndex, const Matrix44& offsetMatrix)
{
	ASSERT_OR_DIE(boneIndex < m_boneData.size(), Stringf("Error: SkeletonBase::SetOffsetMatrix received index out of bounds - size is %i, index is %i.", m_boneData.size(), boneIndex));

	m_boneData[boneIndex].boneToMeshMatrix = offsetMatrix;
}


void Skeleton::SetLocalTransform(unsigned int boneIndex, const Matrix44& localTransform)
{
	ASSERT_OR_DIE(boneIndex < m_boneData.size(), Stringf("Error: SkeletonBase::SetLocalTransform received index out of bounds - size is %i, index is %i.", m_boneData.size(), boneIndex));

	m_boneData[boneIndex].localTransform = localTransform;
}


//-----------------------------------------------------------------------------------------------
// Sets the world matrix of the bone given by boneIndex
// The world matrix is the matrix that transforms from bone space directly to world space
//
void Skeleton::SetWorldTransform(unsigned int boneIndex, const Matrix44& worldTransform)
{
	m_boneData[boneIndex].worldTransform = worldTransform;
}


//-----------------------------------------------------------------------------------------------
// Sets the parent index of the given bone at boneIndex
//
void Skeleton::SetParentBoneIndex(unsigned int boneIndex, int parentBoneIndex)
{
	ASSERT_OR_DIE(boneIndex < m_boneData.size(), Stringf("Error: SkeletonBase::SetToWorldMatrix received index out of bounds - size is %i, index is %i.", m_boneData.size(), boneIndex));

	if (boneIndex == 0 && parentBoneIndex != -1)
	{
		ERROR_AND_DIE("Root got a parent");
	}

	m_boneData[boneIndex].parentIndex = parentBoneIndex;
}


//-----------------------------------------------------------------------------------------------
// Sets the mesh to bone matrix (inverse bind pose)
//
void Skeleton::SetMeshToBoneMatrix(unsigned int boneIndex, const Matrix44& meshToBoneTransform)
{
	ASSERT_OR_DIE(boneIndex < m_boneData.size(), Stringf("Error: SkeletonBase::SetBindPose received index out of bounds - size is %i, index is %i.", m_boneData.size(), boneIndex));

	m_boneData[boneIndex].meshToBoneMatrix = meshToBoneTransform;
}


//-----------------------------------------------------------------------------------------------
// Sets the offset matrix from Assimp's data
//
void Skeleton::SetOffsetMatrix(unsigned int boneIndex, const Matrix44& offsetTransform)
{
	ASSERT_OR_DIE(boneIndex < m_boneData.size(), Stringf("Error: SkeletonBase::SetBindPose received index out of bounds - size is %i, index is %i.", m_boneData.size(), boneIndex));

	m_boneData[boneIndex].offsetMatrix = offsetTransform;
}


//-----------------------------------------------------------------------------------------------
// Sets the prerotation of this bone to the one specified
//
void Skeleton::SetBonePreRotation(unsigned int boneIndex, const Matrix44& preRotation)
{
	ASSERT_OR_DIE(boneIndex < m_boneData.size(), Stringf("Error: SkeletonBase::SetBindPose received index out of bounds - size is %i, index is %i.", m_boneData.size(), boneIndex));

	m_boneData[boneIndex].preRotation = preRotation;
}
