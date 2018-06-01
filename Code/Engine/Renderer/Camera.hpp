/************************************************************************/
/* File: Camera.hpp
/* Author: Andrew Chase
/* Date: February 13th, 2018
/* Description: Class to represent a draw-to buffer with projection
/************************************************************************/
#pragma once
#include "Engine/Renderer/FrameBuffer.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/FloatRange.hpp"


class Texture;
class Matrix44;

class Camera
{
public:
	//-----Public Methods-----

	Camera() {}
	~Camera() {}

	void SetColorTarget(Texture* color_target);
	void SetDepthTarget(Texture* depth_target);

	void Finalize();	// Finalizes the frame buffer

	// model setters
	void LookAt(const Vector3& position, const Vector3& target, const Vector3& up = Vector3::DIRECTION_UP); 
	void SetCameraMatrix(const Matrix44& cameraMatrix);
	void SetViewMatrix(const Matrix44& viewMatrix);

	// projection settings
	void SetProjection(const Matrix44& projection); 
	void SetProjectionOrtho(float height, float nearZ, float farZ); 

	// Other setters
	void SetOrthoSize(float newSize);
	void AdjustOrthoSize(float additiveModifier);
	void SetOrthoSizeLimits(float min, float max);

	// Accessors
	Matrix44 GetCameraMatrix() const;
	Matrix44 GetViewMatrix() const;
	Matrix44 GetProjectionMatrix() const;

	Vector3 GetForwardVector() const;
	Vector3 GetRightVector() const;
	Vector3 GetUpVector() const;

	unsigned int GetFrameBufferHandle() const;
	

private:
	//-----Private Methods-----

	Matrix44 InvertLookAtMatrix(const Matrix44& lookAt) const;	// Inverts the TR lookAt matrix, for the camera view matrix


public:
	//-----Public Data-----

	FrameBuffer m_frameBuffer;		// Color and depth targets of this camera


private:
	//-----Private Data-----

	// All initialized to identity
	Matrix44 m_cameraMatrix;		// where is the camera?
	Matrix44 m_viewMatrix;			// inverse of camera (used for shader)
	Matrix44 m_projectionMatrix;    // projection

	float m_nearClipZ;				// For referencing later
	float m_farClipZ;

	FloatRange m_orthoSizeLimits;	// Bounds on the ortho size
	float m_orthoSize;				// Height of the orthographic projection

}; 
