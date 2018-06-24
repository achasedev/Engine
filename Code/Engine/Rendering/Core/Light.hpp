/************************************************************************/
/* File: Light.hpp
/* Author: Andrew Chase
/* Date: May 2nd, 2018
/* Description: Class to represent a single light in a scene
/************************************************************************/
#pragma once
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Transform.hpp"

#define MAX_NUMBER_OF_LIGHTS (8)

class Texture;

// Light data for a single light
struct LightData
{
	Vector3 m_position;
	float m_dotOuterAngle;

	Vector3 m_lightDirection;
	float m_dotInnerAngle;

	Vector3 m_attenuation;
	float m_directionFactor;

	Vector4 m_color;

	Matrix44 m_shadowVP;

	Vector3 m_padding0;
	float m_castsShadows = 0.f;
};


class Light
{
public:
	//-----Public Methods-----

	~Light();

	// Mutators
	void		SetPosition(const Vector3& position);
	void		SetLightData(const LightData& data);
	void		SetShadowCasting(bool castsShadows);

	// Accessors
	LightData	GetLightData() const;
	bool		IsShadowCasting() const;
	Texture*	GetShadowTexture() const;

	// Producers
	float		CalculateIntensityForPosition(const Vector3& position) const;

	// Statics
	static Light* CreatePointLight(const Vector3& position, const Rgba& color = Rgba::WHITE, const Vector3& attenuation = Vector3(1.f, 0.f, 0.f));
	static Light* CreateDirectionalLight(const Vector3& position, const Vector3& direction = Vector3::DIRECTION_DOWN, const Rgba& color = Rgba::WHITE, const Vector3& attenuation = Vector3(1.f, 0.f, 0.f));
	static Light* CreateConeLight(const Vector3& position, const Vector3& direction, float outerAngle, float innerAngle, const Rgba& color = Rgba::WHITE, const Vector3& attenuation = Vector3(1.f, 0.f, 0.f));

	
private:
	//-----Private Data-----

	LightData m_lightData;

	bool m_isShadowCasting = false;
	Texture* m_shadowTexture = nullptr;

};
