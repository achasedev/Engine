/************************************************************************/
/* File: Matrix44.cpp
/* Author: Andrew Chase
/* Date: November 10th, 2017
/* Bugs: None
/* Description: Implementation of the Matrix44 class
/************************************************************************/
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

const Matrix44 Matrix44::IDENTITY = Matrix44();

//-----------------------------------------------------------------------------------------------
// Default constructor, sets to Identity
//
Matrix44::Matrix44()
{
	SetIdentity();
}


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
Matrix44::Matrix44(const Vector3& iBasis, const Vector3& jBasis, const Vector3& kBasis, const Vector3& translation/*=Vector3::ZERO*/)
	: Matrix44()
{
	Ix = iBasis.x;
	Iy = iBasis.y;
	Iz = iBasis.z;

	Jx = jBasis.x;
	Jy = jBasis.y;
	Jz = jBasis.z;

	Kx = kBasis.x;
	Ky = kBasis.y;
	Kz = kBasis.z;

	Tx = translation.x;
	Ty = translation.y;
	Tz = translation.z;
}


//-----------------------------------------------------------------------------------------------
// Operator for multiplying matrices
//
const Matrix44 Matrix44::operator*(const Matrix44& rightMat) const
{
	Matrix44 result = *this;
	result.Append(rightMat);

	return result;
}


//-----------------------------------------------------------------------------------------------
// Transforms the given vector by this matrix and returns the result
//
const Vector4 Matrix44::operator*(const Vector4& rightVector) const
{
	return Transform(rightVector);
}


//-----------------------------------------------------------------------------------------------
// Applies a component-wise scale to the entire matrix
//
const Matrix44 Matrix44::operator*(float scaler) const
{
	Matrix44 result = *this;

	result.Ix *= scaler;
	result.Iy *= scaler;
	result.Iz *= scaler;
	result.Iw *= scaler;

	result.Jx *= scaler;
	result.Jy *= scaler;
	result.Jz *= scaler;
	result.Jw *= scaler;

	result.Kx *= scaler;
	result.Ky *= scaler;
	result.Kz *= scaler;
	result.Kw *= scaler;

	result.Tx *= scaler;
	result.Ty *= scaler;
	result.Tz *= scaler;
	result.Tw *= scaler;

	return result;
}


//-----------------------------------------------------------------------------------------------
// Transforms the given Vector3 by this matrix, treating it as a point (z = 0, w = 1)
//
Vector4 Matrix44::TransformPoint(const Vector2& point) const
{
	Vector4 pointToTransform = Vector4(point.x, point.y, 0.f, 1.0f);
	return Transform(pointToTransform);
}


//-----------------------------------------------------------------------------------------------
// Transforms the given Vector3 by this matrix, treating it as a point (w = 1)
//
Vector4 Matrix44::TransformPoint(const Vector3& point) const
{
	Vector4 pointToTransform = Vector4(point.x, point.y, point.z, 1.0f);
	return Transform(pointToTransform);
}


//-----------------------------------------------------------------------------------------------
// Transforms the given Vector2 by this matrix, treating it as a vector (z = 0, w = 0)
//
Vector4 Matrix44::TransformVector(const Vector2& vector) const
{
	Vector4 vectorToTransform = Vector4(vector.x, vector.y, 0.f, 0.0f);
	return Transform(vectorToTransform);
}


//-----------------------------------------------------------------------------------------------
// Transforms the given Vector3 by this matrix, treating it as a vector (w = 0)
//
Vector4 Matrix44::TransformVector(const Vector3& vector) const
{
	Vector4 vectorToTransform = Vector4(vector.x, vector.y, vector.z, 0.0f);
	return Transform(vectorToTransform);
}


//-----------------------------------------------------------------------------------------------
// Transforms the given Vector4 by this matrix
//
Vector4 Matrix44::Transform(const Vector4& vector) const
{
	Vector4 result;

	result.x = DotProduct(GetXVector(), vector);
	result.y = DotProduct(GetYVector(), vector);
	result.z = DotProduct(GetZVector(), vector);
	result.w = DotProduct(GetWVector(), vector);

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
// Transposes the matrix (flips over the diagonal)
//
void Matrix44::Transpose()
{
	Matrix44 original = *this;

	Iy = original.Jx;
	Jx = original.Iy;

	Iz = original.Kx;
	Kx = original.Iz;

	Iw = original.Tx;
	Tx = original.Iw;

	Jz = original.Ky;
	Ky = original.Jz;

	Jw = original.Ty;
	Ty = original.Jw;

	Kw = original.Tz;
	Tz = original.Kw;
}


//-----------------------------------------------------------------------------------------------
// Returns the I vector of the matrix
//
Vector4 Matrix44::GetIVector() const
{
	return Vector4(Ix, Iy, Iz, Iw);
}


//-----------------------------------------------------------------------------------------------
// Returns the J vector of the matrix
//
Vector4 Matrix44::GetJVector() const
{
	return Vector4(Jx, Jy, Jz, Jw);
}


//-----------------------------------------------------------------------------------------------
// Returns the K vector of the matrix
//
Vector4 Matrix44::GetKVector() const
{
	return Vector4(Kx, Ky, Kz, Kw);
}


//-----------------------------------------------------------------------------------------------
// Returns the T vector of the matrix
//
Vector4 Matrix44::GetTVector() const
{
	return Vector4(Tx, Ty, Tz, Tw);
}


//-----------------------------------------------------------------------------------------------
// Returns the X vector of the matrix
//
Vector4 Matrix44::GetXVector() const
{
	return Vector4(Ix, Jx, Kx, Tx);
}


//-----------------------------------------------------------------------------------------------
// Returns the Y vector of the matrix
//
Vector4 Matrix44::GetYVector() const
{
	return Vector4(Iy, Jy, Ky, Ty);
}


//-----------------------------------------------------------------------------------------------
// Returns the Z vector of the matrix
//
Vector4 Matrix44::GetZVector() const
{
	return Vector4(Iz, Jz, Kz, Tz);
}


//-----------------------------------------------------------------------------------------------
// Returns the W vector of the matrix
//
Vector4 Matrix44::GetWVector() const
{
	return Vector4(Iw, Jw, Kw, Tw);
}


//-----------------------------------------------------------------------------------------------
// Constructs a 2D rotation matrix for the given 2D angle and returns it
//
Matrix44 Matrix44::MakeRotation(const Vector3& rotation)
{
	// Roll matrix - rotation about z
	Matrix44 rollMatrix;

	rollMatrix.Ix = CosDegrees(rotation.z);
	rollMatrix.Iy = SinDegrees(rotation.z);

	rollMatrix.Jx = -SinDegrees(rotation.z);
	rollMatrix.Jy = CosDegrees(rotation.z);

	// Pitch matrix - rotation about x
	Matrix44 pitchMatrix;

	pitchMatrix.Ix = CosDegrees(rotation.x);
	pitchMatrix.Iz = -SinDegrees(rotation.x);

	pitchMatrix.Kx = SinDegrees(rotation.x);
	pitchMatrix.Kz = CosDegrees(rotation.x);

	// Yaw matrix - rotation about y
	Matrix44 yawMatrix;

	yawMatrix.Jy = CosDegrees(rotation.y);
	yawMatrix.Jz = SinDegrees(rotation.y);

	yawMatrix.Ky = -SinDegrees(rotation.y);
	yawMatrix.Kz = CosDegrees(rotation.y);

	// Concatenate and return
	return yawMatrix * pitchMatrix * rollMatrix;
}


//-----------------------------------------------------------------------------------------------
// Constructs a 2D translation matrix for the given 2D translation and returns it
//
Matrix44 Matrix44::MakeTranslation(const Vector3& translation)
{
	Matrix44 translationMatrix;

	translationMatrix.Tx = translation.x;
	translationMatrix.Ty = translation.y;
	translationMatrix.Tz = translation.z;

	return translationMatrix;
}


//-----------------------------------------------------------------------------------------------
// Constructs a scale matrix given the 3D scale and returns it
//
Matrix44 Matrix44::MakeScale(const Vector3& scale)
{
	Matrix44 scaleMatrix;

	scaleMatrix.Ix = scale.x;
	scaleMatrix.Jy = scale.y;
	scaleMatrix.Kz = scale.z;

	return scaleMatrix;
}


//-----------------------------------------------------------------------------------------------
// Constructs a uniform scale matrix given the scale
//
Matrix44 Matrix44::MakeScaleUniform(float uniformScale)
{
	return MakeScale(Vector3(uniformScale, uniformScale, uniformScale));
}


//-----------------------------------------------------------------------------------------------
// Constructs a model matrix given the transform's translation, rotation, and scale
//
Matrix44 Matrix44::MakeModelMatrix(const Vector3& translation, const Vector3& rotation, const Vector3& scale)
{
	Matrix44 translationMatrix	= MakeTranslation(translation);
	Matrix44 rotationMatrix		= MakeRotation(rotation);
	Matrix44 scaleMatrix		= MakeScale(scale);

	Matrix44 result = translationMatrix * rotationMatrix * scaleMatrix;

	return result;
}


//-----------------------------------------------------------------------------------------------
// Constructs an orthographic matrix give the axis bounds
//
Matrix44 Matrix44::MakeOrtho(float leftX, float rightX, float bottomY, float topY, float nearZ, float farZ)
{
	Matrix44 orthoMatrix;

	orthoMatrix.Ix = 2.f / (rightX - leftX);
	orthoMatrix.Jy = 2.f / (topY - bottomY);
	orthoMatrix.Kz = 2.f / (farZ - nearZ);

	orthoMatrix.Tx = -(rightX + leftX)/(rightX - leftX);
	orthoMatrix.Ty = -(topY + bottomY)/(topY - bottomY);
	orthoMatrix.Tz = -(farZ + nearZ)/(farZ - nearZ);

	return orthoMatrix;
}


//-----------------------------------------------------------------------------------------------
// Constructs a matrix that transforms points from orthographic space (within the bounds bottomLeft
// to topRight and clipZ) into clips space (bounds (-1, -1) to (1, 1) with center at (0, 0))
//
Matrix44 Matrix44::MakeOrtho(const Vector2& bottomLeft, const Vector2& topRight, float nearZ/*=0.f*/, float farZ/*=1.0f*/)
{
	return MakeOrtho(bottomLeft.x, topRight.x, bottomLeft.y, topRight.y, nearZ, farZ);
}


//-----------------------------------------------------------------------------------------------
// Constructs a Look At matrix from position looking at target with the given up reference vector
//
Matrix44 Matrix44::MakeLookAt(const Vector3& position, const Vector3& target, const Vector3& referenceUp /*= Vector3::DIRECTION_UP*/)
{
	// Edge case - Target and position are the same position, then just look world forward
	Vector3 forward;
	if (position == target)
	{
		forward = Vector3::DIRECTION_FORWARD;
	}
	else
	{
		forward = (target - position).GetNormalized();
	}

	// Edge case - check if the forward happens to be the reference up vector, and if so just set right to the reference up
	ASSERT_OR_DIE(forward != referenceUp, "Error: Matrix44::LookAt() had forward and up vector matched.");

	Vector3 right = CrossProduct(referenceUp, forward);
	right.NormalizeAndGetLength();

	Vector3 lookUp = CrossProduct(forward, right);
	
	return Matrix44(right, lookUp, forward, position);
}
