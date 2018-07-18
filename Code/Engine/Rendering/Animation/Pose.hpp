/************************************************************************/
/* File: Pose.hpp
/* Author: Andrew Chase
/* Date: July 16th, 2018
/* Description: Class to represent a single state of a skeleton 
				(single frame of an animation clip)
/************************************************************************/
#pragma once
#include <vector>
#include "Engine/Math/Matrix44.hpp"

class Skeleton;

class Pose
{
public:
	//-----Public Methods-----

	Pose() {}
	~Pose();

	void			Initialize(const Skeleton* skeleton);

	// Accessors
	unsigned int	GetBoneCount() const;
	Matrix44		GetBoneTransform(unsigned int transformIndex) const;
	const Matrix44* GetTotalBoneData() const;
	const Skeleton* GetSkeleton() const;


	// Mutators
	void			SetBoneTransform(unsigned int index, const Matrix44& transform);
	void			ConstructWorldMatrices();


private:
	//-----Private Data-----

	Matrix44* m_boneTransforms = nullptr;
	unsigned int m_boneCount = 0;

	const Skeleton* m_skeleton;

};
