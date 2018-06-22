#pragma once
#include <vector>
#include "Engine/Rendering/Animation/Pose.hpp"

class AnimationClip
{
public:
	//-----Public Methods-----

	void Initialize(unsigned int numPoses, const SkeletonBase* skeleton);

	// Accessors
	Pose*	GetPoseAtIndex(unsigned int poseIndex);
	Pose*	CalculatePoseAtTime(float t) const;
	Pose*	CalculatePoseAtNormalizedTime(float t) const;

	int		GetPoseCount() const;
	float	GetTotalDurationSeconds() const;

	
	// Mutators
	void SetName(const std::string& name);
	void SetFramesPerSecond(float framesPerSecond);
	void SetDurationSeconds(float durationSeconds);


private:
	//-----Private Methods-----

	Pose* CalcuateInterpolatedPose(unsigned int firstPoseIndex, unsigned int secondPoseIndex, float t) const;


private:
	//-----Private Data-----

	std::string m_name;

	Pose* m_poses = nullptr;
	unsigned int m_numPoses;

	float m_durationSeconds;
	float m_framesPerSecond;

	const SkeletonBase* m_baseSkeleton;
};
