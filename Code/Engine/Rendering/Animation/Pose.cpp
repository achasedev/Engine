#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Rendering/Animation/Pose.hpp"

int Pose::AddTransform(const Matrix44& transform)
{
	m_transforms.push_back(transform);

	return (int) (m_transforms.size() - 1);
}

const Matrix44* Pose::GetTransformData() const
{
	return m_transforms.data();
}

Matrix44 Pose::GetTransfrom(unsigned int transformIndex) const
{
	ASSERT_OR_DIE(transformIndex < (unsigned int) m_transforms.size(), Stringf("Error: Pose::GetTransfrom received index out of range, index was %i", transformIndex));

	return m_transforms[transformIndex];
}

