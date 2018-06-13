/************************************************************************/
/* File: Quaternion.hpp
/* Author: Andrew Chase
/* Date: June 12th, 2018
/* Description: Class to represent a Quaternion rotation
/************************************************************************/
#include "Engine/Math/Vector3.hpp"

class Quaternion
{
public:
	//-----Public Methods-----

	// Constructors
	Quaternion();
	Quaternion(float scalar, const Vector3& vector);
	Quaternion(const Quaternion& copy);
	~Quaternion() {}


	// Operators
	void operator=(const Quaternion& copy);
	const Quaternion operator+(const Quaternion& other) const;
	const Quaternion operator-(const Quaternion& other) const;
	const Quaternion operator*(const Quaternion& other) const;

	const Quaternion operator*(float scalar) const;
	friend const Quaternion operator*(float scalar, const Quaternion& quat);

	void operator+=(const Quaternion& other);
	void operator-=(const Quaternion& other);
	void operator*=(const Quaternion& other);
	void operator*=(float scalar);

	float		GetMagnitude() const;
	Quaternion	GetNormalized() const;
	Quaternion	GetConjugate() const;
	Quaternion	GetInverse() const;
	Vector3		GetAsEulerAngles() const;

	void		Normalize();
	void		ConvertToUnitNorm();


	static float		GetAngleBetweenDegrees(const Quaternion& a, const Quaternion& b);
	static Quaternion	FromEuler(const Vector3& eulerAnglesDegrees);
	static Quaternion	RotateToward(const Quaternion& start, const Quaternion& end, float maxAngleDegrees);

	static Quaternion Lerp(const Quaternion& a, const Quaternion& b, float fractionTowardEnd);
	static Quaternion Slerp(const Quaternion& start, const Quaternion& end, float fractionTowardEnd);


public:
	//-----Public Data-----

	float s;
	Vector3 v;

	// Statics
	static const Quaternion IDENTITY;

};
