/************************************************************************/
/* File: MeshBuilder.hpp
/* Author: Andrew Chase
/* Date: March 25th, 2018
/* Description: Class to construct Meshes incrementally
/************************************************************************/
#pragma once
#include <vector>
#include "Engine/Math/AABB2.hpp"
#include "Engine/Rendering/Meshes/Mesh.hpp"
#include "Engine/Rendering/Core/Vertex.hpp"

typedef Vector3 (*SurfacePatchFunction)(const Vector2&);

class MeshBuilder
{
public:
	//-----Public Member Methods-----

	void BeginBuilding(PrimitiveType type, bool useIndices);
	void FinishBuilding();
	void Clear();

	// Stamp Functions
	void			SetColor(const Rgba& color);
	void			SetUVs(const Vector2& uvs);
	void			SetNormal(const Vector3& normal);
	void			SetTangent(const Vector4& tangent);

	// Basic Builder Functions
	unsigned int	PushVertex(const Vector3& position);
	unsigned int	PushVertex(const VertexMaster& master);

	void			PushIndex(unsigned int index);
	void			PushIndices(unsigned int first, unsigned int second, unsigned int third);

	// Helper Builder Functions
	void PushPoint(const Vector3& position, const Rgba& color = Rgba::WHITE, float radius = 1.0f);
	void Push2DQuad(const AABB2& bounds, const AABB2& uvs, const Rgba& color = Rgba::WHITE);
	void Push3DQuad(const Vector3& position, const Vector2& dimensions, const AABB2& uvs, const Rgba& color = Rgba::WHITE, 
		const Vector3& rightVector = Vector3::DIRECTION_RIGHT, const Vector3& upVector = Vector3::DIRECTION_UP, const Vector2& pivot = Vector2(0.5f, 0.5f));
	void PushCube(const Vector3& center, const Vector3& dimensions, const Rgba& color = Rgba::WHITE, 
		const AABB2& sideUVs = AABB2::UNIT_SQUARE_OFFCENTER, const AABB2& topUVs = AABB2::UNIT_SQUARE_OFFCENTER, const AABB2& bottomUVs = AABB2::UNIT_SQUARE_OFFCENTER);
	void PushUVSphere(const Vector3& position, float radius, unsigned int numWedges, unsigned int numSlices, const Rgba& color = Rgba::WHITE);

	void PushSurfacePatch(SurfacePatchFunction function, unsigned int numUSteps = 10, unsigned int numVSteps = 10, const Rgba& color = Rgba::WHITE);
	
	// For OBJ loading
	void LoadFromObjFile(const std::string& filePath);

	// Manipulators
	void FlipHorizontal();

	void	GenerateFlatTBN();
	void	GenerateSmoothNormals();

	// Accessors
	template <typename VERT_TYPE>
	VERT_TYPE GetVertex(int index)
	{
		VERT_TYPE vertex = VERT_TYPE(m_vertices[index]);
		return vertex;
	}

	int		GetVertexCount();
	int		GetIndexCount();
	int		GetElementCount();


public:
	//-----Helpers for MikkTSpace generation-----

	Vector3 GetVertexPosition(int vboIndex) const;	// All work with indices into the vertex vector, not the index vector
	Vector3 GetVertexNormal(int vboIndex) const;
	Vector2 GetVertexUV(int vboIndex) const;
	int		GetNumTriangles() const;

	void	SetVertexTangent(int vboIndex, const Vector4& tangent);


public:
	//-----For Skeleton Animation-----

	void AddBoneData(int vboIndex, unsigned int boneIndex, float weight);


public:
	//-----Templates for creating meshes-----

	// Return Mesh as a pointer
	template <typename VERT_TYPE = VertexLit>
	Mesh* CreateMesh() const
	{
		Mesh* mesh = new Mesh();
		UpdateMesh<VERT_TYPE>(*mesh);

		return mesh;
	}

	// Return mesh in provided reference
	template <typename VERT_TYPE = VertexLit>
	void UpdateMesh(Mesh& out_mesh) const
	{
		// Convert the list of VertexMasters to the specified vertex type
		unsigned int vertexCount = (unsigned int) m_vertices.size();
		VERT_TYPE* temp = (VERT_TYPE*)malloc(sizeof(VERT_TYPE) * vertexCount);

		for (unsigned int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
		{
			temp[vertexIndex] = VERT_TYPE(m_vertices[vertexIndex]);
		}

		// Set up the mesh
		out_mesh.SetVertices(vertexCount, temp);
		out_mesh.SetIndices((unsigned int) m_indices.size(), m_indices.data());
		out_mesh.SetDrawInstruction(m_instruction);

		free(temp);
	}

	void AssertBuildState(bool shouldBeBuilding, PrimitiveType primitiveType, bool shouldUseIndices) const;

	// For Object file loading
	static VertexMaster CreateMasterFromString(const std::string& text, const std::vector<Vector3>& positions, const std::vector<Vector3>& normals, const std::vector<Vector2>& uvs);


private:
	//-----Private Data-----

	bool					m_isBuilding = false;
	VertexMaster			m_master;
	DrawInstruction			m_instruction;

	std::vector<unsigned int>	m_indices;
	std::vector<VertexMaster>	m_vertices;
	
};


// For MikkTSpace
bool GenerateMikkTangents(MeshBuilder& mb);