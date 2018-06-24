/************************************************************************/
/* File: Camera.cpp
/* Author: Andrew Chase
/* Date: February 13th, 2018
/* Description: Implementation of the Camera class
/************************************************************************/
#include "Engine/Rendering/Core/Camera.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"

// Struct used for uniform buffer
struct CameraBufferData
{
	Matrix44 m_viewMatrix;
	Matrix44 m_projectionMatrix;

	Matrix44 m_cameraMatrix;

	Vector3 m_cameraRight;
	float	m_padding0;
	Vector3 m_cameraUp;
	float	m_padding1;
	Vector3 m_cameraForward;
	float	m_padding2;
	Vector3 m_cameraPosition;
	float	m_padding3;
};

//-----------------------------------------------------------------------------------------------
// Constructor
//
Camera::Camera()
	: m_drawOrder(0)
{
}


//-----------------------------------------------------------------------------------------------
// Moves the camera in world space, given the direction and speed
//
void Camera::TranslateWorld(const Vector3& translation)
{
	m_transform.TranslateWorld(translation);
	m_viewMatrix = InvertLookAtMatrix(m_transform.GetWorldMatrix());
}


//-----------------------------------------------------------------------------------------------
// Moves the camera in local space, given the direction and speed
//
void Camera::TranslateLocal(const Vector3& localTranslation)
{
	m_transform.TranslateLocal(localTranslation);
	m_viewMatrix = InvertLookAtMatrix(m_transform.GetWorldMatrix());
}


//-----------------------------------------------------------------------------------------------
// Rotates the camera by the given euler angle values - do it here ourselves so we can clamp it
//
void Camera::Rotate(const Vector3& rotation)
{
	Vector3 newRotation = m_transform.rotation.GetAsEulerAngles() + rotation;

	newRotation.x = GetAngleBetweenZeroThreeSixty(newRotation.x);
	newRotation.y = GetAngleBetweenZeroThreeSixty(newRotation.y);
	newRotation.z = GetAngleBetweenZeroThreeSixty(newRotation.z);

	if (newRotation.x > 90.f && newRotation.x < 180.f)
	{
		newRotation.x = 90.f;
	}

	if (newRotation.x > 180.f && newRotation.x < 270.f)
	{
		newRotation.x = 270.f;
	}

	m_transform.SetRotation(newRotation);

	m_viewMatrix = InvertLookAtMatrix(m_transform.GetWorldMatrix());
}


//-----------------------------------------------------------------------------------------------
// Sets the camera transform to that specified, and updates the view matrix
//
void Camera::SetTransform(const Transform& transform)
{
	m_transform = transform;

	m_viewMatrix = InvertLookAtMatrix(m_transform.GetWorldMatrix());
}


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
void Camera::FinalizeFrameBuffer()
{
	m_frameBuffer.Finalize();
}


//-----------------------------------------------------------------------------------------------
// Sets the camera to look at target from position, with the reference up-vector up
//
void Camera::LookAt(const Vector3& position, const Vector3& target, const Vector3& up /*= Vector3::DIRECTION_UP*/)
{
	Matrix44 cameraMatrix = Matrix44::MakeLookAt(position, target, up);

	m_transform.position = position;
	m_transform.rotation = Quaternion::FromEuler(Matrix44::ExtractRotationDegrees(cameraMatrix));

	m_transform.SetModelMatrix(cameraMatrix);
	m_viewMatrix = InvertLookAtMatrix(cameraMatrix);
}


//-----------------------------------------------------------------------------------------------
// Sets the camera matrix to the one passed
//
void Camera::SetCameraMatrix(const Matrix44& cameraMatrix)
{
	m_transform.SetModelMatrix(cameraMatrix);
	m_viewMatrix = InvertLookAtMatrix(cameraMatrix);
}


//-----------------------------------------------------------------------------------------------
// Sets the view matrix to the one passed
//
void Camera::SetViewMatrix(const Matrix44& viewMatrix)
{
	m_viewMatrix = viewMatrix;
	m_transform.SetModelMatrix(InvertLookAtMatrix(viewMatrix));
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
void Camera::SetProjectionOrtho(float width, float height, float nearZ, float farZ)
{
	m_orthoSize = height;
	m_nearClipZ = nearZ;
	m_farClipZ = farZ;
	m_projectionMatrix = Matrix44::MakeOrtho(-width / 2.f, width / 2.f, -height / 2.f, height / 2.f, nearZ, farZ);
}


//-----------------------------------------------------------------------------------------------
// Sets this camera to be a perspective projection with the given params
//
void Camera::SetProjectionPerspective(float fovDegrees, float nearZ, float farZ)
{
	m_fov = fovDegrees;
	m_nearClipZ = nearZ;
	m_farClipZ = farZ;
	m_projectionMatrix = Matrix44::MakePerspective(fovDegrees, nearZ, farZ);
}


//-----------------------------------------------------------------------------------------------
// Sets the ortho size of the camera to the one given, not taking into consideration size
// Also recalculates the ortho projection matrix
//
void Camera::SetOrthoSize(float newSize)
{
	float width = m_frameBuffer.GetAspect() * newSize;
	SetProjectionOrtho(width, newSize, m_nearClipZ, m_farClipZ);
}


//-----------------------------------------------------------------------------------------------
// Adjusts the camera's ortho size by adding the modifier to it, clamping to the size limits
// Also recalculates the ortho projection matrix
//
void Camera::AdjustOrthoSize(float additiveModifier)
{
	m_orthoSize = ClampFloat(m_orthoSize + additiveModifier, m_orthoSizeLimits.min, m_orthoSizeLimits.max);
	float width = m_frameBuffer.GetAspect() * m_orthoSize;
	SetProjectionOrtho(width, m_orthoSize, m_nearClipZ, m_farClipZ);
}


//-----------------------------------------------------------------------------------------------
// Sets the ortho size limits to the ones given
//
void Camera::SetOrthoSizeLimits(float min, float max)
{
	m_orthoSizeLimits = FloatRange(min, max);
}


//-----------------------------------------------------------------------------------------------
// Sets the draw order for the camera, used in ForwardRenderPath sorting
//
void Camera::SetDrawOrder(unsigned int order)
{
	m_drawOrder = order;
}


//-----------------------------------------------------------------------------------------------
// Update the camera's uniform buffer with the camera's current state
//
void Camera::FinalizeUniformBuffer()
{
	CameraBufferData bufferData;

	bufferData.m_viewMatrix = m_viewMatrix;
	bufferData.m_projectionMatrix = m_projectionMatrix;
	bufferData.m_cameraMatrix = m_transform.GetWorldMatrix();

	bufferData.m_cameraRight	= GetRightVector();
	bufferData.m_cameraUp		= GetUpVector();
	bufferData.m_cameraForward	= GetForwardVector();
	bufferData.m_cameraPosition = m_transform.position;

	m_uniformBuffer.SetCPUAndGPUData(sizeof(CameraBufferData), &bufferData);
}


//-----------------------------------------------------------------------------------------------
// Returns this camera's gpu-side uniform buffer handle
//
GLuint Camera::GetUniformBufferHandle() const
{
	return m_uniformBuffer.GetHandle();
}


//-----------------------------------------------------------------------------------------------
// Returns the camera matrix (Camera's model matrix, inverse of the view matrix)
// (local to world)
//
Matrix44 Camera::GetCameraMatrix() const
{
	return m_transform.GetWorldMatrix();
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
// Returns the position of the camera
//
Vector3 Camera::GetPosition() const
{
	return m_transform.position;
}


//-----------------------------------------------------------------------------------------------
// Returns the rotation of the camera's transform
//
Vector3 Camera::GetRotation() const
{
	return m_transform.rotation.GetAsEulerAngles();
}


//-----------------------------------------------------------------------------------------------
// Returns the forward (K) vector of the camera's transform
//
Vector3 Camera::GetForwardVector() const
{
	return m_transform.GetWorldMatrix().GetKVector().xyz();
}


//-----------------------------------------------------------------------------------------------
// Returns the right (I) vector of the camera's transform
//
Vector3 Camera::GetRightVector() const
{
	return m_transform.GetWorldMatrix().GetIVector().xyz();
}


//-----------------------------------------------------------------------------------------------
// Returns the up (J) vector of the camera's transform
//
Vector3 Camera::GetUpVector() const
{
	return m_transform.GetWorldMatrix().GetJVector().xyz();
}


//-----------------------------------------------------------------------------------------------
// Returns the GPU handle of this camera's FrameBuffer member
//
unsigned int Camera::GetFrameBufferHandle() const
{
	return m_frameBuffer.GetHandle();
}


//-----------------------------------------------------------------------------------------------
// Returns the draw order of the camera
//
unsigned int Camera::GetDrawOrder() const
{
	return m_drawOrder;
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
