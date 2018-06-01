/************************************************************************/
/* File: Camera.cpp
/* Author: Andrew Chase
/* Date: February 13th, 2018
/* Description: Implementation of the Camera class
/************************************************************************/
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
// Sets the color target of the Camera's FrameBuffer to the one passed
//
void Camera::SetColorTarget(Texture* colorTarget)
{
	m_frameBuffer.SetColorTarget(colorTarget);
}


//-----------------------------------------------------------------------------------------------
// Sets the depth target of the Camera's FrameBuffer to the one passed
//
void Camera::SetDepthTarget(Texture* depthTarget)
{
	m_frameBuffer.SetDepthTarget(depthTarget);
}


//-----------------------------------------------------------------------------------------------
// Finalizes the Camera's FrameBuffer
//
void Camera::Finalize()
{
	m_frameBuffer.Finalize();
}


//-----------------------------------------------------------------------------------------------
// Sets the camera to look at target from position, with the reference up-vector up
//
void Camera::LookAt(const Vector3& position, const Vector3& target, const Vector3& up /*= Vector3::DIRECTION_UP*/)
{
	m_cameraMatrix	= Matrix44::MakeLookAt(position, target, up);
	m_viewMatrix	= InvertLookAtMatrix(m_cameraMatrix);
}


//-----------------------------------------------------------------------------------------------
// Sets the camera matrix to the one passed
//
void Camera::SetCameraMatrix(const Matrix44& cameraMatrix)
{
	m_cameraMatrix = cameraMatrix;
}


//-----------------------------------------------------------------------------------------------
// Sets the view matrix to the one passed
//
void Camera::SetViewMatrix(const Matrix44& viewMatrix)
{
	m_viewMatrix = viewMatrix;
}


//-----------------------------------------------------------------------------------------------
// Sets the projection matrix to the one passed
//
void Camera::SetProjection(const Matrix44& projection)
{
	m_projectionMatrix = projection;
}


//-----------------------------------------------------------------------------------------------
// Sets the camera matrix to an orthographic projection given the ortho parameters
//
void Camera::SetProjectionOrtho(float height, float nearZ, float farZ)
{
	m_orthoSize = height;
	m_nearClipZ = nearZ;
	m_farClipZ = farZ;
	float width = Window::GetInstance()->GetWindowAspect() * height;
	m_projectionMatrix = Matrix44::MakeOrtho(-width / 2.f, width / 2.f, -height / 2.f, height / 2.f, nearZ, farZ);
}


//-----------------------------------------------------------------------------------------------
// Sets the ortho size of the camera to the one given, not taking into consideration size
// Also recalculates the ortho projection matrix
//
void Camera::SetOrthoSize(float newSize)
{
	SetProjectionOrtho(newSize, m_nearClipZ, m_farClipZ);
}


//-----------------------------------------------------------------------------------------------
// Adjusts the camera's ortho size by adding the modifier to it, clamping to the size limits
// Also recalculates the ortho projection matrix
//
void Camera::AdjustOrthoSize(float additiveModifier)
{
	m_orthoSize = ClampFloat(m_orthoSize + additiveModifier, m_orthoSizeLimits.min, m_orthoSizeLimits.max);
	SetProjectionOrtho(m_orthoSize, m_nearClipZ, m_farClipZ);
}


//-----------------------------------------------------------------------------------------------
// Sets the ortho size limits to the ones given
//
void Camera::SetOrthoSizeLimits(float min, float max)
{
	m_orthoSizeLimits = FloatRange(min, max);
}


//-----------------------------------------------------------------------------------------------
// Returns the camera matrix (Camera's model matrix, inverse of the view matrix)
// (local to world)
//
Matrix44 Camera::GetCameraMatrix() const
{
	return m_cameraMatrix;
}


//-----------------------------------------------------------------------------------------------
// Returns the view matrix (world to camera local, inverse of the camera matrix)
//
Matrix44 Camera::GetViewMatrix() const
{
	return m_viewMatrix;
}


//-----------------------------------------------------------------------------------------------
// Returns the projection matrix of the Camera
//
Matrix44 Camera::GetProjectionMatrix() const
{
	return m_projectionMatrix;
}


//-----------------------------------------------------------------------------------------------
// Returns the forward (K) vector of the camera's transform
//
Vector3 Camera::GetForwardVector() const
{
	return Vector3(m_cameraMatrix.Kx, m_cameraMatrix.Ky, m_cameraMatrix.Kz);
}


//-----------------------------------------------------------------------------------------------
// Returns the right (I) vector of the camera's transform
//
Vector3 Camera::GetRightVector() const
{
	return Vector3(m_cameraMatrix.Ix, m_cameraMatrix.Iy, m_cameraMatrix.Iz);

}


//-----------------------------------------------------------------------------------------------
// Returns the up (J) vector of the camera's transform
//
Vector3 Camera::GetUpVector() const
{
	return Vector3(m_cameraMatrix.Jx, m_cameraMatrix.Jy, m_cameraMatrix.Jz);

}


//-----------------------------------------------------------------------------------------------
// Returns the GPU handle of this camera's FrameBuffer member
//
unsigned int Camera::GetFrameBufferHandle() const
{
	return m_frameBuffer.GetHandle();
}


//-----------------------------------------------------------------------------------------------
// Inverts the lookat matrix given, used to construct the view matrix from the camera matrix
//
Matrix44 Camera::InvertLookAtMatrix(const Matrix44& lookAt) const
{
	Matrix44 rotation = lookAt;

	rotation.Tx = 0.f;
	rotation.Ty = 0.f;
	rotation.Tz = 0.f;

	rotation.Transpose();

	Matrix44 translation = Matrix44::MakeTranslation(Vector3(-lookAt.Tx, -lookAt.Ty, -lookAt.Tz));
	rotation.Append(translation);
	return rotation;
}
