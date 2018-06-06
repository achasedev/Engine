#pragma once
#include "ThirdParty/assimp/include/assimp/cimport.h"
#include "ThirdParty/assimp/include/assimp/scene.h"
#include "ThirdParty/assimp/include/assimp/postprocess.h"

class Mesh;
class Material;
class Renderable;

class AssetImporter
{
public:
	//-----Public Methods-----

	const aiScene* LoadFile(const std::string& filepath);


private:
	//-----Private Data-----

	void ProcessNode(const aiScene* scene, aiNode* node);
		Mesh* ProcessMesh(const aiScene* scene, aiMesh* mesh);
		Material* ProcessMaterial(const aiScene* scene, aiMaterial* material);

private:
	//-----Private Data-----

	const aiScene* m_assimpScene;

	// Extracted Data
	std::vector<Mesh*> m_meshes;

};
