#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Rendering/Animation/Pose.hpp"
#include "Engine/Rendering/Animation/SkeletonBase.hpp"

Pose::~Pose()
{
	if (m_boneTransforms != NULL)
	{
		free(m_boneTransforms);
		m_boneTransforms = NULL;
	}
}

void Pose::Initialize(const SkeletonBase* skeleton)
{
	int numBones = skeleton->GetBoneCount();
	m_boneTransforms = (Matrix44*) malloc (sizeof(Matrix44) * numBones);
	m_boneCount = numBones;
	m_baseSkeleton = skeleton;
}

const Matrix44* Pose::GetBoneTransformData() const
{
	return m_boneTransforms;
}

const SkeletonBase* Pose::GetBaseSkeleton() const
{
	return m_baseSkeleton;
}

void Pose::SetBoneTransform(unsigned int index, const Matrix44& transform)
{
	ASSERT_OR_DIE(index < m_boneCount, Stringf("Error: Pose::GetTransfrom received index out of range, index was %i", index));

	m_boneTransforms[index] = transform;
}

void Pose::ConstructGlobalMatrices()
{
	for (int boneIndex = 0; boneIndex < m_boneCount; ++boneIndex)
	{	
		BoneData_t boneData = m_baseSkeleton->GetBoneData(boneIndex);

		int parentIndex = boneData.parentIndex;

		ASSERT_OR_DIE(parentIndex < boneIndex, Stringf("Child was before parent in the pose transform array."));

		if (parentIndex >= 0)
		{
			Matrix44 localMatrix = m_boneTransforms[boneIndex];
			Matrix44 parentMatrix = m_boneTransforms[parentIndex];

			m_boneTransforms[boneIndex] = m_baseSkeleton->GetGlobalInverseTransform() * parentMatrix * localMatrix;
		}
		else
		{
			m_boneTransforms[boneIndex] = m_baseSkeleton->GetGlobalInverseTransform() * m_boneTransforms[boneIndex];
		}
	}
}

Matrix44 Pose::GetBoneTransform(unsigned int transformIndex) const
{
	ASSERT_OR_DIE(transformIndex < m_boneCount, Stringf("Error: Pose::GetTransfrom received index out of range, index was %i", transformIndex));

	return m_boneTransforms[transformIndex];
}

unsigned int Pose::GetBoneCount() const
{
	return m_boneCount;
}

