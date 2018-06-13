#pragma once
#include <map>
#include "Engine/Math/Matrix44.hpp"

struct Bone_t
{
	std::string name;
	Matrix44	localToParentTransform;	// Matrix from the Node it was in
	Matrix44	localToBoneTransform;	// Matrix that converts vertices from the local space into the bone space
};

class SkeletonBase
{
public:

	Bone_t GetBone(const std::string& boneName);


private:

	std::map<std::string, Bone_t> m_bones;


};
