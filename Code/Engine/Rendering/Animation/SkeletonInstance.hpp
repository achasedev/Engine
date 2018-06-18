#pragma once
#include "Engine/Rendering/Animation/Pose.hpp"
#include "Engine/Rendering/Animation/SkeletonBase.hpp"


class SkeletonInstance
{
public:
	//-----Public Methods-----

	SkeletonInstance(const SkeletonBase* base);

	void ResetToBase();


private:
	//-----Private Data-----

	Pose m_currentPose;
	const SkeletonBase* m_baseSkeleton;

};
