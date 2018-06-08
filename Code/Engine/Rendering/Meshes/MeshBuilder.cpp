/************************************************************************/
/* File: MeshBuilder.cpp
/* Author: Andrew Chase
/* Date: March 25th, 2018
/* Description: Implementation of the MeshBuilder class
/************************************************************************/
#include "Engine/Core/File.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"

#include "ThirdParty/mikkt/mikktspace.h"


// MikkTSpace C functions
static int	GetNumFaces(const SMikkTSpaceContext* pContext);
static int	GetNumVerticesPerFace(const SMikkTSpaceContext* pContext, const int iFace);
static void GetVertexPosition(const SMikkTSpaceContext * pContext, float fvPosOut[], const int iFace, const int iVert);
static void GetVertexNormal(const SMikkTSpaceContext * pContext, float fvNormOut[], const int iFace, const int iVert);
static void GetVertexUV(const SMikkTSpaceContext * pContext, float fvTexcOut[], const int iFace, const int iVert);
static void SetVertexTangent(const SMikkTSpaceContext * pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert);

//-----------------------------------------------------------------------------------------------
// Begins the build process by setting up the instruction
//
void MeshBuilder::BeginBuilding(PrimitiveType type, bool useIndices)
{
	m_instruction.m_primType = type;
	m_instruction.m_usingIndices = useIndices;

	if (useIndices) { m_instruction.m_startIndex = (int) m_indices.size();  }
	else			{ m_instruction.m_startIndex = (int) m_vertices.size(); }

	// Reset the master
	m_master = VertexMaster();
	m_isBuilding = true;
}


//-----------------------------------------------------------------------------------------------
// Finalizes the instruction data given the vertices and indices
//
void MeshBuilder::FinishBuilding()
{
	unsigned int endIndex;

	if (m_instruction.m_usingIndices)	{ endIndex = (unsigned int) m_indices.size();  }
	else								{ endIndex = (unsigned int) m_vertices.size(); }

	m_instruction.m_elementCount = (endIndex - m_instruction.m_startIndex);
	m_isBuilding = false;
}


//-----------------------------------------------------------------------------------------------
// Clears all state and data from the builder, starting from a clean state
//
void MeshBuilder::Clear()
{
	// Clean up the stamp
	m_master = VertexMaster();

	m_vertices.clear();
	m_indices.clear();
	m_isBuilding = false;
}


//-----------------------------------------------------------------------------------------------
// Loads an OBJ file at the given path and parses the information
//
void MeshBuilder::LoadFromObjFile(const std::string& filePath)
{
	AssertBuildState(false, PRIMITIVE_TRIANGLES, false);
	BeginBuilding(PRIMITIVE_TRIANGLES, false);

	const char* buffer = (const char*)FileReadToNewBuffer(filePath.c_str());
	if (buffer == nullptr) { return; }

	std::string fileContents = std::string(buffer);

	std::vector<Vector3> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> uvs;

	std::vector<std::string> lines = Tokenize(fileContents, '\n');
	int numLines = (int) lines.size();

	for (int lineIndex = 0; lineIndex < numLines; ++lineIndex)
	{
		std::string currLine = lines[lineIndex];

		std::vector<std::string> tokens = Tokenize(currLine, ' ');

		// Empty line check
		if (tokens.size() == 0) { continue; }

		// Figure out how to treat the line
		bool isPosition = (tokens[0] == "v");
		bool isNormal	= (tokens[0] == "vn");
		bool isUV		= (tokens[0] == "vt");
		bool isFace		= (tokens[0] == "f");

		if (isPosition || isNormal || isUV)
		{
			// Treat tokens as floats
			float x = StringToFloat(tokens[1]);
			float y = StringToFloat(tokens[2]);

			// Only care about the z if it's not a UV
			if (isPosition || isNormal)
			{
				float z = StringToFloat(tokens[3]);
				
				// Figure out where to push it
				if (isPosition) { positions.push_back(Vector3(x, y, z)); }
				else			{ normals.push_back(Vector3(x, y, z));   }
			}
			else
			{
				// Is a UV
				uvs.push_back(Vector2(x, y));
			}
		}
		else if (isFace)
		{
			// flag + 3 tokens specifies 3 vertices, which is one triangle
			// otherwise we assume 4 vertices are specified, for a quad
			bool isTwoTriangles = (tokens.size() == 5);

			std::vector<VertexMaster> vertices;

			// Start at one to miss the flag
			for (int tokenIndex = 1; tokenIndex < (int) tokens.size(); ++tokenIndex)
			{
				vertices.push_back(CreateMasterFromString(tokens[tokenIndex], positions, normals, uvs));
			}

			// Push the vertices in order
			m_vertices.push_back(vertices[0]);
			m_vertices.push_back(vertices[1]);
			m_vertices.push_back(vertices[2]);

			if (isTwoTriangles)
			{
				m_vertices.push_back(vertices[0]);
				m_vertices.push_back(vertices[2]);
				m_vertices.push_back(vertices[3]);
			}
		}
	}

	// Flip the Mesh horizontally, since OBJ files use a right-handed basis
	FlipHorizontal();

	// Generate normals (and tangents), only if none were specified in the OBJ file
	bool normalsSpecified = (normals.size() > 0);

	if (!normalsSpecified)
	{
		GenerateFlatTBN();
	}
	else
	{
		// Generate tangents using existing normals
		GenerateMikkTangents(*this);
	}

	FinishBuilding();
}


//-----------------------------------------------------------------------------------------------
// Flips the X on positions and normals for the masters
//
void MeshBuilder::FlipHorizontal()
{
	int numVertices = (int) m_vertices.size();

	for (int vertexIndex = 0; vertexIndex < numVertices; ++vertexIndex)
	{
		m_vertices[vertexIndex].m_position.x *= -1.0f;
		m_vertices[vertexIndex].m_normal.x *= -1.0f;
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the position of the vertex at the given index
//
Vector3 MeshBuilder::GetVertexPosition(int vboIndex) const
{
	ASSERT_OR_DIE(vboIndex >= 0 && vboIndex < (int) m_vertices.size(), Stringf("Error: MeshBuilder::GetVertexPosition() received bad index: %i", vboIndex).c_str());
	return m_vertices[vboIndex].m_position;
}


//-----------------------------------------------------------------------------------------------
// Returns the normal of the vertex at the given index
//
Vector3 MeshBuilder::GetVertexNormal(int vboIndex) const
{
	ASSERT_OR_DIE(vboIndex >= 0 && vboIndex < (int) m_vertices.size(), Stringf("Error: MeshBuilder::GetVertexNormal() received bad index: %i", vboIndex).c_str());
	return m_vertices[vboIndex].m_normal;
}


//-----------------------------------------------------------------------------------------------
// Returns the UV of the vertex at the given index
//
Vector2 MeshBuilder::GetVertexUV(int vboIndex) const
{
	ASSERT_OR_DIE(vboIndex >= 0 && vboIndex < (int) m_vertices.size(), Stringf("Error: MeshBuilder::GetVertexUV() received bad index: %i", vboIndex).c_str());
	return m_vertices[vboIndex].m_uvs;
}


//-----------------------------------------------------------------------------------------------
// Returns the number of triangles in the builder
//
int MeshBuilder::GetNumTriangles() const
{
	ASSERT_OR_DIE(m_instruction.m_primType == PRIMITIVE_TRIANGLES, Stringf("Error: MeshBuilder::GetNumTriangles() called on builder not building triangles.").c_str());

	if (m_instruction.m_usingIndices) 
	{ 
		return (int) m_indices.size() / 3; 
	}

	return (int) m_vertices.size() / 3;
}


//-----------------------------------------------------------------------------------------------
// Sets the tangent of the vertex at the given index to the one specified
//
void MeshBuilder::SetVertexTangent(int vboIndex, const Vector4& tangent)
{
	ASSERT_OR_DIE(vboIndex >= 0 && vboIndex < (int) m_vertices.size(), Stringf("Error: MeshBuilder::SetVertexTangent() received bad index: %i", vboIndex).c_str());
	m_vertices[vboIndex].m_tangent = tangent;
}


//-----------------------------------------------------------------------------------------------
// Generates the normals for all vertices in the MeshBuilder on a per-face basis
//
void MeshBuilder::GenerateFlatTBN()
{
	ASSERT_OR_DIE((int) m_vertices.size() % 3 == 0, Stringf("Error: MeshBuilder::GenerateFlatNormals() called with weird number of vertices: %i", (int) m_vertices.size()));
	ASSERT_OR_DIE(m_instruction.m_primType == PRIMITIVE_TRIANGLES, Stringf("Error: MeshBuilder::GenerateFlatNormals() called on builder that isn't using triangles"));
	ASSERT_OR_DIE(!m_instruction.m_usingIndices, Stringf("Error: MeshBuilder::GenerateFlatNormals() called on builder that is using indices."));

	for (int faceIndex = 0; faceIndex < (int) m_vertices.size(); faceIndex += 3)
	{
		Vector3 a = m_vertices[faceIndex].m_position;
		Vector3 b = m_vertices[faceIndex + 1].m_position;
		Vector3 c = m_vertices[faceIndex + 2].m_position;

		Vector3 rightSide = b - a; // Order is important for the cross product
		Vector3 leftSide = c - a;

		Vector3 normal = CrossProduct(leftSide, rightSide).GetNormalized();

		// Set all to use the same normals
		m_vertices[faceIndex].m_normal = normal;
		m_vertices[faceIndex + 1].m_normal = normal;
		m_vertices[faceIndex + 2].m_normal = normal;
	}

	// Also generate tangents
	GenerateMikkTangents(*this);
}


//-----------------------------------------------------------------------------------------------
// Generates the normals for all vertices in the MeshBuilder on a per-face basis, with no smoothing
//
void MeshBuilder::GenerateSmoothNormals()
{
	ASSERT_OR_DIE((int) m_vertices.size() % 3 == 0, Stringf("Error: MeshBuilder::GenerateSmoothNormals() called with weird number of vertices: %i", (int) m_vertices.size()));
	ASSERT_OR_DIE(m_instruction.m_primType == PRIMITIVE_TRIANGLES, Stringf("Error: MeshBuilder::GenerateSmoothNormals() called on builder that isn't using triangles"));
	ASSERT_OR_DIE(!m_instruction.m_usingIndices, Stringf("Error: MeshBuilder::GenerateSmoothNormals() called on builder that is using indices."));

	// We reset the master at the start of every build, so normal defaults to (0,0,0)
	// If we still have (0,0,0) as a normal, then we assume normals haven't been assigned yet, so do that first
	bool hasVertexNormals = m_vertices[0].m_normal != Vector3::ZERO;
	if (!hasVertexNormals)
	{
		GenerateFlatTBN();
	}

	std::vector<Vector3> positionsWithNormals;

	for (int vertexIndex = 0; vertexIndex < (int) m_vertices.size(); ++vertexIndex)
	{
		Vector3 currPosition = m_vertices[vertexIndex].m_position;
	
		// To avoid floating point errors, I'll check if we've done it myself, not use std::find()
		bool positionAlreadyDone = false;
		for (int finishedIndex = 0; finishedIndex < (int) positionsWithNormals.size(); ++finishedIndex)
		{
			if (AreMostlyEqual(currPosition, positionsWithNormals[finishedIndex]))
			{
				positionAlreadyDone = true;
			}
		}

		if (positionAlreadyDone) { continue; }

		// Find all copies of this position
		std::vector<int> indices;

		for (int positionIndex = 0; positionIndex < (int) m_vertices.size(); ++positionIndex)
		{
			if (AreMostlyEqual(currPosition, m_vertices[positionIndex].m_position))
			{
				indices.push_back(positionIndex);
			}
		}

		// For each position copy, get the normal and area of the shared face
		std::vector<Vector3> normals;
		std::vector<float> faceAreas;
		float totalArea = 0.f;

		for (int index = 0; index < (int) indices.size(); ++index)
		{
			int faceIndex = indices[index] / 3;

			Vector3 a = m_vertices[3 * faceIndex].m_position;
			Vector3 b = m_vertices[3 * faceIndex + 1].m_position;
			Vector3 c = m_vertices[3 * faceIndex + 2].m_position;

			Vector3 rightSide = (b - a);
			Vector3 leftSide = (c - a);

			Vector3 normal = CrossProduct(leftSide, rightSide);
			float area = normal.NormalizeAndGetLength() * 0.5f;

			totalArea += area;
			faceAreas.push_back(area);
			normals.push_back(normal);
		}

		// Now we have all normals and face areas that this position is part of, so now calculate
		// the weighed average
		Vector3 averageNormal = Vector3::ZERO;
		for (int index = 0; index < (int) indices.size(); ++index)
		{
			averageNormal += faceAreas[index] * normals[index];
		}

		averageNormal /= totalArea;

		// Assign all vertices with this position the weighted normal
		for (int index = 0; index < (int) indices.size(); ++index)
		{
			m_vertices[indices[index]].m_normal = averageNormal;
		}

		// Done with this position, so add the position to the finished list
		positionsWithNormals.push_back(currPosition);
	}

	// Also generate tangents
	GenerateMikkTangents(*this);
}


//-----------------------------------------------------------------------------------------------
// Sets the color on the vertex stamp to the one given
//
void MeshBuilder::SetColor(const Rgba& color)
{
	m_master.m_color = color;
}


//-----------------------------------------------------------------------------------------------
// Sets the color on the vertex stamp to the one given
//
void MeshBuilder::SetUVs(const Vector2& uvs)
{
	m_master.m_uvs = uvs;
}


//-----------------------------------------------------------------------------------------------
// Sets the normal on the vertex stamp to the one given
//
void MeshBuilder::SetNormal(const Vector3& normal)
{
	m_master.m_normal = normal;
}


//-----------------------------------------------------------------------------------------------
// Sets the tangent on the vertex stamp to the one given
//
void MeshBuilder::SetTangent(const Vector4& tangent)
{
	m_master.m_tangent = tangent;
}


//-----------------------------------------------------------------------------------------------
// Adds a vertex to the list of vertices, given the position and current stamp state
//
unsigned int MeshBuilder::PushVertex(const Vector3& position)
{
	m_master.m_position = position;

	m_vertices.push_back(m_master);
	return (unsigned int) m_vertices.size() - 1;
}


//-----------------------------------------------------------------------------------------------
// Adds a vertex to the list of vertices, given the total vertex data to push
//
unsigned int MeshBuilder::PushVertex(const VertexMaster& master)
{
	m_master = master;
	m_vertices.push_back(m_master);
	return (unsigned int) m_vertices.size() - 1;
}


//-----------------------------------------------------------------------------------------------
// Pushes a single index into the index buffer
//
void MeshBuilder::PushIndex(unsigned int index)
{
	m_indices.push_back(index);
}


//-----------------------------------------------------------------------------------------------
// Adds the given indices to the index list (for a triangle)
//
void MeshBuilder::PushIndices(unsigned int first, unsigned int second, unsigned int third)
{
	m_indices.push_back(first);
	m_indices.push_back(second);
	m_indices.push_back(third);
}


//-----------------------------------------------------------------------------------------------
// Pushes the vertices and indices needed to construt a point with the given params
//
void MeshBuilder::PushPoint(const Vector3& position, const Rgba& color /*= Rgba::WHITE*/, float radius /*= 1.0f*/)
{
	AssertBuildState(true, PRIMITIVE_LINES, false);

	SetColor(color);
	SetUVs(Vector2::ZERO);

	PushVertex(position - Vector3::DIRECTION_RIGHT * radius);
	PushVertex(position + Vector3::DIRECTION_RIGHT * radius);

	PushVertex(position - Vector3::DIRECTION_UP * radius);
	PushVertex(position + Vector3::DIRECTION_UP * radius);

	PushVertex(position - Vector3::DIRECTION_FORWARD * radius);
	PushVertex(position + Vector3::DIRECTION_FORWARD * radius);

	PushVertex(position - Vector3(1.0f, 1.0f, 1.0f) * radius);
	PushVertex(position + Vector3(1.0f, 1.0f, 1.0f) * radius);

	PushVertex(position - Vector3(-1.0f, 1.0f, 1.0f) * radius);
	PushVertex(position + Vector3(-1.0f, 1.0f, 1.0f) * radius);

	PushVertex(position - Vector3(1.0f, 1.0f, -1.0f) * radius);
	PushVertex(position + Vector3(1.0f, 1.0f, -1.0f) * radius);

	PushVertex(position - Vector3(-1.0f, 1.0f, -1.0f) * radius);
	PushVertex(position + Vector3(-1.0f, 1.0f, -1.0f) * radius);
}


//-----------------------------------------------------------------------------------------------
// Pushes the vertices and indices needed to construct a quad with the given params
//
void MeshBuilder::Push2DQuad(const AABB2& bounds, const AABB2& uvs, const Rgba& color /*= Rgba::WHITE*/)
{
	AssertBuildState(true, PRIMITIVE_TRIANGLES, true);

	SetColor(color);

	// Get the vertices
	Vector2 bottomLeft	= bounds.GetBottomLeft();
	Vector2 bottomRight = bounds.GetBottomRight();
	Vector2 topRight	= bounds.GetTopRight();
	Vector2 topLeft		= bounds.GetTopLeft();

	// Push vertices

	// Bottom Left
	SetUVs(uvs.GetBottomLeft());
	unsigned int index = PushVertex(Vector3(bottomLeft.x, bottomLeft.y, 0.f));

	// Bottom Right
	SetUVs(uvs.GetBottomRight());
	PushVertex(Vector3(bottomRight.x, bottomRight.y, 0.f));

	// Top Right
	SetUVs(uvs.GetTopRight());
	PushVertex(Vector3(topRight.x, topRight.y, 0.f));

	// Top Left
	SetUVs(uvs.GetTopLeft());
	PushVertex(Vector3(topLeft.x, topLeft.y, 0.f));

	// Push the triangle indices
	PushIndices(index + 0, index + 1, index + 2);
	PushIndices(index + 0, index + 2, index + 3);
}


//-----------------------------------------------------------------------------------------------
// Pushes the vertices and indices needed to construct a 2D Quad in 3D space
//
void MeshBuilder::Push3DQuad(const Vector3& position, const Vector2& dimensions, const AABB2& uvs, const Rgba& color /*= Rgba::WHITE*/, const Vector3& rightVector /*= Vector3::DIRECTION_RIGHT*/, const Vector3& upVector /*= Vector3::DIRECTION_UP*/, const Vector2& pivot /*= Vector2(0.5f, 0.5f)*/)
{
	AssertBuildState(true, PRIMITIVE_TRIANGLES, true);

	//-----Set up the vertices-----
	// Find the min and max X values for the AABB2 draw bounds
	float minX = -1.0f * (pivot.x * dimensions.x);
	float maxX = minX + dimensions.x;

	// Find the min and max X values for the sprite AABB2 draw bounds
	float minY = -1.0f * (pivot.y * dimensions.y);
	float maxY = minY + dimensions.y;

	// Determine corner locations based on the alignment vectors
	Vector3 bottomLeft 		= position + minX * rightVector + minY * upVector;
	Vector3 bottomRight 	= position + maxX * rightVector + minY * upVector;
	Vector3 topLeft 		= position + minX * rightVector + maxY * upVector;
	Vector3 topRight 		= position + maxX * rightVector + maxY * upVector;

	// Calculate normal(s)
	Vector3 normal = CrossProduct(upVector, rightVector);

	// Begin adding to the mesh
	SetColor(color);
	SetNormal(normal);
	SetTangent(Vector4(rightVector, 1.0f));

	//-----Push the Vertices-----

	// Bottom Left
	SetUVs(uvs.GetBottomLeft());
	unsigned int index = PushVertex(bottomLeft);

	// Bottom Right
	SetUVs(uvs.GetBottomRight());
	PushVertex(bottomRight);

	// Top Right
	SetUVs(uvs.GetTopRight());
	PushVertex(topRight);

	// Top Left
	SetUVs(uvs.GetTopLeft());
	PushVertex(topLeft);

	//-----Push the indices-----
	PushIndices(index + 0, index + 1, index + 2);
	PushIndices(index + 0, index + 2, index + 3);
}


//-----------------------------------------------------------------------------------------------
// Pushes the vertices and indices needed to construct a 3D cube with the given params
//
void MeshBuilder::PushCube(const Vector3& center, const Vector3& dimensions, const Rgba& color /*= Rgba::WHITE*/, 
	const AABB2& sideUVs /*= AABB2::UNIT_SQUARE_OFFCENTER*/, const AABB2& topUVs /*= AABB2::UNIT_SQUARE_OFFCENTER*/, const AABB2& bottomUVs /*= AABB2::UNIT_SQUARE_OFFCENTER*/)
{
	AssertBuildState(true, PRIMITIVE_TRIANGLES, true);

	// Set up the corner vertices
	AABB3 cubeBounds = AABB3(center - dimensions * 0.5f, center + dimensions * 0.5f);

	SetColor(color);

	//-------------------------------------Front face--------------------------------------------------------
	{
		SetUVs(sideUVs.GetBottomLeft());
		SetNormal(Vector3::DIRECTION_BACK);
		SetTangent(Vector4::DIRECTION_RIGHT);

		unsigned int index = PushVertex(cubeBounds.GetFrontBottomLeft());

		SetUVs(sideUVs.GetBottomRight());
		PushVertex(cubeBounds.GetFrontBottomRight());

		SetUVs(sideUVs.GetTopRight());
		PushVertex(cubeBounds.GetFrontTopRight());

		SetUVs(sideUVs.GetTopLeft());
		PushVertex(cubeBounds.GetFrontTopLeft());

		PushIndices(index + 0, index + 1, index + 2);
		PushIndices(index + 0, index + 2, index + 3);
	}

	//-------------------------------------Back face--------------------------------------------------------
	{
		SetUVs(sideUVs.GetBottomLeft());
		SetNormal(Vector3::DIRECTION_FORWARD);
		SetTangent(Vector4::DIRECTION_LEFT);

		unsigned int index = PushVertex(cubeBounds.GetBackBottomRight());

		SetUVs(sideUVs.GetBottomRight());
		PushVertex(cubeBounds.GetBackBottomLeft());

		SetUVs(sideUVs.GetTopRight());
		PushVertex(cubeBounds.GetBackTopLeft());

		SetUVs(sideUVs.GetTopLeft());
		PushVertex(cubeBounds.GetBackTopRight());

		PushIndices(index + 0, index + 1, index + 2);
		PushIndices(index + 0, index + 2, index + 3);
	}

	//-------------------------------------Left face--------------------------------------------------------
	{
		SetUVs(sideUVs.GetBottomLeft());
		SetNormal(Vector3::DIRECTION_LEFT);
		SetTangent(Vector4::DIRECTION_BACK);

		unsigned int index = PushVertex(cubeBounds.GetBackBottomLeft());

		SetUVs(sideUVs.GetBottomRight());
		PushVertex(cubeBounds.GetFrontBottomLeft());

		SetUVs(sideUVs.GetTopRight());
		PushVertex(cubeBounds.GetFrontTopLeft());

		SetUVs(sideUVs.GetTopLeft());
		PushVertex(cubeBounds.GetBackTopLeft());

		PushIndices(index + 0, index + 1, index + 2);
		PushIndices(index + 0, index + 2, index + 3);
	}

	//-------------------------------------Right face--------------------------------------------------------
	{
		SetUVs(sideUVs.GetBottomLeft());
		SetNormal(Vector3::DIRECTION_RIGHT);
		SetTangent(Vector4::DIRECTION_FORWARD);

		unsigned int index = PushVertex(cubeBounds.GetFrontBottomRight());

		SetUVs(sideUVs.GetBottomRight());
		PushVertex(cubeBounds.GetBackBottomRight());

		SetUVs(sideUVs.GetTopRight());
		PushVertex(cubeBounds.GetBackTopRight());

		SetUVs(sideUVs.GetTopLeft());
		PushVertex(cubeBounds.GetFrontTopRight());

		PushIndices(index + 0, index + 1, index + 2);
		PushIndices(index + 0, index + 2, index + 3);
	}

	//-------------------------------------Top face--------------------------------------------------------
	{
		SetUVs(topUVs.GetBottomLeft());
		SetNormal(Vector3::DIRECTION_UP);
		SetTangent(Vector4::DIRECTION_RIGHT);

		unsigned int index = PushVertex(cubeBounds.GetFrontTopLeft());

		SetUVs(topUVs.GetBottomRight());
		PushVertex(cubeBounds.GetFrontTopRight());

		SetUVs(topUVs.GetTopRight());
		PushVertex(cubeBounds.GetBackTopRight());

		SetUVs(topUVs.GetTopLeft());
		PushVertex(cubeBounds.GetBackTopLeft());

		PushIndices(index + 0, index + 1, index + 2);
		PushIndices(index + 0, index + 2, index + 3);
	}

	//-------------------------------------Bottom face--------------------------------------------------------
	{
		SetUVs(bottomUVs.GetBottomLeft());
		SetNormal(Vector3::DIRECTION_DOWN);
		SetTangent(Vector4::DIRECTION_RIGHT);

		unsigned int index = PushVertex(cubeBounds.GetBackBottomLeft());

		SetUVs(bottomUVs.GetBottomRight());
		PushVertex(cubeBounds.GetBackBottomRight());

		SetUVs(bottomUVs.GetTopRight());
		PushVertex(cubeBounds.GetFrontBottomRight());

		SetUVs(bottomUVs.GetTopLeft());
		PushVertex(cubeBounds.GetFrontBottomLeft());

		PushIndices(index + 0, index + 1, index + 2);
		PushIndices(index + 0, index + 2, index + 3);
	}
}


//-----------------------------------------------------------------------------------------------
// Pushes the vertices and indices of a UV-generated sphere
//
void MeshBuilder::PushUVSphere(const Vector3& spherePosition, float radius, unsigned int numWedges, unsigned int numSlices, const Rgba& color /*= Rgba::WHITE*/)
{
	AssertBuildState(true, PRIMITIVE_TRIANGLES, true);

	SetColor(color);

	// Pushing the vertices
	for (int sliceIndex = 0; sliceIndex <= (int) numSlices; ++sliceIndex)
	{
		float v = (float) sliceIndex / (float) numSlices;
		float azimuth = RangeMapFloat(v, 0.f, 1.0f, 180.f, 0.f);

		for (int wedgeIndex = 0; wedgeIndex <= (int) numWedges; ++wedgeIndex)
		{
			float u = (float) wedgeIndex / (float) numWedges;
			float rotationAngle = 360.f * u;

			Vector3 vertexPosition = spherePosition + SphericalToCartesian(radius, rotationAngle, azimuth);
			Vector3 normal = (vertexPosition - spherePosition).GetNormalized();
			
			// Tangent - set the tangents at the poles to local RIGHT
			Vector3 tangent = Vector3(-SinDegrees(rotationAngle) * SinDegrees(azimuth), 0.f, CosDegrees(rotationAngle) * SinDegrees(azimuth));
			if (AreMostlyEqual(tangent, Vector3::ZERO))
			{
				tangent = Vector3::DIRECTION_RIGHT;
			}
			tangent.NormalizeAndGetLength();

			SetUVs(Vector2(u, v));
			SetNormal(normal);
			SetTangent(Vector4(tangent, 1.0f));
			PushVertex(vertexPosition);
		}
	}

	// Pushing the indices
	unsigned int numVerticesPerSlice = numWedges + 1;	// We push a vertex at the end of a slice that coincides with the first vertex of that slice, hence + 1

	for (int sliceIndex = 0; sliceIndex < (int) numSlices; ++sliceIndex)
	{
		for (int wedgeIndex = 0; wedgeIndex < (int) numWedges; ++wedgeIndex)
		{
			unsigned int bottomLeft		= numVerticesPerSlice * sliceIndex + wedgeIndex;
			unsigned int bottomRight	= bottomLeft + 1;
			unsigned int topRight		= bottomRight + numVerticesPerSlice;
			unsigned int topLeft		= bottomLeft + numVerticesPerSlice;

			PushIndices(bottomLeft, bottomRight, topRight);
			PushIndices(bottomLeft, topRight, topLeft);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Pushes the vertices for a mesh generated by the given surface patch function
//
void MeshBuilder::PushSurfacePatch(SurfacePatchFunction patchFunction, unsigned int numUSteps /*= 10*/, unsigned int numVSteps /*= 10*/, const Rgba& color /* = Rgba::WHITE*/)
{
	AssertBuildState(true, PRIMITIVE_TRIANGLES, true);
	SetColor(color);

	// Generate the vertices
	for (int vIndex = 0; vIndex <= (int) numVSteps; ++vIndex)
	{
		float v = (float) vIndex / numVSteps;

		for (int uIndex = 0; uIndex <= (int) numUSteps; ++uIndex)
		{
			float u = (float) uIndex / numUSteps;
			Vector3 position = patchFunction(Vector2(u, v));

			// Calculate tangent and normal
			float h = 0.01f;
			Vector3 tangent = (patchFunction(Vector2(u + h, v)) - patchFunction(Vector2(u - h, v))) * 0.5f * (1.f / h);
			tangent.NormalizeAndGetLength();
			Vector3 bitangent = (patchFunction(Vector2(u, v + h)) - patchFunction(Vector2(u, v - h))) * 0.5f * (1.f / h);

			Vector3 normal = CrossProduct(bitangent, tangent).GetNormalized();

			// Set and push
			SetUVs(Vector2(u, v));
			SetNormal(normal);
			SetTangent(Vector4(tangent, 1.0f));
			PushVertex(position);		
		}
	}

	// Push indices to generate the mesh

	int numVerticesAlongU = numUSteps + 1;
	for (int vIndex = 0; vIndex < (int) numVSteps; ++vIndex)
	{
		for (int uIndex = 0; uIndex < (int) numUSteps; ++uIndex)
		{
			unsigned int bottomLeft		= numVerticesAlongU * vIndex + uIndex;
			unsigned int bottomRight	= bottomLeft + 1;
			unsigned int topRight		= bottomRight + numVerticesAlongU;
			unsigned int topLeft		= bottomLeft + numVerticesAlongU;

			PushIndices(bottomLeft, bottomRight, topRight);
			PushIndices(bottomLeft, topRight, topLeft);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks to ensure the given params match the state of the builder, and throws an error if state
// doesn't match
//
void MeshBuilder::AssertBuildState(bool shouldBeBuilding, PrimitiveType primitiveType, bool shouldUseIndices) const
{
	bool buildStateMatches		= (shouldBeBuilding == m_isBuilding);					// If we should/shouldn't be building, and we aren't/are
	bool primitiveTypeMatches	= (primitiveType == m_instruction.m_primType);			// If part of the mesh uses one primitive, and part uses another
	bool indexStateMatches		= (shouldUseIndices == m_instruction.m_usingIndices);	// If part of the mesh uses indices and part doesn't

	ASSERT_OR_DIE(buildStateMatches,	Stringf("Error: MeshBuilder::AssertBuildState() failed on build state check."));

	// Only check these if we survive the build check, and that we are building. If we're not building, these get set when we start building.
	if (m_isBuilding)
	{
		ASSERT_OR_DIE(primitiveTypeMatches, Stringf("Error: MeshBuilder::AssertBuildState() failed on primitive type check."));
		ASSERT_OR_DIE(indexStateMatches,	Stringf("Error: MeshBuilder::AssertBuildState() failed on index state check."));
	}
}


//-----------------------------------------------------------------------------------------------
// For object loading, parses the token and returns a vertex master
//
VertexMaster MeshBuilder::CreateMasterFromString(const std::string& text, const std::vector<Vector3>& positions, const std::vector<Vector3>& normals, const std::vector<Vector2>& uvs)
{
	std::vector<std::string> indicesText = Tokenize(text, '/');

	// OBJ FILES USE 1-BASED INDEXING, SO SUBTRACT ONE FROM THEM
	VertexMaster master;
	master.m_color = Rgba::WHITE;

	// First is index for position, always (it's required)
	int positionIndex = StringToInt(indicesText[0]) - 1;
	master.m_position = positions[positionIndex];

	// If either text coord or normal was omitted, then we need to see which was omitted
	if (indicesText.size() == 2)
	{
		bool normalWasSpecified = (GetCharacterCount(text, '/') == 2); // Two slashes means position and normal

		if (normalWasSpecified)
		{
			int normalIndex = StringToInt(indicesText[1]) - 1;
			master.m_normal = normals[normalIndex];
		}
		else
		{
			int uvIndex = StringToInt(indicesText[1]) - 1;
			master.m_uvs = uvs[uvIndex];
		}
	}
	else
	{
		// Both UV and normal were specified
		int uvIndex = StringToInt(indicesText[1]) - 1;
		int normalIndex = StringToInt(indicesText[2]) - 1;

		master.m_normal = normals[normalIndex];
		master.m_uvs = uvs[uvIndex];
	}

	return master;
}


//-------------------------MikkT Tangent Space--------------------------------

static int GetNumFaces(const SMikkTSpaceContext* pContext)
{
	MeshBuilder* mb = (MeshBuilder*)pContext->m_pUserData;
	return mb->GetNumTriangles();
}

static int GetNumVerticesPerFace(const SMikkTSpaceContext* pContext, const int iFace)
{
	UNUSED(pContext);
	UNUSED(iFace);
	return 3;	// always return 3, since a face in our engine is always 3 verts
}

static void GetVertexPosition(const SMikkTSpaceContext * pContext, float fvPosOut[], const int iFace, const int iVert)
{
	MeshBuilder* mb = (MeshBuilder*)pContext->m_pUserData;
	int vertexIndex = iFace * 3 + iVert;

	Vector3 position = mb->GetVertexPosition(vertexIndex);

	fvPosOut[0] = position.x;
	fvPosOut[1] = position.y;
	fvPosOut[2] = position.z;
}

static void GetVertexNormal(const SMikkTSpaceContext * pContext, float fvNormOut[], const int iFace, const int iVert)
{
	MeshBuilder* mb = (MeshBuilder*)pContext->m_pUserData;
	int vertexIndex = iFace * 3 + iVert;

	Vector3 normal = mb->GetVertexNormal(vertexIndex);

	fvNormOut[0] = normal.x;
	fvNormOut[1] = normal.y;
	fvNormOut[2] = normal.z;
}

static void GetVertexUV(const SMikkTSpaceContext * pContext, float fvTexcOut[], const int iFace, const int iVert)
{
	MeshBuilder* mb = (MeshBuilder*)pContext->m_pUserData;
	int vertexIndex = iFace * 3 + iVert;

	Vector2 uv = mb->GetVertexUV(vertexIndex);

	fvTexcOut[0] = uv.x;
	fvTexcOut[1] = uv.y;
}


void SetVertexTangent(const SMikkTSpaceContext * pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert)
{
	MeshBuilder* mb = (MeshBuilder*)pContext->m_pUserData;
	int vertexIndex = iFace * 3 + iVert;

	Vector4 tangent = Vector4(fvTangent[0], fvTangent[1], fvTangent[2], fSign);

	mb->SetVertexTangent(vertexIndex, tangent);
}


bool GenerateMikkTangents(MeshBuilder& mb)
{
	SMikkTSpaceInterface mikkInterface;

	mikkInterface.m_getNumFaces = GetNumFaces;
	mikkInterface.m_getNumVerticesOfFace = GetNumVerticesPerFace;
	mikkInterface.m_getPosition = GetVertexPosition;
	mikkInterface.m_getNormal = GetVertexNormal;
	mikkInterface.m_getTexCoord = GetVertexUV;

	mikkInterface.m_setTSpaceBasic = SetVertexTangent;
	mikkInterface.m_setTSpace = NULL;

	SMikkTSpaceContext context;
	context.m_pInterface = &mikkInterface;
	context.m_pUserData = &mb;

	return genTangSpaceDefault(&context);
}
