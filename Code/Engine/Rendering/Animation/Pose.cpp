/************************************************************************/
/* File: Pose.cpp
/* Author: Andrew Chase
/* Date: July 16th, 2018
/* Description: Implementation of the Pose class
/************************************************************************/
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Rendering/Animation/Pose.hpp"
#include "Engine/Rendering/Animation/Skeleton.hpp"


//-----------------------------------------------------------------------------------------------
// Destructor
//
Pose::~Pose()
{
	if (m_boneTransforms != NULL)
	{
		free(m_boneTransforms);
		m_boneTransforms = NULL;
	}
}


//-----------------------------------------------------------------------------------------------
// Initializes the pose for the given skeleton
//
void Pose::Initialize(const Skeleton* skeleton)
{
	// Only support up to 150 bones
	int numBones = skeleton->GetBoneCount();
	ASSERT_OR_DIE(numBones <= 150, 
		Stringf("Error: Pose::Initialize called for skeleton with more than 100 bones, unsupported. Count was %i", numBones));

	m_boneTransforms = (Matrix44*) malloc (sizeof(Matrix44) * numBones);
	m_boneCount = numBones;
	m_skeleton = skeleton;

	for (int i = 0; i < numBones; ++i)
	{
		m_boneTransforms[i] = skeleton->GetBoneData(i).localTransform;
	}
}


//-----------------------------------------------------------------------------------------------
// Returns all the bone transform data for the pose, to be used in the skinning shader
//
const Matrix44* Pose::GetTotalBoneData() const
{
	return m_boneTransforms;
}


//-----------------------------------------------------------------------------------------------
// Returns the Skeleton that this pose is based on
//
const Skeleton* Pose::GetSkeleton() const
{
	return m_skeleton;
}


//-----------------------------------------------------------------------------------------------
// Sets the bone transform at the given index to the transform specified
//
void Pose::SetBoneTransform(unsigned int index, const Matrix44& transform)
{
	ASSERT_OR_DIE(index < m_boneCount, Stringf("Error: Pose::GetTransfrom received index out of range, index was %i", index));

	m_boneTransforms[index] = transform;
}


//-----------------------------------------------------------------------------------------------
// Converts all locally-defined bone transforms in the pose into world-space transforms through
// concatenation
//
void Pose::ConstructWorldMatrices()
{
	for (int boneIndex = 0; boneIndex < (int) m_boneCount; ++boneIndex)
	{	
		BoneData_t boneData = m_skeleton->GetBoneData(boneIndex);

		int parentIndex = boneData.parentIndex;
		ASSERT_OR_DIE(parentIndex < boneIndex, Stringf("Child was before parent in the pose transform array."));

		if (parentIndex >= 0)
		{
			Matrix44 localMatrix = m_boneTransforms[boneIndex];
			Matrix44 parentMatrix = m_boneTransforms[parentIndex];

			m_boneTransforms[boneIndex] = parentMatrix * localMatrix;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the transform for the bone at the given index
//
Matrix44 Pose::GetBoneTransform(unsigned int boneIndex) const
{
	ASSERT_OR_DIE(boneIndex < m_boneCount, Stringf("Error: Pose::GetBoneTransfrom received index out of range, index was %i", boneIndex));

	return m_boneTransforms[boneIndex];
}


//-----------------------------------------------------------------------------------------------
// Returns the number of bones in this pose
//
unsigned int Pose::GetBoneCount() const
{
	return m_boneCount;
}
