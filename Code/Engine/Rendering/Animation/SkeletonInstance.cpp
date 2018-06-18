#include "Engine/Rendering/Animation/SkeletonInstance.hpp"

SkeletonInstance::SkeletonInstance(const SkeletonBase* base)
{
	m_baseSkeleton = base;

	ResetToBase();
}

void SkeletonInstance::ResetToBase()
{
	int numBones = m_baseSkeleton->GetBoneCount();

	for (int boneIndex = 0; boneIndex < numBones; ++boneIndex)
	{
		BoneData_t currBone = m_baseSkeleton->GetBoneData(boneIndex);

		m_currentPose.AddTransform(currBone.finalTransformation);
	}

	m_baseSkeleton = m_baseSkeleton;
}
