
class Quaternion
{

};
























































































/*
// DAMN NEAR EVERYTHING IS FROM;
// 3D Math Primer for Graphics and Game Development
// by Fletcher Dunn and Ian Parberry
#pragma once

class Vector3;
class Vector4;
class Matrix44;

class Quaternion 
{
public:
	//-----Public Methods-----
	
	// Constructors
	Quaternion();
	Quaternion(float _x, float _y, float _z, float _w);
	Quaternion(float _r, const Vector3& _i);

	Quaternion operator-(Quaternion const &a);
	Quaternion operator*(Quaternion const &a, Quaternion const &b); 
	Vector3 operator*(Vector3 const &v, Quaternion const &q);
	Vector4 operator*(Vector4 const &v, Quaternion const &q);

	void Normalize();
	bool IsUnit() const;

	float GetMagnitude() const;
	float GetMagnitude2() const;

	Quaternion GetConjugate() const;
	Quaternion GetInverse() const;

	Vector3 GetAsEulerAngles() const;
	Matrix44 GetAsMatrix() const;

	void Invert() { *this = GetInverse(); } 

	inline Vector3 GetRightVector() const { return Vector3::DIRECTION_RIGHT * (*this); }
	inline Vector3 GetUpVector() const { return Vector3::DIRECTION_UP * (*this); }
	inline Vector3 GetForwardVector() const { return Vector3::DIRECTION_FORWARD * (*this); }


	// Static Methods
	static Quaternion const IDENTITY;

	static Quaternion FromMatrix(const Matrix44& mat); 

	static Quaternion Around( Vector3 const &axis, float const angle_radians );
	static Quaternion FromEuler( Vector3 const &euler );
	static inline Quaternion FromEuler( float x, float y, float z ) { return FromEuler( Vector3( x, y, z ) ); }
	static Quaternion FromEuler( Vector3 const &euler, eRotationOrder const order );

	static Quaternion LookAt( Vector3 const forward );

	static Quaternion	QuaternionGetDifference( Quaternion const &a, Quaternion const &b ); 
	static float		QuaternionAngleInRadians( Quaternion const &a, Quaternion const &b );
 
	static float		QuaternionDot( Quaternion const &a, Quaternion const &b ); 
	static Quaternion	QuaternionLog(Quaternion const &q); // Works on normalized Quaternion - returns a non-normalized Quaternion
	static Quaternion	QuaternionExp( Quaternion const &q );	// Works on Quaternions of the form [0, a * i]
	static Quaternion	QuaternionScale( Quaternion const &q, float s ); 
	static Quaternion	QuaternionPow( Quaternion const &q, float e );
	static Quaternion	Slerp( Quaternion const &a, Quaternion const &b, float const t ); 
	static Quaternion	QuaternionRotateTorward( Quaternion const &start, Quaternion const &end, float max_angle_radians );

	static Quaternion	Lerp( Quaternion const &a, Quaternion const &b, float const &t );
	static bool			NearEqual( Quaternion const &a, Quaternion const &b );


private:
	//-----Private Data-----

	union {
		struct {
			Vector3 i;
			float r;
		};
		Vector4 data;
	};
};
*/