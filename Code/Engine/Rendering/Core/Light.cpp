/************************************************************************/
/* File: Light.cpp
/* Author: Andrew Chase
/* Date: May 2nd, 2018
/* Description: Implementation of the light class
/************************************************************************/
#include "Engine/Core/Rgba.hpp"
#include "Engine/Rendering/Core/Light.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/Resources/Texture.hpp"


//-----------------------------------------------------------------------------------------------
// Checks to delete the shadow texture if it was using one
//
Light::~Light()
{
	if (m_shadowTexture != nullptr)
	{
		delete m_shadowTexture;
		m_shadowTexture = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the position of the light to the position specified
//
void Light::SetPosition(const Vector3& position)
{
	m_lightData.m_position = position;
}


//-----------------------------------------------------------------------------------------------
// Sets the light data information for this light
//
void Light::SetLightData(const LightData& data)
{
	m_lightData = data;
}


//-----------------------------------------------------------------------------------------------
// Sets the flag indicating whether this light casts shadows
//
void Light::SetShadowCasting(bool castsShadows)
{
	m_isShadowCasting = castsShadows;

	if (m_isShadowCasting)
	{
		if (m_shadowTexture == nullptr)
		{
			m_shadowTexture = new Texture();
			m_shadowTexture->CreateRenderTarget(4096, 4096, TEXTURE_FORMAT_D24S8);
		}

		m_lightData.m_castsShadows = 1.0f;	// To indicate in the shader that we do shadows
	}
	else
	{
		if (m_shadowTexture != nullptr)
		{
			delete m_shadowTexture;
			m_shadowTexture = nullptr;
		}

		m_lightData.m_castsShadows = 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the light data struct for this light
//
LightData Light::GetLightData() const
{
	return m_lightData;
}


//-----------------------------------------------------------------------------------------------
// Returns the flag indicating if this light casts shadows
//
bool Light::IsShadowCasting() const
{
	return m_isShadowCasting;
}


//-----------------------------------------------------------------------------------------------
// Returns the shadow texture used by this light, nullptr if it doesn't have one
//
Texture* Light::GetShadowTexture() const
{
	return m_shadowTexture;
}


//-----------------------------------------------------------------------------------------------
// Given a position, calculates this light's intensity at that position (based on distance and
// attenuation)
//
float Light::CalculateIntensityForPosition(const Vector3& position) const
{
	float distance = (position - m_lightData.m_position).GetLength();
	float attenuation = 1.f / (m_lightData.m_attenuation.x + m_lightData.m_attenuation.y * distance + m_lightData.m_attenuation.z * distance * distance);
	float result = m_lightData.m_color.w * attenuation;

	return result;
}


//-----------------------------------------------------------------------------------------------
// Constructs and returns a Light as a point light
//
Light* Light::CreatePointLight(const Vector3& position, const Rgba& color /*= Rgba::WHITE*/, const Vector3& attenuation /*= Vector3(1.f, 0.f, 0.f)*/)
{
	Light* light = new Light();
	
	light->m_lightData.m_position = position;

	float red, green, blue, intensity;
	color.GetAsFloats(red, green, blue, intensity);
	light->m_lightData.m_color = Vector4(red, green, blue, intensity);

	light->m_lightData.m_attenuation = attenuation;

	// Don't need dots since this isn't a spot light, and directional factor = 1.f indicates this is a point light
	light->m_lightData.m_dotOuterAngle = -2.f;
	light->m_lightData.m_dotInnerAngle = -1.f;
	light->m_lightData.m_directionFactor = 1.0f;

	// Light direction is unused, as point lights emit light in all directions
	light->m_lightData.m_lightDirection = Vector3::ZERO;

	return light;
}


//-----------------------------------------------------------------------------------------------
// Constructs and returns a Light as a directional light
//
Light* Light::CreateDirectionalLight(const Vector3& position, const Vector3& direction /*= Vector3::DIRECTION_DOWN*/, const Rgba& color /*= Rgba::WHITE*/, const Vector3& attenuation /*= Vector3(1.f, 0.f, 0.f)*/)
{
	Light* light = new Light();

	light->m_lightData.m_position = position;
	light->m_lightData.m_lightDirection = direction;

	float red, green, blue, intensity;
	color.GetAsFloats(red, green, blue, intensity);
	light->m_lightData.m_color = Vector4(red, green, blue, intensity);

	light->m_lightData.m_attenuation = attenuation;

	// Don't need dots since this isn't a spot light, and directional factor = 0.f indicates this is a directional light
	light->m_lightData.m_dotOuterAngle = -2.0f;
	light->m_lightData.m_dotInnerAngle = -1.f;
	light->m_lightData.m_directionFactor = 0.f;

	return light;
}


//-----------------------------------------------------------------------------------------------
// Constructs and returns a Light as a cone light
//
Light* Light::CreateConeLight(const Vector3& position, const Vector3& direction, float outerAngle, float innerAngle, const Rgba& color /*= Rgba::WHITE*/, const Vector3& attenuation /*= Vector3(1.f, 0.f, 0.f)*/)
{
	Light* light = new Light();

	light->m_lightData.m_position = position;
	light->m_lightData.m_lightDirection = direction;

	float red, green, blue, intensity;
	color.GetAsFloats(red, green, blue, intensity);
	light->m_lightData.m_color = Vector4(red, green, blue, intensity);

	light->m_lightData.m_attenuation = attenuation;

	// Need dots since this is a spot light, and directional factor = 1.f indicates this is a point light
	light->m_lightData.m_dotOuterAngle = CosDegrees(outerAngle * 0.5f);
	light->m_lightData.m_dotInnerAngle = CosDegrees(innerAngle * 0.5f);
	light->m_lightData.m_directionFactor = 1.0f;

	return light;
}
