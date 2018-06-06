#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Assets/AssetImporter.hpp"
#include "Engine/Rendering/Meshes/Mesh.hpp"
#include "ThirdParty/assimp/include/assimp/Importer.hpp"

const aiScene* AssetImporter::LoadFile(const std::string& filepath)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filepath.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);

	if (scene == nullptr || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr)
	{
		ERROR_AND_DIE(Stringf("Error: AssetImporter::LoadFile ran into error \"%s\" while loading file \"%s\"", importer.GetErrorString(), filepath.c_str()));
	}

	// Process the nodes in the scene to extract the data
	ProcessNode(scene, scene->mRootNode);
	

	aiReleaseImport(scene);
}

void AssetImporter::ProcessNode(const aiScene* scene, aiNode* node)
{
	// Process meshes
	for (unsigned int meshIndex = 0; meshIndex < node->mNumMeshes; ++meshIndex)
	{
		aiMesh* aimesh = scene->mMeshes[node->mMeshes[meshIndex]];

		Mesh* mesh = ProcessMesh(scene, aimesh);
		m_meshes.push_back(mesh);
	}

	// Process materials

	// Recursively process the child nodes
	for (unsigned int nodeIndex = 0; nodeIndex < node->mNumChildren; ++nodeIndex)
	{
		ProcessNode(scene, node->mChildren[nodeIndex]);
	}
}

Mesh* AssetImporter::ProcessMesh(const aiScene* scene, aiMesh* mesh)
{
	// Lists for data
	std::vector<VertexLit> vertices;
	std::vector<unsigned int> indices;
	
	for (unsigned int vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex)
	{
		// Get position
		Vector3 position;

		position.x = mesh->mVertices[vertexIndex].x;
		position.y = mesh->mVertices[vertexIndex].y;
		position.z = mesh->mVertices[vertexIndex].z;

		// Get normal
		Vector3 normal;

		normal.x = mesh->mNormals[vertexIndex].x;
		normal.y = mesh->mNormals[vertexIndex].y;
		normal.z = mesh->mNormals[vertexIndex].z;

		// Get tangent
		Vector3 tangent;

		tangent.x = mesh->mTangents[vertexIndex].x;
		tangent.y = mesh->mTangents[vertexIndex].y;
		tangent.z = mesh->mTangents[vertexIndex].z;

		// Get uvs, if they exist
		Vector2 uvs = Vector2::ZERO;
		if (mesh->mTextureCoords[0])
		{
			// Only one texture coordinate per vertex, so take the 0th one
			uvs.x = mesh->mTextureCoords[0][vertexIndex].x;
			uvs.y = mesh->mTextureCoords[0][vertexIndex].y;
		}

		VertexLit vertex = VertexLit(position, Rgba::WHITE, uvs, normal, Vector4(tangent, 1.0f));

		vertices.push_back(vertex);
	}

	return nullptr;
}
