/************************************************************************/
/* File: Matrix44.hpp
/* Author: Andrew Chase
/* Date: November 10th, 2017
/* Bugs: None
/* Description: Class to represent a homogeneous 3D matrix
/************************************************************************/
#pragma once
#include "Engine/Math/Vector2.hpp"

class Matrix44
{
public:
	//-----Public Methods-----

	Matrix44() {} // default-construct to Identity matrix (via variable initialization), for speed
	explicit Matrix44( const float* sixteenValuesBasisMajor ); // float[16] array in order Ix, Iy...
	explicit Matrix44( const Vector2& iBasis, const Vector2& jBasis, const Vector2& translation=Vector2(0.f,0.f) );

	// Accessors
	Vector2 TransformPosition2D( const Vector2& position2D );			// Written assuming z=0, w=1
	Vector2 TransformDisplacement2D( const Vector2& displacement2D );	// Written assuming z=0, w=0

	// Mutators
	void SetIdentity();
	void SetValues( const float* sixteenValuesBasisMajor );				// float[16] array in order Ix, Iy...
	void Append( const Matrix44& matrixToAppend );						// a.k.a. Concatenate (right-multiply)
	void RotateDegrees2D( float rotationDegreesAboutZ );
	void Translate2D( const Vector2& translation );
	void ScaleUniform2D( float scaleXY );
	void Scale2D( float scaleX, float scaleY );

	// Producers
	static Matrix44 MakeRotationDegrees2D( float rotationDegreesAboutZ );
	static Matrix44 MakeTranslation2D( const Vector2& translation );
	static Matrix44 MakeScaleUniform2D( float scaleXY );
	static Matrix44 MakeScale2D( float scaleX, float scaleY );
	static Matrix44 MakeOrtho2D( const Vector2& bottomLeft, const Vector2& topRight );

public:
	//-----Public data----- 
	// 16 floats to represent the 4x4 Basis-major ordered matrix
	// Values are initialized to the identity matrix

	// I basis vector
	float Ix = 1.0f;
	float Iy = 0.f;
	float Iz = 0.f;
	float Iw = 0.f;

	// J basis vector
	float Jx = 0.f;
	float Jy = 1.f;
	float Jz = 0.f;
	float Jw = 0.f;

	// K basis vector
	float Kx = 0.f;
	float Ky = 0.f;
	float Kz = 1.f;
	float Kw = 0.f;

	// T (translation) vector
	float Tx = 0.f;
	float Ty = 0.f;
	float Tz = 0.f;
	float Tw = 1.f;

};
