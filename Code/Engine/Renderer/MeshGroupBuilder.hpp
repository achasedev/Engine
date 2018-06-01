#pragma once
#include <string>
#include <vector>
#include "Engine/Renderer/MeshGroup.hpp"

class MeshBuilder;

class MeshGroupBuilder
{
public:

	~MeshGroupBuilder();

	void LoadFromObjFile(const std::string& filename);
	
	template <typename VERT_TYPE = VertexLit>
	MeshGroup* CreateMeshGroup() const
	{
		int numBuilders = (int) m_meshBuilders.size();
		MeshGroup* group = new MeshGroup();

		for (int builderIndex = 0; builderIndex < numBuilders; ++builderIndex)
		{
			Mesh* mesh = m_meshBuilders[builderIndex]->CreateMesh<VERT_TYPE>();
			group->AddMeshUnique(mesh);
		}

		return group;
	}


private:

	std::vector<MeshBuilder*> m_meshBuilders;

};