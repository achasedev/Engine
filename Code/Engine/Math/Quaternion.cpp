#include <math.h>
#include "Engine/Math/Quaternion.hpp"
#include "Engine/Math/MathUtils.hpp"


Quaternion::Quaternion(float scalar, const Vector3& vector)
	: v(vector), s(scalar)
{
}


Quaternion::Quaternion()
	: v(Vector3::ZERO), s(1.f)
{
}

Quaternion::Quaternion(const Quaternion& copy)
	: v(copy.v), s(copy.s)
{
}

const Quaternion Quaternion::operator+(const Quaternion& other) const
{
	Quaternion result;
	result.v = v + other.v;
	result.s = s + other.s;

	return result;
}

const Quaternion Quaternion::operator-(const Quaternion& other) const
{
	Quaternion result;
	result.v = v - other.v;
	result.s = s - other.s;

	return result;
}

const Quaternion Quaternion::operator*(const Quaternion& other) const
{
	Quaternion result;

	result.s = s * other.s - DotProduct(v, other.v);
	result.v = s * other.v + v * other.s + CrossProduct(v, other.v);

	return result;
}

const Quaternion Quaternion::operator*(float scalar) const
{
	Quaternion result;

	result.s = s * scalar;
	result.v = v * scalar;

	return result;
}

const Quaternion operator*(float scalar, const Quaternion& quat)
{
	Quaternion result;

	result.s = (scalar * quat.s);
	result.v = (scalar * quat.v);

	return result;
}

Quaternion Quaternion::Lerp(const Quaternion& a, const Quaternion& b, float fractionTowardEnd)
{
	float sResult	= Interpolate(a.s, b.s, fractionTowardEnd);
	Vector3 vResult = Interpolate(a.v, b.v, fractionTowardEnd);

	return Quaternion(sResult, vResult);
}

void Quaternion::operator*=(const Quaternion& other)
{
	Quaternion old = (*this);

	s = old.s * other.s - DotProduct(old.v, other.v);
	v = old.s * other.v + old.v * other.s + CrossProduct(old.v, other.v);
}



void Quaternion::operator*=(float scalar)
{
	s = s * scalar;
	v = v * scalar;
}

float Quaternion::GetMagnitude() const
{
	float squaredNorm = (s * s) + (v.x * v.x) + (v.y * v.y) + (v.z * v.z);
	float magnitude = sqrtf(squaredNorm);

	return magnitude;
}

Quaternion Quaternion::GetNormalized() const
{
	float magnitude = GetMagnitude();
	if (magnitude == 0.f)
	{
		return (*this);
	}

	float oneOverMag = (1.f / GetMagnitude());
	Quaternion result = (*this);
	
	return result * oneOverMag;
}

Quaternion Quaternion::GetInverse() const
{
	float absoluteValue = GetMagnitude();
	absoluteValue *= absoluteValue;
	absoluteValue = 1.0f / absoluteValue;

	Quaternion conjugate = GetConjugate();

	float scalar = conjugate.s * absoluteValue;
	Vector3 vector = conjugate.v * absoluteValue;

	return Quaternion(scalar, vector);
}

void Quaternion::Normalize()
{
	(*this) = GetNormalized();
}

void Quaternion::ConvertToUnitNorm()
{
	float angleDegrees = s;

	v.NormalizeAndGetLength();
	s = CosDegrees(0.5f * angleDegrees);
	v = (v * SinDegrees(0.5f * angleDegrees));
}


float Quaternion::GetAngleBetweenDegrees(const Quaternion& a, const Quaternion& b)
{	
	float newReal = a.s * b.s - DotProduct(-1.0f * a.v, b.v);
	float result = 2.0f * ACosDegrees(newReal);

	return result;
}

Quaternion Quaternion::FromEuler(const Vector3& eulerAnglesDegrees)
{
	const Vector3 he = 0.5f * eulerAnglesDegrees;

	float cx = CosDegrees( he.x );
	float sx = SinDegrees( he.x );
	float cy = CosDegrees( he.y );
	float sy = SinDegrees( he.y );
	float cz = CosDegrees( he.z );
	float sz = SinDegrees( he.z );

	float r =   cx*cy*cz + sx*sy*sz;
	float ix =  sx*cy*cz + cx*sy*sz;
	float iy =  cx*sy*cz - sx*cy*sz;
	float iz =  cx*cy*sz - sx*sy*cz;


	Quaternion result = Quaternion(r, Vector3(ix, iy, iz));
	result.Normalize();

	return result;
}

Quaternion Quaternion::RotateToward(const Quaternion& start, const Quaternion& end, float maxAngleDegrees)
{
	float angleBetween = GetAngleBetweenDegrees(start, end);

	if (angleBetween < 0.f)
	{
		angleBetween = -angleBetween;
	}

	if (AreMostlyEqual(angleBetween, 0.f))
	{
		return end;
	}

	float t = ClampFloatZeroToOne(maxAngleDegrees / angleBetween);
	Quaternion result = Slerp(start, end, t);

	return result;
}

Quaternion Quaternion::Slerp(const Quaternion& a, const Quaternion& b, float fractionTowardEnd)
{
	fractionTowardEnd = ClampFloatZeroToOne(fractionTowardEnd);
	float cosAngle = DotProduct(a,b);


	Quaternion start;
	if (cosAngle < 0.0f) {
		// If it's negative - it means it's going the long way
		// flip it.
		start = -1.0f * a;
		cosAngle = -cosAngle;
	} else {
		start = a;
	}

	float f0, f1;
	if (cosAngle >= .9999f) {
		// very close - just linearly interpolate for speed
		f0 = 1.0f - fractionTowardEnd;
		f1 = fractionTowardEnd;
	} else {
		float sinAngle = sqrtf( 1.0f - cosAngle * cosAngle );
		float angle = atan2f( sinAngle, cosAngle );

		float den = 1.0f / sinAngle;
		f0 = sinf((1.0f - fractionTowardEnd) * angle) * den;
		f1 = sinf(fractionTowardEnd * angle ) * den;
	}

	Quaternion r0 = start * f0;
	Quaternion r1 = b * f1;

	return Quaternion(r0.s + r1.s, r0.v + r1.v); 
}

Quaternion Quaternion::GetConjugate() const
{
	Quaternion result;

	result.s = s;
	result.v = -1.0f * v;

	return result;
}

void Quaternion::operator+=(const Quaternion& other)
{
	v += other.v;
	s += other.s;
}

void Quaternion::operator-=(const Quaternion& other)
{
	v -= other.v;
	s -= other.s;
}

void Quaternion::operator=(const Quaternion& copy)
{
	v = copy.v;
	s = copy.s;
}
