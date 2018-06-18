#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/Animation/Pose.hpp"
#include "Engine/Rendering/Animation/AnimationClip.hpp"

Pose AnimationClip::GetPoseAtTime(float t) const
{
	float normalizedTime = RangeMapFloat(t, 0.f, GetTotalDuration(), 0.f, 1.f);

	return GetPoseAtNormalizedTime(normalizedTime);
}

// Pose AnimationClip::GetPoseAtNormalizedTime(float t) const
// {
// // 	// Loop the animation for now
// // 	while (t >= 1.0f)
// // 	{
// // 		t -= 1.0f;
// // 	}
// // 
// // 	int numPoses = GetPoseCount();
// // 
// // 	int firstPoseIndex = (int) (t * numPoses);
// // 	int secondPoseIndex = firstPoseIndex == (numPoses - 1) ? 0 : numPoses + 1;
// 
// 
// }

int AnimationClip::GetPoseCount() const
{
	return (int) m_poses.size();
}

float AnimationClip::GetFrameDuration() const
{
	return m_frameDuration;
}

float AnimationClip::GetTotalDuration() const
{
	return GetPoseCount() * m_frameDuration;
}
