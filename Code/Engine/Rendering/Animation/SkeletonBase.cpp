#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Rendering/Animation/SkeletonBase.hpp"

int SkeletonBase::GetBoneMapping(const std::string name)
{
	bool nameExists = m_boneNameMappings.find(name) != m_boneNameMappings.end();

	if (nameExists)
	{
		return m_boneNameMappings[name];
	}

	return -1;
}

int SkeletonBase::CreateOrGetBoneMapping(const std::string& boneName)
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
	}

	return boneIndex;
}

void SkeletonBase::SetOffsetMatrix(unsigned int boneIndex, const Matrix44& offsetMatrix)
{
	ASSERT_OR_DIE(boneIndex < m_boneData.size(), Stringf("Error: SkeletonBase::SetOffsetMatrix received index out of bounds - size is %i, index is %i.", m_boneData.size(), boneIndex));

	m_boneData[boneIndex].offsetMatrix = offsetMatrix;
}

