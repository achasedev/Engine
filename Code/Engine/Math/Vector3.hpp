/************************************************************************/
/* File: Vector3.hpp
/* Author: Andrew Chase
/* Date: November 7th, 2017
/* Bugs: None
/* Description: Class representing a vector of three elements, x, y, and z
/************************************************************************/
#pragma once

class IntVector3;
class Vector2;

//-----------------------------------------------------------------------------------------------
class Vector3
{

public:
	//-----Public Methods-----

	// Construction/Destruction
	~Vector3() {}															// destructor: do nothing (for speed)
	Vector3() {}															// default constructor: do nothing (for speed)
	Vector3( const Vector3& copyFrom );										// copy constructor (from another vec2)
	explicit Vector3( float initialX, float initialY, float initialZ);		// explicit constructor (from x, y)
	explicit Vector3(int initialX, int initialY, int initialZ);		// explicit constructor (from x, y)
	explicit Vector3(const IntVector3& intVector);							// constructor from an IntVector3
	Vector3(float value);

	// Operators
	const	Vector3 operator+( const Vector3& vecToAdd ) const;				// vec3 + vec3
	const	Vector3 operator-( const Vector3& vecToSubtract ) const;		// vec3 - vec3
	const	Vector3 operator*( float uniformScale ) const;					// vec3 * float
	const	Vector3 operator/( float inverseScale ) const;					// vec3 / float
	void	operator+=( const Vector3& vecToAdd );							// vec3 += vec3
	void	operator-=( const Vector3& vecToSubtract );						// vec3 -= vec3
	void	operator*=( const float uniformScale );							// vec3 *= float
	void	operator/=( const float uniformDivisor );						// vec3 /= float
	void	operator=( const Vector3& copyFrom );							// vec3 = vec3
	bool	operator==( const Vector3& compare ) const;						// vec3 == vec3
	bool	operator!=( const Vector3& compare ) const;						// vec3 != vec3

	friend const Vector3 operator*( float uniformScale, const Vector3& vecToScale );	// float * vec2

	float	GetLength() const;									// Calculates the magnitude of the vector
	float	GetLengthSquared() const;							// Calculates the squared magnitude of the vector
	float	NormalizeAndGetLength();							// Normalizes the vector and returns its original length
	Vector3 GetNormalized() const;								// Calculates the normalized form of the vector
	bool	SetFromText(const char* text);						// For turning a text representation into a Vector2

	Vector2 xy() const;
	Vector2 xz() const;

	static Vector3 GetRandomVector(float desiredMagnitude);		// Returns a random vector with the given magnitude
	static Vector3 Slerp(const Vector3& start, const Vector3& end, float percent);

																
	// Static constants
	const static Vector3 ZERO;
	const static Vector3 ONES;
	const static Vector3 Y_AXIS;
	const static Vector3 MINUS_Y_AXIS;
	const static Vector3 MINUS_X_AXIS;
	const static Vector3 X_AXIS;
	const static Vector3 Z_AXIS;
	const static Vector3 MINUS_Z_AXIS;

public:
	float x;
	float y;
	float z;
};

const Vector3 Interpolate(const Vector3& start, const Vector3& end, float fractionTowardEnd);
