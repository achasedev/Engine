/************************************************************************/
/* File: Vector4.hpp
/* Author: Andrew Chase
/* Date: February 15th, 2017
/* Bugs: None
/* Description: Class representing a vector of four elements, x, y, z, and w
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"

//-----------------------------------------------------------------------------------------------
class Vector4
{

public:
	//-----Public Methods-----

	// Construction/Destruction
	~Vector4() {}																			// destructor: do nothing (for speed)
	Vector4() {}																			// default constructor: do nothing (for speed)
	Vector4( const Vector4& copyFrom );														// copy constructor (from another vec2)
	explicit Vector4( float initialX, float initialY, float initialZ, float initialW);		// explicit constructor (from x, y, z, w)

	explicit Vector4(const Vector3& xyzVector, float wValue);
																			// Operators
	const	Vector4 operator+( const Vector4& vecToAdd ) const;				// vec3 + vec3
	const	Vector4 operator-( const Vector4& vecToSubtract ) const;		// vec3 - vec3
	const	Vector4 operator*( float uniformScale ) const;					// vec3 * float
	const	Vector4 operator/( float inverseScale ) const;					// vec3 / float
	void	operator+=( const Vector4& vecToAdd );							// vec3 += vec3
	void	operator-=( const Vector4& vecToSubtract );						// vec3 -= vec3
	void	operator*=( const float uniformScale );							// vec3 *= float
	void	operator/=( const float uniformDivisor );						// vec3 /= float
	void	operator=( const Vector4& copyFrom );							// vec3 = vec3
	bool	operator==( const Vector4& compare ) const;						// vec3 == vec3
	bool	operator!=( const Vector4& compare ) const;						// vec3 != vec3

	friend const Vector4 operator*( float uniformScale, const Vector4& vecToScale );	// float * vec2

	float	GetLength() const;									// Calculates the magnitude of the vector
	float	GetLengthSquared() const;							// Calculates the squared magnitude of the vector
	float	NormalizeAndGetLength();							// Normalizes the vector and returns its original length
	Vector4 GetNormalized() const;								// Calculates the normalized form of the vector

	static Vector4 GetRandomVector(float desiredMagnitude);		// Returns a random vector with the given magnitude

	Vector2 xz() const;
	Vector3 xyz() const;

	// Static constants
	const static Vector4 ZERO;
	const static Vector4 ONES;
	const static Vector4 DIRECTION_UP;
	const static Vector4 DIRECTION_DOWN;
	const static Vector4 DIRECTION_LEFT;
	const static Vector4 DIRECTION_RIGHT;
	const static Vector4 DIRECTION_FORWARD;
	const static Vector4 DIRECTION_BACK;

public:
	float x;
	float y;
	float z;
	float w;
};

const Vector4 Interpolate(const Vector4& start, const Vector4& end, float fractionTowardEnd);
