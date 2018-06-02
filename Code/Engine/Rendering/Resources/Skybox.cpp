#include "Engine/Rendering/Resources/Skybox.hpp"
#include "Engine/Rendering/Resources/TextureCube.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Materials/Material.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"

Skybox::Skybox(TextureCube* skyboxTexture)
{
	Shader* skyboxShader = AssetDB::CreateOrGetShader("Skybox");
	m_skyboxMaterial = new Material();
	m_skyboxMaterial->SetDiffuse(skyboxTexture);
	m_skyboxMaterial->SetShader(skyboxShader);
}

void Skybox::Render() const
{
	Renderer* renderer = Renderer::GetInstance();
	renderer->DrawCube(Vector3::ZERO, Vector3::ONES, Rgba::WHITE, AABB2::UNIT_SQUARE_OFFCENTER, AABB2::UNIT_SQUARE_OFFCENTER, AABB2::UNIT_SQUARE_OFFCENTER, m_skyboxMaterial);
}

