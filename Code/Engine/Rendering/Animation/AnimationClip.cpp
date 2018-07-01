#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/Animation/Pose.hpp"
#include "Engine/Rendering/Animation/AnimationClip.hpp"
#include "Engine/Rendering/Animation/SkeletonBase.hpp"


void AnimationClip::Initialize(unsigned int numPoses, const SkeletonBase* skeleton)
{
	m_poses = (Pose*) malloc(sizeof(Pose) * numPoses);
	m_numPoses = numPoses;

	m_baseSkeleton = skeleton;
}

Pose* AnimationClip::CalculatePoseAtTime(float t) const
{
	float normalizedTime = RangeMapFloat(t, 0.f, GetTotalDurationSeconds(), 0.f, 1.f);

	return CalculatePoseAtNormalizedTime(normalizedTime);
}

Pose* AnimationClip::GetPoseAtIndex(unsigned int poseIndex)
{
	return &m_poses[poseIndex];
}

Pose* AnimationClip::CalculatePoseAtNormalizedTime(float t) const
{
	// Loop the animation for now
	while (t >= 1.0f)
	{
		t -= 1.0f;
	}

	int numPoses = GetPoseCount();

	int firstPoseIndex = (int) (t * numPoses);
	int secondPoseIndex = firstPoseIndex == (numPoses - 1) ? 0 : firstPoseIndex + 1;

	return CalcuateInterpolatedPose(firstPoseIndex, secondPoseIndex, t);
}

int AnimationClip::GetPoseCount() const
{
	return m_numPoses;
}

float AnimationClip::GetTotalDurationSeconds() const
{
	return m_durationSeconds;
}


void AnimationClip::SetName(const std::string& name)
{
	m_name = name;
}

void AnimationClip::SetFramesPerSecond(float framesPerSecond)
{
	m_framesPerSecond = framesPerSecond;
}

void AnimationClip::SetDurationSeconds(float durationSeconds)
{
	m_durationSeconds = durationSeconds;
}

Pose* AnimationClip::CalcuateInterpolatedPose(unsigned int firstPoseIndex, unsigned int secondPoseIndex, float t) const
{
	Pose* firstPose = &m_poses[firstPoseIndex];
	Pose* secondPose = &m_poses[secondPoseIndex];
	unsigned int transformCount = firstPose->GetBoneCount();

	Pose* result = new Pose();
	result->Initialize(m_baseSkeleton);

	for (unsigned int transformIndex = 0; transformIndex < transformCount; ++transformIndex)
	{
		Matrix44 firstTransfrom = firstPose->GetBoneTransform(transformIndex);
		Matrix44 secondTransfrom = secondPose->GetBoneTransform(transformIndex);

		Matrix44 finalTransform = Interpolate(firstTransfrom, secondTransfrom, t);
		result->SetBoneTransform(transformIndex, finalTransform);
	}

	return result;
}
