#pragma once
#include <map>
#include "Engine/Math/Matrix44.hpp"

#define MAX_BONES_PER_VERTEX (4)

struct VertexBoneData_t
{
	unsigned int boneIndices[MAX_BONES_PER_VERTEX];
	float boneWeights[MAX_BONES_PER_VERTEX];
};

struct BoneData_t
{
	Matrix44	localToParentTransform;	// Matrix from the Node it was in
	Matrix44	offsetMatrix;	// Matrix that converts vertices from the local space into the bone space
};

class SkeletonBase
{
public:

	int	GetBoneMapping(const std::string name);
	int	CreateOrGetBoneMapping(const std::string& boneName);

	void SetOffsetMatrix(unsigned int boneIndex, const Matrix44& offsetMatrix);


private:

	std::map<std::string, unsigned int> m_boneNameMappings;
	std::vector<BoneData_t> m_boneData;

};
