

































































































































/*
#include "Engine/Math/Quaternion.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Matrix44.hpp"

const Quaternion Quaternion::IDENTITY = Quaternion( 1.0f, 0.0f, 0.0f, 0.0f );

//-----------------------------------------------------------------------------------------------
// Constructor
//
Quaternion::Quaternion( float r, float ix, float iy, float iz ) 
{
	data = Vector4( ix, iy, iz, r );
}

//------------------------------------------------------------------------
Quaternion::Quaternion( float _r, Vector3 const &_i )
{
	r = _r;
	i = _i;
}

Quaternion::Quaternion()
	: i(0.0f)
	, r(1.0f) 
{
}

Quaternion Quaternion::operator-( Quaternion const &a ) 
{
	return Quaternion(-a.r, -a.i);
}

//------------------------------------------------------------------------
Quaternion Quaternion::operator*( Quaternion const &a, Quaternion const &b ) 
{
	// Taken from 3D Math Primer - it's the reverse definition since
	// my engine also uses Row Major matrices by default, and I want to 
	// keep with that.
	Quaternion ret;
	ret.r = a.r * b.r - Dot(a.i, b.i);
	ret.i = a.r * b.i 
		+ b.r * a.i
		+ Cross( b.i, a.i );

	return ret;
}

//------------------------------------------------------------------------
Vector3 Quaternion::operator*( vec3 const &v, Quaternion const &q ) 
{
	Quaternion p = Quaternion( 0.0f, v );
	Quaternion result = q.GetInverse() * p * q;

	return result.i;
}

//------------------------------------------------------------------------
Vector4 Quaternion::operator*( vec4 const &v, Quaternion const &q ) 
{
	vec3 result = v.xyz() * q;
	return Vector4( result, v.w );
}

//------------------------------------------------------------------------
bool Quaternion::operator==( Quaternion const &a, Quaternion const &b ) 
{
	return (FlEqual(a.r, b.r) && (a.i == b.i))
		|| (FlEqual(a.r, -b.r) && (a.i == -b.i));
}

//------------------------------------------------------------------------
bool Quaternion::operator!=( Quaternion const &a, Quaternion const &b ) 
{
	return !(a == b);
}

//------------------------------------------------------------------------
float Quaternion::GetMagnitude() const
{
	return sqrtf( GetMagnitude2() );
}

//------------------------------------------------------------------------
float Quaternion::GetMagnitude2() const 
{
	return Dot( data, data );
}

//------------------------------------------------------------------------
void Quaternion::Normalize() 
{
	float const len_squared = GetMagnitude2();

	// Already normalized.
	if (FlEqual(len_squared, 1.0f)) {
		return;
	}

	if (r < .9999f) {
		float s = sqrtf( 1.0f - r * r );
		vec3 vector = i / s;
		Normalize( &vector );

		i = s * vector;
	} else {
		r = 1.0f;
		i = vec3(0.0f);
	}
}

//------------------------------------------------------------------------
bool Quaternion::IsUnit() const
{
	return FlEqual( QuaternionDot(*this, *this), 1.0f );
}

//------------------------------------------------------------------------
Quaternion Quaternion::GetConjugate() const
{
	return Quaternion( r, -i );
}

//------------------------------------------------------------------------
Quaternion Quaternion::GetInverse() const
{
	// We only deal with unit Quaternions in this engine!
	// ASSERT( is_unit() );
	return GetConjugate();
}


//------------------------------------------------------------------------
mat44 Quaternion::GetAsMatrix() const
{
	return mat44( get_mat3() );
}

//------------------------------------------------------------------------
vec3 Quaternion::GetAsEulerAngles() const 
{
	mat33 mat = get_mat3();
	return EulerFromMatrix(mat);
}

//------------------------------------------------------------------------
Quaternion Quaternion::FromMatrix( mat33 const &mat ) 
{
	float m00 = mat.get( 0, 0 );
	float m11 = mat.get( 1, 1 );
	float m22 = mat.get( 2, 2 );
	float tr = m00 + m11 + m22;

	float m21 = mat.get( 2, 1 );
	float m12 = mat.get( 1, 2 );
	float m02 = mat.get( 0, 2 );
	float m20 = mat.get( 2, 0 );
	float m10 = mat.get( 1, 0 );
	float m01 = mat.get( 0, 1 );

	Quaternion q;
	if (tr >= 0.0f) {
		float s = sqrtf(tr + 1.0f) * 2.0f;
		float is = 1.0f / s;
		q.r = .25f * s;
		q.i.x = (m21 - m12) * is;
		q.i.y = (m02 - m20) * is;
		q.i.z = (m10 - m01) * is;
	} else if ((m00 > m11) & (m00 > m22)) {
		float s = sqrtf( 1.0f + m00 - m11 - m22 ) * 2.0f;
		float is = 1.0f / s;
		q.r = (m21 - m12) * is;
		q.i.x = .25f * s;
		q.i.y = (m01 + m10) * is;
		q.i.z = (m02 + m20) * is;
	} else if (m11 > m22) {
		float s = sqrtf( 1.0f + m11 - m00 - m22 ) * 2.0f;
		float is = 1.0f / s;
		q.r = (m02 - m20) * is;
		q.i.x = (m01 + m10) * is;
		q.i.y = .25f * s;
		q.i.z = (m12 + m21) * is;
	} else {
		float s = sqrtf( 1.0f + m22 - m00 - m11 ) * 2.0f;
		float is = 1.0f / s;
		q.r = (m10 - m01) * is;
		q.i.x = (m02 + m20) * is;
		q.i.y = (m12 + m21) * is;
		q.i.z = .25f * s;
	}

	q.Normalize();
	return q;
}

//------------------------------------------------------------------------
Quaternion Quaternion::FromMatrix( mat44 const &mat )
{
	return Quaternion::FromMatrix( mat.to_mat3() );
}

//------------------------------------------------------------------------
Quaternion Quaternion::Around( vec3 const &axis, float const angle_radians ) 
{
	float const half_angle = .5f * angle_radians;
	float const c = cosf(half_angle);
	float const s = sinf(half_angle);

	return Quaternion( c, s * axis );
}

//------------------------------------------------------------------------
Quaternion Quaternion::FromEuler( vec3 const &euler, eRotationOrder const rot_order ) 
{
	if (rot_order == ROTATE_DEFAULT) {
		return Quaternion::FromEuler(euler);
	} else {
		Quaternion rotx = Quaternion::Around( vec3::RIGHT, euler.x * .5f );
		Quaternion roty = Quaternion::Around( vec3::UP, euler.y * .5f );
		Quaternion rotz = Quaternion::Around( vec3::FORWARD, euler.z * .5f );

		switch (rot_order) {
		case ROTATE_XYZ:
			return rotx * roty * rotz;
		case ROTATE_XZY:
			return rotx * rotz * roty;
		case ROTATE_YXZ:
			return roty * rotx * rotz;
		case ROTATE_YZX:
			return roty * rotz * rotx;
		case ROTATE_ZYX:
		default:
			return rotz * roty * rotx;
		}
	}
}

//------------------------------------------------------------------------
Quaternion Quaternion::FromEuler( vec3 const &euler ) 
{
	// If this changes - this method is no longer valid
	ASSERT( ROTATE_DEFAULT == ROTATE_ZXY );

	vec3 const he = .5f * euler;
	float cx = cosf( he.x );
	float sx = sinf( he.x );
	float cy = cosf( he.y );
	float sy = sinf( he.y );
	float cz = cosf( he.z );
	float sz = sinf( he.z );

	float r =   cx*cy*cz + sx*sy*sz;
	float ix =  sx*cy*cz + cx*sy*sz;
	float iy =  cx*sy*cz - sx*cy*sz;
	float iz =  cx*cy*sz - sx*sy*cz;


	Quaternion q = Quaternion( r, ix, iy, iz );
	q.Normalize();

	// ASSERT( q.is_unit() );
	return q;
}

//------------------------------------------------------------------------
Quaternion Quaternion::LookAt( vec3 const forward )
{
	mat33 lookat = mat33::LookAt( forward );
	return FromMatrix( lookat );
}


//------------------------------------------------------------------------
Quaternion Quaternion::QuaternionGetDifference( Quaternion const &a, Quaternion const &b ) 
{
	return a.GetInverse() * b;
}

//------------------------------------------------------------------------
float Quaternion::QuaternionAngleInRadians( Quaternion const &a, Quaternion const &b )
{
	// so just trying to get real part of the difference
	// inverse of a real part is the real part, so that doesn't change
	// but the imaginary part negates.
	// so need to cmpute that.
	float new_real = a.r * b.r - Dot(-a.i, b.i);
	// new_real = cosf(half_angle);
	// half_angle = acosf(new_real);
	// angle = 2 * half_angle;

	float angle = 2.0f * acosf(new_real);
	return angle;
}

//------------------------------------------------------------------------
float Quaternion::QuaternionDot( Quaternion const &a, Quaternion const &b ) 
{
	return Dot( a.data, b.data );
}


//------------------------------------------------------------------------
// Works on normalized Quaternion - returns a non-normalized Quaternion
Quaternion Quaternion::QuaternionLog( Quaternion const &q ) 
{
	// Purely just the real part
	if (q.r >= .9999f) {
		return Quaternion( 0, 0, 0, 0 );
	}

	float half_angle = acosf(q.r);
	float s = sinf( half_angle );

	vec3 n =  q.i / s;
	return Quaternion( 0.0f, half_angle * n );
}

//------------------------------------------------------------------------
// Works on Quaternions of the form [0, a * i]
Quaternion Quaternion::QuaternionExp( Quaternion const &q ) 
{
	float half_angle = Magnitude( q.i );
	vec3 vec = vec3(0.0f);
	float r = cosf(half_angle);
	if (half_angle > 0.00001f) {
		vec3 n = q.i / half_angle;
		vec = sinf(half_angle) * n;
	}

	return Quaternion( r, vec );
}

//------------------------------------------------------------------------
Quaternion Quaternion::Quaternion::QuaternionScale( Quaternion const &q, float s ) 
{
	return Quaternion( s * q.r, s * q.i );
}

//------------------------------------------------------------------------
Quaternion Quaternion::( Quaternion const &q, float e ) 
{
	Quaternion ret = QuaternionExp( QuaternionScale( QuaternionLog( q ), e ) );
	ret.Normalize();
	return ret;
}

//------------------------------------------------------------------------
Quaternion Quaternion::Slerp( Quaternion const &a, Quaternion const &b, float const t_val ) 
{
	float time = Clamp01(t_val);
	float cos_angle = QuaternionDot( a, b );


	Quaternion start;
	if (cos_angle < 0.0f) {
		// If it's negative - it means it's going the long way
		// flip it.
		start = -a;
		cos_angle = -cos_angle;
	} else {
		start = a;
	}

	float f0, f1;
	if (cos_angle >= .9999f) {
		// very close - just linearly interpolate.
		f0 = 1.0f - time;
		f1 = time;
	} else {
		float sin_angle = sqrtf( 1.0f - cos_angle * cos_angle );
		float angle = atan2f( sin_angle, cos_angle );

		float den = 1.0f / sin_angle;
		f0 = sinf( (1.0f - time) * angle ) * den;
		f1 = sinf( time * angle ) * den;
	}

	Quaternion r0 = QuaternionScale( start, f0 );
	Quaternion r1 = QuaternionScale( b, f1 );
	return Quaternion( r0.r + r1.r, r0.i + r1.i );
}

Quaternion Quaternion::Lerp( Quaternion const &a, Quaternion const &b, float const &t ) 
{
	return Slerp( a, b, t );
}

//------------------------------------------------------------------------
Quaternion Quaternion::QuaternionRotateTorward( Quaternion const &start, Quaternion const &end, float max_angle_radians ) 
{
	float angle = acosf( QuaternionDot( start, end ) );
	if (angle < 0.0f) {
		angle = -angle;
	}

	if (FlEqual(angle, 0.0f)) {
		return end;
	}

	float t = Clamp01( max_angle_radians / angle );
	return Slerp( start, end, t );
}

//------------------------------------------------------------------------
bool Quaternion::NearEqual( Quaternion const &a, Quaternion const &b )
{
	float angle = QuaternionAngleInRadians(a, b);
	return angle < 0.001f;
}
*/