#pragma once
#include <vector>

class Pose;

class AnimationClip
{
public:
	//-----Public Methods-----

	Pose	GetPoseAtTime(float t) const;
	Pose	GetPoseAtNormalizedTime(float t) const;

	int		GetPoseCount() const;
	float	GetFrameDuration() const;
	float	GetTotalDuration() const;


private:
	//-----Private Data-----

	std::vector<Pose> m_poses;
	float m_frameDuration;

};
