/************************************************************************/
/* File: Transform.hpp
/* Author: Andrew Chase
/* Date: March 27th, 2017
/* Description: Class to represent a Translation/Rotation/Scale in 3D
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix44.hpp"

class Transform
{
public:
	//-----Public Methods-----

	Transform();
	Transform(const Vector3& startPosition, const Vector3& startRotation, const Vector3& startScale);
	void operator=(const Transform& copyFrom);

	// Mutators
	void SetPosition(const Vector3& newPosition);
	void SetRotation(const Vector3& newRotation);
	void SetScale(const Vector3& newScale);

	void SetModelMatrix(const Matrix44& model);
	void SetParentTransform(Transform* parent);

	void TranslateWorld(const Vector3& worldTranslation);
	void TranslateLocal(const Vector3& localTranslation);

	void Rotate(const Vector3& deltaRotation);
	void Scale(const Vector3& deltaScale);

	Matrix44 GetLocalMatrix();	// Matrix that transforms this space to parent's space
	Matrix44 GetWorldMatrix();	// Matrix that transforms this space to absolute world space
	Matrix44 GetParentsToWorldMatrix();

	Vector3 GetIVector();
	Vector3 GetJVector();
	Vector3 GetKVector();

	Vector3 GetWorldPosition();
	Vector3 GetWorldRotation();


private:
	//-----Private Methods-----

	void CheckAndUpdateLocalMatrix();


public:
	//-----Public Data-----

	// All defined in parent space!
	Vector3		position = Vector3::ZERO;	
	Vector3		rotation = Vector3::ZERO;	
	Vector3		scale = Vector3::ONES;		


private:
	//-----Private Data-----

	Vector3 m_oldPosition = Vector3::ZERO;
	Vector3 m_oldRotation = Vector3::ZERO;
	Vector3 m_oldScale = Vector3::ONES;

	Matrix44 m_localMatrix;

	Transform* m_parentTransform = nullptr;
};
