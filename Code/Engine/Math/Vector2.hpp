/************************************************************************/
/* Project: MP1 C27 A1 - Vector2, Basics, MathUtils
/* File: Vector2.hpp
/* Author: Andrew Chase
/* Date: August 30th, 2017
/* Bugs: None
/* Description: Class representing a vector of two elements, x and y
/************************************************************************/
#pragma once


//-----------------------------------------------------------------------------------------------
class Vector2
{

public:
	// Construction/Destruction
	~Vector2() {}											// destructor: do nothing (for speed)
	Vector2() {}											// default constructor: do nothing (for speed)
	Vector2( const Vector2& copyFrom );						// copy constructor (from another vec2)
	explicit Vector2( float initialX, float initialY );		// explicit constructor (from x, y)

															        // Operators
	const Vector2 operator+( const Vector2& vecToAdd ) const;		// vec2 + vec2
	const Vector2 operator-( const Vector2& vecToSubtract ) const;	// vec2 - vec2
	const Vector2 operator*( float uniformScale ) const;			// vec2 * float
	const Vector2 operator/( float inverseScale ) const;			// vec2 / float
	void operator+=( const Vector2& vecToAdd );						// vec2 += vec2
	void operator-=( const Vector2& vecToSubtract );				// vec2 -= vec2
	void operator*=( const float uniformScale );					// vec2 *= float
	void operator/=( const float uniformDivisor );					// vec2 /= float
	void operator=( const Vector2& copyFrom );						// vec2 = vec2
	bool operator==( const Vector2& compare ) const;				// vec2 == vec2
	bool operator!=( const Vector2& compare ) const;				// vec2 != vec2

	friend const Vector2 operator*( float uniformScale, const Vector2& vecToScale );	// float * vec2

	float GetLength() const;				// calculates the magnitude of the vector
	float GetLengthSquared() const;			// calculates the squared magnitude of the vector
	float NormalizeAndGetLength();			// Normalizes the vector and returns its original length
	Vector2 GetNormalized() const;			// calculates the normalized form of the vector
	float GetOrientationDegrees() const;

	static Vector2 MakeDirectionAtDegrees(float degrees);
	static Vector2 GetRandomVector(float desiredMagnitude);

	const static Vector2 ZERO;
	const static Vector2 ONES;

public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x;
	float y;
};


float GetDistance(const Vector2& a, const Vector2& b);			// Returns the distance between points a and b

float GetDistanceSquared(const Vector2& a, const Vector2& b);	// Returns the squared distance between points a and b
