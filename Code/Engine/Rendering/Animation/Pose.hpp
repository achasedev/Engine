#pragma once
#include <vector>
#include "Engine/Math/Matrix44.hpp"

class SkeletonBase;

class Pose
{
public:
	//-----Public Methods-----

	Pose() {}
	~Pose();

	void			Initialize(const SkeletonBase* skeleton);

	// Accessors
	unsigned int	GetBoneCount() const;
	Matrix44		GetBoneTransform(unsigned int transformIndex) const;
	const Matrix44* GetBoneTransformData() const;
	const SkeletonBase* GetBaseSkeleton() const;

	// Mutators
	void			SetBoneTransform(unsigned int index, const Matrix44& transform);

	void			ConstructGlobalMatrices();


private:
	//-----Private Data-----

	Matrix44* m_boneTransforms = NULL;
	unsigned int m_boneCount = 0;

	const SkeletonBase* m_baseSkeleton;
};
