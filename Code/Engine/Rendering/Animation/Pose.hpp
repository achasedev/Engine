#pragma once
#include <vector>
#include "Engine/Math/Matrix44.hpp"

class Pose
{
public:
	//-----Public Methods-----

	int				AddTransform(const Matrix44& transform);
	const Matrix44* GetTransformData() const;
	Matrix44		GetTransfrom(unsigned int transformIndex) const;


private:
	//-----Private Data-----

	std::vector<Matrix44> m_transforms;

};
