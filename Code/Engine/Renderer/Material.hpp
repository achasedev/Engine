/************************************************************************/
/* File: Material.hpp
/* Author: Andrew Chase
/* Date: April 23rd, 2018
/* Description: Class to represent a texture/sampler/shader set for drawing
/************************************************************************/
#pragma once
#include <vector>
#include "Engine/Core/XmlUtilities.hpp"

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
	Material(const std::string& xmlFilepath); // For CreateOrGet() data driving
	virtual ~Material();

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

	// Uniform block mutators
	bool SetProperty(const char* propertyName, const void* data, size_t byteSize);

	// Template mutators
	template <typename T>
	bool SetProperty(const char* propertyName, const T& value)
	{
		return SetProperty(propertyName, &value, sizeof(T));
	}

	template <typename T>
	bool SetPropertyBlock(const char* blockName, const T& blockData)
	{
		MaterialPropertyBlock* block = GetPropertyBlock(blockName);

		if (block != nullptr)
		{
			block->SetCPUData(blockData);	
			return true;
		}

		// No block exists, so see if we can create it
		const ShaderDescription*		shaderDescription	= m_shader->GetProgram()->GetUniformDescription();
		const PropertyBlockDescription* blockDescription	= shaderDescription->GetBlockDescription(blockName); 

		// If the block doesn't exist or ff the uniform block is an engine reserved one, do nothing
		if (blockDescription == nullptr || blockDescription->GetBlockBinding() < ENGINE_RESERVED_UNIFORM_BLOCK_COUNT)
		{
			return false;
		}

		block = CreatePropertyBlock(blockDescription);
		block->SetCPUData(blockData);
		return true;
	}


protected:
	//-----Protected Methods-----

	MaterialPropertyBlock* CreatePropertyBlock(const PropertyBlockDescription* blockDescription);


private:
	//-----Private Data-----

	Shader* m_shader;
	bool	m_isInstancedShader;

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