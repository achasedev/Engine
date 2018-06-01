/************************************************************************/
/* File: Matrix44.cpp
/* Author: Andrew Chase
/* Date: November 10th, 2017
/* Bugs: None
/* Description: Implementation of the Matrix44 class
/************************************************************************/
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor, from a 16-element float array (float[16])
//
Matrix44::Matrix44(const float* sixteenValuesBasisMajor)
{
	Ix = sixteenValuesBasisMajor[0];
	Iy = sixteenValuesBasisMajor[1];
	Iz = sixteenValuesBasisMajor[2];
	Iw = sixteenValuesBasisMajor[3];

	Jx = sixteenValuesBasisMajor[4];
	Jy = sixteenValuesBasisMajor[5];
	Jz = sixteenValuesBasisMajor[6];
	Jw = sixteenValuesBasisMajor[7];

	Kx = sixteenValuesBasisMajor[8];
	Ky = sixteenValuesBasisMajor[9];
	Kz = sixteenValuesBasisMajor[10];
	Kw = sixteenValuesBasisMajor[11];

	Tx = sixteenValuesBasisMajor[12];
	Ty = sixteenValuesBasisMajor[13];
	Tz = sixteenValuesBasisMajor[14];
	Tw = sixteenValuesBasisMajor[15];
}


//-----------------------------------------------------------------------------------------------
// Constructor, from the explicit I, J, K basis vectors and T translation vector
//
Matrix44::Matrix44(const Vector2& iBasis, const Vector2& jBasis, const Vector2& translation/*=Vector2(0.f,0.f) */)
{
	// Values are already initialized to identity, so just change the values we are passed

	Ix = iBasis.x;
	Iy = iBasis.y;

	Jx = jBasis.x;
	Jy = jBasis.y;

	Tx = translation.x;
	Ty = translation.y;
}


//-----------------------------------------------------------------------------------------------
// Transforms the 2D POSITION through matrix multiplication
// Written as if position2D is a Vector4, with z=0 and w=1
//
Vector2 Matrix44::TransformPosition2D(const Vector2& position2D)
{
	Vector2 result;

	result.x = (Ix * position2D.x) + (Jx * position2D.y) + Tx;
	result.y = (Iy * position2D.x) + (Jy * position2D.y) + Ty;

	// Returning a 2D vector, so no need to calculate z or w
	return result;
}


//-----------------------------------------------------------------------------------------------
// Transforms the 2D DISPLACEMENT through matrix multiplication
// Written as if displacement2D is a Vector4, with z=0 and w=0
//
Vector2 Matrix44::TransformDisplacement2D(const Vector2& displacement2D)
{
	Vector2 result;

	result.x = (Ix * displacement2D.x) + (Jx * displacement2D.y);
	result.y = (Iy * displacement2D.x) + (Jy * displacement2D.y);

	// Returning a 2D vector, so no need to calculate z or w
	return result;
}


//-----------------------------------------------------------------------------------------------
// Sets the values of this matrix to the identity matrix
//
void Matrix44::SetIdentity()
{
	Ix = 1.0f;
	Iy = 0.f;
	Iz = 0.f;
	Iw = 0.f;

	Jx = 0.f;
	Jy = 1.f;
	Jz = 0.f;
	Jw = 0.f;

	Kx = 0.f;
	Ky = 0.f;
	Kz = 1.f;
	Kw = 0.f;

	Tx = 0.f;
	Ty = 0.f;
	Tz = 0.f;
	Tw = 1.f;
}


//-----------------------------------------------------------------------------------------------
// Sets the values of this matrix to the ones specified in the 16-element float array provided
//
void Matrix44::SetValues(const float* sixteenValuesBasisMajor)
{
	Ix = sixteenValuesBasisMajor[0];
	Iy = sixteenValuesBasisMajor[1];
	Iz = sixteenValuesBasisMajor[2];
	Iw = sixteenValuesBasisMajor[3];

	Jx = sixteenValuesBasisMajor[4];
	Jy = sixteenValuesBasisMajor[5];
	Jz = sixteenValuesBasisMajor[6];
	Jw = sixteenValuesBasisMajor[7];

	Kx = sixteenValuesBasisMajor[8];
	Ky = sixteenValuesBasisMajor[9];
	Kz = sixteenValuesBasisMajor[10];
	Kw = sixteenValuesBasisMajor[11];

	Tx = sixteenValuesBasisMajor[12];
	Ty = sixteenValuesBasisMajor[13];
	Tz = sixteenValuesBasisMajor[14];
	Tw = sixteenValuesBasisMajor[15];
}


//-----------------------------------------------------------------------------------------------
// Appends/concatenates the provided matrix on the RIGHT of the current matrix
// i.e. thisMatrix = thisMatrix * matrixToAppend;
//
void Matrix44::Append(const Matrix44& matrixToAppend)
{
	// Copy old values for calculation
	Matrix44 oldValues = *this;

	// New I basis vector
	Ix = oldValues.Ix * matrixToAppend.Ix + oldValues.Jx * matrixToAppend.Iy + oldValues.Kx * matrixToAppend.Iz + oldValues.Tx * matrixToAppend.Iw;
	Iy = oldValues.Iy * matrixToAppend.Ix + oldValues.Jy * matrixToAppend.Iy + oldValues.Ky * matrixToAppend.Iz + oldValues.Ty * matrixToAppend.Iw;
	Iz = oldValues.Iz * matrixToAppend.Ix + oldValues.Jz * matrixToAppend.Iy + oldValues.Kz * matrixToAppend.Iz + oldValues.Tz * matrixToAppend.Iw;
	Iw = oldValues.Iw * matrixToAppend.Ix + oldValues.Jw * matrixToAppend.Iy + oldValues.Kw * matrixToAppend.Iz + oldValues.Tw * matrixToAppend.Iw;

	// New J basis vector
	Jx = oldValues.Ix * matrixToAppend.Jx + oldValues.Jx * matrixToAppend.Jy + oldValues.Kx * matrixToAppend.Jz + oldValues.Tx * matrixToAppend.Jw;
	Jy = oldValues.Iy * matrixToAppend.Jx + oldValues.Jy * matrixToAppend.Jy + oldValues.Ky * matrixToAppend.Jz + oldValues.Ty * matrixToAppend.Jw;
	Jz = oldValues.Iz * matrixToAppend.Jx + oldValues.Jz * matrixToAppend.Jy + oldValues.Kz * matrixToAppend.Jz + oldValues.Tz * matrixToAppend.Jw;
	Jw = oldValues.Iw * matrixToAppend.Jx + oldValues.Jw * matrixToAppend.Jy + oldValues.Kw * matrixToAppend.Jz + oldValues.Tw * matrixToAppend.Jw;

	// New K basis vector
	Kx = oldValues.Ix * matrixToAppend.Kx + oldValues.Jx * matrixToAppend.Ky + oldValues.Kx * matrixToAppend.Kz + oldValues.Tx * matrixToAppend.Kw;
	Ky = oldValues.Iy * matrixToAppend.Kx + oldValues.Jy * matrixToAppend.Ky + oldValues.Ky * matrixToAppend.Kz + oldValues.Ty * matrixToAppend.Kw;
	Kz = oldValues.Iz * matrixToAppend.Kx + oldValues.Jz * matrixToAppend.Ky + oldValues.Kz * matrixToAppend.Kz + oldValues.Tz * matrixToAppend.Kw;
	Kw = oldValues.Iw * matrixToAppend.Kx + oldValues.Jw * matrixToAppend.Ky + oldValues.Kw * matrixToAppend.Kz + oldValues.Tw * matrixToAppend.Kw;

	// New T basis vector
	Tx = oldValues.Ix * matrixToAppend.Tx + oldValues.Jx * matrixToAppend.Ty + oldValues.Kx * matrixToAppend.Tz + oldValues.Tx * matrixToAppend.Tw;
	Ty = oldValues.Iy * matrixToAppend.Tx + oldValues.Jy * matrixToAppend.Ty + oldValues.Ky * matrixToAppend.Tz + oldValues.Ty * matrixToAppend.Tw;
	Tz = oldValues.Iz * matrixToAppend.Tx + oldValues.Jz * matrixToAppend.Ty + oldValues.Kz * matrixToAppend.Tz + oldValues.Tz * matrixToAppend.Tw;
	Tw = oldValues.Iw * matrixToAppend.Tx + oldValues.Jw * matrixToAppend.Ty + oldValues.Kw * matrixToAppend.Tz + oldValues.Tw * matrixToAppend.Tw;
}


//-----------------------------------------------------------------------------------------------
// Appends/concatenates a rotation matrix representing the given 2D degree rotation to this matrix
// on the RIGHT
//
void Matrix44::RotateDegrees2D(float rotationDegreesAboutZ)
{
	Matrix44 rotationMatrix = MakeRotationDegrees2D(rotationDegreesAboutZ);

	Append(rotationMatrix);
}


//-----------------------------------------------------------------------------------------------
// Appends/concatenates a translation matrix representing the given 2D translation to this matrix
// on the RIGHT
//
void Matrix44::Translate2D(const Vector2& translation)
{
	Matrix44 translationMatrix = MakeTranslation2D(translation);

	Append(translationMatrix);
}


//-----------------------------------------------------------------------------------------------
// Appends/concatenates a uniform scale matrix representing the given XY scalar to this matrix
// on the RIGHT
//
void Matrix44::ScaleUniform2D(float scaleXY)
{
	Matrix44 uniformScaleMatrix = MakeScaleUniform2D(scaleXY);

	Append(uniformScaleMatrix);
}


//-----------------------------------------------------------------------------------------------
// Appends/concatenates a scale matrix representing the given XY scalars to this matrix
// on the RIGHT
//
void Matrix44::Scale2D(float scaleX, float scaleY)
{
	Matrix44 scaleMatrix = MakeScale2D(scaleX, scaleY);

	Append(scaleMatrix);
}


//-----------------------------------------------------------------------------------------------
// Constructs a 2D rotation matrix for the given 2D angle and returns it
//
Matrix44 Matrix44::MakeRotationDegrees2D(float rotationDegreesAboutZ)
{
	Matrix44 rotationMatrix;

	rotationMatrix.Ix = CosDegrees(rotationDegreesAboutZ);
	rotationMatrix.Iy = SinDegrees(rotationDegreesAboutZ);

	rotationMatrix.Jx = -SinDegrees(rotationDegreesAboutZ);
	rotationMatrix.Jy = CosDegrees(rotationDegreesAboutZ);

	return rotationMatrix;
}


//-----------------------------------------------------------------------------------------------
// Constructs a 2D translation matrix for the given 2D translation and returns it
//
Matrix44 Matrix44::MakeTranslation2D(const Vector2& translation)
{
	Matrix44 translationMatrix;

	translationMatrix.Tx = translation.x;
	translationMatrix.Ty = translation.y;

	return translationMatrix;
}


//-----------------------------------------------------------------------------------------------
// Constructs a 2D uniform scale matrix for the given 2D XY scale and returns it
//
Matrix44 Matrix44::MakeScaleUniform2D(float scaleXY)
{
	Matrix44 uniformScaleMatrix;

	uniformScaleMatrix.Ix = scaleXY;
	uniformScaleMatrix.Jy = scaleXY;

	return uniformScaleMatrix;
}


//-----------------------------------------------------------------------------------------------
// Constructs a 2D scale matrix for the given 2D XY scalars and returns it
//
Matrix44 Matrix44::MakeScale2D(float scaleX, float scaleY)
{
	Matrix44 scaleMatrix;

	scaleMatrix.Ix = scaleX;
	scaleMatrix.Jy = scaleY;

	return scaleMatrix;
}


//-----------------------------------------------------------------------------------------------
// Constructs a matrix that transforms points from orthographic space (within the bounds bottomLeft
// to topRight) into clips space (bounds (-1, -1) to (1, 1) with center at (0, 0))
// Also transforms the zDepth of any point from [0, 1] in ortho space to [-1, 1] in clip space
//
Matrix44 Matrix44::MakeOrtho2D(const Vector2& bottomLeft, const Vector2& topRight)
{
	//-----Make the Scale matrix-----
	float orthoWidth = topRight.x - bottomLeft.x;
	float orthoHeight = topRight.y - bottomLeft.y;

	float xScale = (2.f / orthoWidth);
	float yScale = (2.f / orthoHeight);

	Matrix44 scaleMatrix = MakeScale2D(xScale, yScale);

	//-----Make the Translation matrix-----
	Vector2 orthoCenter = bottomLeft + (0.5f * Vector2(orthoWidth, orthoHeight));
	Vector2 translation = -1.f * orthoCenter;

	Matrix44 translationMatrix = MakeTranslation2D(translation);

	//-----Appending the two together-----
	Matrix44 orthoMatrix = scaleMatrix;
	orthoMatrix.Append(translationMatrix);

	// ZDepth adjustments
	orthoMatrix.Kz = -2.0f;
	orthoMatrix.Tz = -1.0f;

	return orthoMatrix;
}
