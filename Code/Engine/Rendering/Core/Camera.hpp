/************************************************************************/
/* File: Camera.hpp
/* Author: Andrew Chase
/* Date: February 13th, 2018
/* Description: Class to represent a draw-to buffer with projection
/************************************************************************/
#pragma once
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Rendering/Buffers/FrameBuffer.hpp"
#include "Engine/Rendering/Buffers/UniformBuffer.hpp"

class Texture;
class Matrix44;

class Camera
{
public:
	//-----Public Methods-----

	Camera();
	~Camera() {}

	// Movement
	void					TranslateWorld(const Vector3& worldTranslation);
	void					TranslateLocal(const Vector3& localTranslation);

	Vector3					Rotate(const Vector3& deltaRotation);
	void					SetRotation(const Vector3& newRotation);

	void					SetTransform(const Transform& transform);

	void					SetColorTarget(Texture* color_target);
	void					SetDepthTarget(Texture* depth_target);

	// Buffers
	void					FinalizeFrameBuffer();
	void					FinalizeUniformBuffer();
	GLuint					GetUniformBufferHandle() const;

	// Model setters
	void					LookAt(const Vector3& position, const Vector3& target, const Vector3& up = Vector3::Y_AXIS);
	void					SetCameraMatrix(const Matrix44& cameraMatrix);
	void					SetViewMatrix(const Matrix44& viewMatrix);

	// Projection settings
	void					SetProjection(const Matrix44& projection); 
	void					SetProjectionOrtho(float width, float height, float nearZ, float farZ); 
	void					SetProjectionPerspective(float fovDegrees, float nearZ, float farZ);

	// Other setters
	void					SetOrthoSize(float newSize);
	void					AdjustOrthoSize(float additiveModifier);
	void					SetOrthoSizeLimits(float min, float max);
	void					SetChangeOfBasisMatrix(const Matrix44& changeOfBasisMatrix);

	void					SetDrawOrder(unsigned int order);

	Matrix44				GetCameraMatrix() const;
	Matrix44				GetViewMatrix() const;
	Matrix44				GetProjectionMatrix() const;

	Vector3					GetPosition() const;
	Vector3					GetRotation() const;

	Vector3					GetZVector() const;
	Vector3					GetXVector() const;
	Vector3					GetYVector() const;

	unsigned int			GetFrameBufferHandle() const;
	unsigned int			GetDrawOrder() const;

private:
	//-----Private Methods-----

	Matrix44 InvertLookAtMatrix(const Matrix44& lookAt) const;	// Inverts the TR lookAt matrix, for the camera view matrix


public:
	//-----Public Data-----

	FrameBuffer m_frameBuffer;		// Color and depth targets of this camera


private:
	//-----Private Data-----

	mutable Transform m_transform;
	
	// All initialized to identity
	Matrix44 m_viewMatrix;			// inverse of camera matrix (model matrix on transform) (used for shader)
	Matrix44 m_projectionMatrix;    // projection
	Matrix44 m_changeOfBasisMatrix; // For different supporting coordinate systems

	UniformBuffer m_uniformBuffer;

	float m_nearClipZ;				// For referencing later
	float m_farClipZ;

	// Ortho
	FloatRange	m_orthoSizeLimits;	// Bounds on the ortho size
	float		m_orthoSize;		// Height of the orthographic projection

	// Perspective
	float m_fov;

	unsigned int m_drawOrder;
}; 
