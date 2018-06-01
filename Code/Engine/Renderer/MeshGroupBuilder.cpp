#include "Engine/Core/File.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/MeshGroupBuilder.hpp"

MeshGroupBuilder::~MeshGroupBuilder()
{
	int numBuilders = (int) m_meshBuilders.size();

	for (int builderIndex = 0; builderIndex < numBuilders; ++builderIndex)
	{
		delete m_meshBuilders[builderIndex];
	}

	m_meshBuilders.clear();
}

void MeshGroupBuilder::LoadFromObjFile(const std::string& filePath)
{
	// Load the file

	// Iterate across all the lines, collecting the vertex and index information

	// When we start hitting faces, start pushing to meshbuilder

	// Once we hit the usemtl, create the mesh, add to list, and clear mb

	MeshBuilder* mb = new MeshBuilder();
	mb->BeginBuilding(PRIMITIVE_TRIANGLES, false);

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
		bool isMaterialChange = (tokens[0] == "usemtl");

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
			bool isTwoTriangles = (tokens.size() == 5); // 5 = 1 from 'f', 4 from the 4 verts

			std::vector<VertexMaster> vertices;

			// Start at one to skip the 'f' flag
			for (int tokenIndex = 1; tokenIndex < (int) tokens.size(); ++tokenIndex)
			{
				vertices.push_back(MeshBuilder::CreateMasterFromString(tokens[tokenIndex], positions, normals, uvs));
			}

			// Push the vertices in order
			mb->PushVertex(vertices[0]);
			mb->PushVertex(vertices[1]);
			mb->PushVertex(vertices[2]);

			if (isTwoTriangles)
			{
				mb->PushVertex(vertices[0]);
				mb->PushVertex(vertices[2]);
				mb->PushVertex(vertices[3]);
			}
		}
		else if (isMaterialChange)
		{
			// Finalize the MeshBuilder, if it has data
			if (mb->GetNumTriangles() > 0)
			{
				// Flip the Mesh horizontally, since OBJ files use a right-handed basis
				mb->FlipHorizontal();

				// Generate normals (and tangents), only if none were specified in the OBJ file
				bool normalsSpecified = (normals.size() > 0);

				if (!normalsSpecified)
				{
					mb->GenerateFlatTBN();
				}
				else
				{
					// Generate tangents using existing normals
					GenerateMikkTangents(*mb);
				}

				mb->FinishBuilding();

				// Add the builder to the list
				m_meshBuilders.push_back(mb);

				// Create a new builder here for the next mesh
				mb = new MeshBuilder();		
				mb->BeginBuilding(PRIMITIVE_TRIANGLES, false);
			}
		}
	}

	// Create a mesh from what's remaining in the builder, if there is any
	// Finalize the MeshBuilder, if it has data
	if (mb->GetNumTriangles() > 0)
	{
		// Flip the Mesh horizontally, since OBJ files use a right-handed basis
		mb->FlipHorizontal();

		// Generate normals (and tangents), only if none were specified in the OBJ file
		bool normalsSpecified = (normals.size() > 0);

		if (!normalsSpecified)
		{
			mb->GenerateFlatTBN();
		}
		else
		{
			// Generate tangents using existing normals
			GenerateMikkTangents(*mb);
		}

		mb->FinishBuilding();

		// Add the builder to the list
		m_meshBuilders.push_back(mb);
	}
}

