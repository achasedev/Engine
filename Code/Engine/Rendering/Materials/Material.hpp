/************************************************************************/
/* File: Material.hpp
/* Author: Andrew Chase
/* Date: April 23rd, 2018
/* Description: Class to represent a texture/sampler/shader set for drawing
/************************************************************************/
#pragma once
#include <string>
#include <vector>
#include "Engine/Core/Utility/XmlUtilities.hpp"

#define MAX_TEXTURES_SAMPLERS (10)

class Shader;
class Texture;
class Sampler;
class MaterialPropertyBlock;
class PropertyBlockDescription;
class Material
{
public:
	//-----Public Methods-----

	friend class MaterialInstance;

	Material();
	Material(const std::string& name);
	virtual ~Material();

	bool LoadFromFile(const std::string& filepath);

	// Accessors
	int GetPropertyBlockCount() const;
	MaterialPropertyBlock* GetPropertyBlock(int index) const;
	MaterialPropertyBlock* GetPropertyBlock(const char* blockName) const;
	
	const Shader*	GetShader() const;
	Shader*			GetEditableShader();

	const Texture* GetTexture(int textureIndex) const;
	const Sampler* GetSampler(int samplerIndex) const;

	bool IsUsingLights() const;

	// Mutators
	void SetShader(Shader* shader, bool isInstancedShader = false);
	void SetTexture(unsigned int bindPoint, const Texture* texture);
	void SetSampler(unsigned int bindPoint, const Sampler* sampler);

	void SetDiffuse(const Texture* diffuse);
	void SetNormal(const Texture* normal);
	void SetSpecular(const Texture* specular);
	void SetEmissive(const Texture* emissive);

	// Uniform block mutators
	bool SetProperty(const char* propertyName, const void* data, size_t byteSize);

	// Template mutators
	template <typename T>
	bool SetProperty(const char* propertyName, const T& value)
	{
		return SetProperty(propertyName, &value, sizeof(T));
	}

	bool SetPropertyBlock(const char* blockName, const void* data, size_t byteSize);

	template <typename T>
	bool SetPropertyBlock(const char* blockName, const T& blockData)
	{
		return SetPropertyBlock(blockName, &blockData, sizeof(T));
	}


protected:
	//-----Protected Methods-----

	MaterialPropertyBlock* CreatePropertyBlock(const PropertyBlockDescription* blockDescription);


private:
	//-----Private Data-----

	std::string		m_name;
	Shader*			m_shader;
	bool			m_isInstancedShader;

	// Parallel vectors
	const Texture* m_textures[MAX_TEXTURES_SAMPLERS];
	const Sampler* m_samplers[MAX_TEXTURES_SAMPLERS];

	std::vector<MaterialPropertyBlock*> m_propertyBlocks; // An array of Uniform Buffers

};

// Example XML format
// <material>
// 
// <shader name="Default_Opaque"/>
// 
// <textures>
// <texture name="Miku.png" bind="0" />
// <texture name="Flat" bind="1"/>
// </textures>
// 
// <samplers>
// </samplers>
// 
// </material>