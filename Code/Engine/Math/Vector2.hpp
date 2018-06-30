/************************************************************************/
/* Project: MP1 C27 A1 - Vector2, Basics, MathUtils
/* File: Vector2.hpp
/* Author: Andrew Chase
/* Date: August 30th, 2017
/* Bugs: None
/* Description: Class representing a vector of two elements, x and y
/************************************************************************/
#pragma once

class IntVector2;

//-----------------------------------------------------------------------------------------------
class Vector2
{

public:
	//-----Public Methods-----

	// Construction/Destruction
	~Vector2() {}											// destructor: do nothing (for speed)
	Vector2() {}											// default constructor: do nothing (for speed)
	Vector2( const Vector2& copyFrom );						// copy constructor (from another vec2)
	explicit Vector2( float initialX, float initialY );		// explicit constructor (from x, y)
	explicit Vector2( int initialX, int initialY );			// explicit int constructor
	explicit Vector2(const IntVector2& intVector);			// Creates a Vector2 from an IntVector2
	explicit Vector2(float initialValue);					// Creates a Vector2 from the single float

																		// Operators
	const	Vector2 operator+( const Vector2& vecToAdd ) const;			// vec2 + vec2
	const	Vector2 operator-( const Vector2& vecToSubtract ) const;	// vec2 - vec2
	const	Vector2 operator*( float uniformScale ) const;				// vec2 * float
	const	Vector2 operator/( float inverseScale ) const;				// vec2 / float
	void	operator+=( const Vector2& vecToAdd );						// vec2 += vec2
	void	operator-=( const Vector2& vecToSubtract );					// vec2 -= vec2
	void	operator*=( const float uniformScale );						// vec2 *= float
	void	operator/=( const float uniformDivisor );					// vec2 /= float
	void	operator=( const Vector2& copyFrom );						// vec2 = vec2
	bool	operator==( const Vector2& compare ) const;					// vec2 == vec2
	bool	operator!=( const Vector2& compare ) const;					// vec2 != vec2

	friend const Vector2 operator*( float uniformScale, const Vector2& vecToScale );	// float * vec2

	float	GetLength() const;							// Calculates the magnitude of the vector
	float	GetLengthSquared() const;					// Calculates the squared magnitude of the vector
	float	NormalizeAndGetLength();					// Normalizes the vector and returns its original length
	Vector2 GetNormalized() const;						// Calculates the normalized form of the vector
	float	GetOrientationDegrees() const;				// Returns the degree representation of this vector
	bool	SetFromText(const char* text);				// For turning a text representation into a Vector2

	static Vector2 MakeDirectionAtDegrees(float degrees);		// Returns the unit vector with the given orientation
	static Vector2 GetRandomVector(float desiredMagnitude);		// Returns a random vector with the given magnitude

	// Static constants
	const static Vector2 ZERO;
	const static Vector2 ONES;
	const static Vector2 DIRECTION_UP;
	const static Vector2 DIRECTION_DOWN;
	const static Vector2 DIRECTION_LEFT;
	const static Vector2 DIRECTION_RIGHT;


public:
	float x;
	float y;
};

//-----Standalone Functions-----

// Returns the distance between points a and b
float GetDistance(const Vector2& a, const Vector2& b);	

// Returns the squared distance between points a and b
float GetDistanceSquared(const Vector2& a, const Vector2& b);		

// Returns the projectOnto component of vectorToProject
const Vector2 GetProjectedVector(const Vector2& vectorToProject, const Vector2& projectOnto);

// Transforms (X, Y) vector into (I, J)
const Vector2 GetTransformedIntoBasis(const Vector2& originalVector, const Vector2& newBasisI, const Vector2& newBasisJ);	

// Transforms (I, J) back into X,Y space
const Vector2 GetTransformedOutOfBasis(const Vector2& vectorInBasis, const Vector2& oldBasisI, const Vector2& oldBasisJ);

// Returns the I,J projections of originalVector
void DecomposeVectorIntoBasis(const Vector2& originalVector, const Vector2& newBasisI, const Vector2& newBasisJ, Vector2& out_vectorAlongI, Vector2& out_vectorAlongJ);		

// Reflects vectorToReflect against a surface with normal 'normal'
const Vector2 Reflect(const Vector2& vectorToReflect, const Vector2& normal);

// Interpolates by interpolating x and y separately
const Vector2 Interpolate(const Vector2& start, const Vector2& end, float fractionTowardEnd);
