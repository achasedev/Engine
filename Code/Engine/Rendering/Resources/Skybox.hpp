#pragma once

class Material;
class TextureCube;

class Skybox
{
public:
	//-----Public Data-----

	Skybox(TextureCube* skyboxTexture);

	void Render() const;


private:
	//-----Private Data-----

	Material* m_skyboxMaterial;

};
