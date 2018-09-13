/************************************************************************/
/* File: Vertex.hpp
/* Author: Andrew Chase
/* Date: April 7th, 2018
/* Description: Collection of all vertex data for the engine
/************************************************************************/
#pragma once
#include <vector>
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Rendering/OpenGL/glTypes.hpp"

#define MAX_BONES_PER_VERTEX (4)

//-----Description for a single attribute of a vertex, a layout is made up of a collection of these-----
struct VertexAttribute
{
	VertexAttribute() : m_name("") {} // For null terminator in ATTRIBUTES array (unused)
	VertexAttribute(const std::string name, RenderDataType type, unsigned int elementCount, bool isNormalized, unsigned int memberOffset)
	: m_name(name), m_dataType(type), m_elementCount(elementCount), m_isNormalized(isNormalized), m_memberOffset(memberOffset) 
	{}

	std::string		m_name;
	RenderDataType	m_dataType;
	bool			m_isNormalized;
	unsigned int	m_elementCount;
	size_t			m_memberOffset;
};

//-----Layout description for a given vertex type-----
class VertexLayout
{
public:
	//-----Public Data-----

	VertexLayout(unsigned int stride, unsigned int numAttributes, const VertexAttribute* attributes);

	unsigned int			GetAttributeCount() const;
	const VertexAttribute&	GetAttribute(unsigned int index) const;
	unsigned int			GetStride() const;


private:
	//-----Private Data-----

	const VertexAttribute*			m_attributes;
	unsigned int					m_numAttributes;
	unsigned int					m_vertexStride;
};

//--------------------VERTEX TYPES--------------------
// Defined by a layout (collection of attributes)


// Used to construct all vertex types in MeshBuilder
struct VertexMaster
{
	Vector3 m_position	= Vector3::ZERO;
	Vector2 m_uvs		= Vector2::ZERO;
	Rgba	m_color		= Rgba::WHITE;
	Vector3 m_normal	= Vector3::ZERO;
	Vector4 m_tangent	= Vector4::ZERO;

	unsigned int	m_boneIndices[MAX_BONES_PER_VERTEX] = {0, 0, 0, 0};
	float			m_boneWeights[MAX_BONES_PER_VERTEX] = {0.f, 0.f, 0.f, 0.f};
};

//-----------------------------------------------------------------------------------------------
// Basis Vertex
//
struct Vertex3D_PCU
{
	// Constructors
	Vertex3D_PCU() {};
	Vertex3D_PCU(const Vector3& position, const Rgba& color, const Vector2& texUVs)
		: m_position(position), m_color(color), m_texUVs(texUVs) {}

	Vertex3D_PCU(const VertexMaster& master)
	: m_position(master.m_position), m_color(master.m_color), m_texUVs(master.m_uvs) {}

	Vector3 m_position;	// Position of the Vertex
	Rgba	m_color;	// Color of the Vertex
	Vector2 m_texUVs;	// Texture UV coordinates for this vertex

	static const VertexAttribute	ATTRIBUTES[];
	static const VertexLayout		LAYOUT;
	static const unsigned int		NUM_ATTRIBUTES;
};


//-----------------------------------------------------------------------------------------------
// Lit Vertex
//
struct VertexLit
{
	// Constructors
	VertexLit() {};
	VertexLit(const Vector3& position, const Rgba& color, const Vector2& texUVs, const Vector3& normal, const Vector4& tangent)
		: m_position(position), m_color(color), m_texUVs(texUVs), m_normal(normal), m_tangent(tangent) {}

	// Construction from the master
	VertexLit(const VertexMaster& master)
		: m_position(master.m_position), m_color(master.m_color), m_texUVs(master.m_uvs), m_normal(master.m_normal), m_tangent(master.m_tangent) 
	{

	}

	Vector3 m_position;	// Position of the Vertex
	Rgba	m_color;	// Color of the Vertex
	Vector2 m_texUVs;	// Texture UV coordinates for this vertex

	Vector3 m_normal;	// Normal to the surface at this vertex
	Vector4 m_tangent;	// Tangent to the surface at this vertex, w = 1 signals the cross direction for the bitangent

	static const VertexAttribute	ATTRIBUTES[];
	static const VertexLayout		LAYOUT;
	static const unsigned int		NUM_ATTRIBUTES;
};


//-----------------------------------------------------------------------------------------------
// Skinned Vertex
//
struct VertexSkinned
{
	// Constructors
	VertexSkinned() {};
	VertexSkinned(const Vector3& position, const Rgba& color, const Vector2& texUVs, const Vector3& normal, const Vector4& tangent)
		: m_position(position), m_color(color), m_texUVs(texUVs), m_normal(normal), m_tangent(tangent) 
	{
		
	}

	// Construction from the master
	VertexSkinned(const VertexMaster& master)
		: m_position(master.m_position), m_color(master.m_color), m_texUVs(master.m_uvs), m_normal(master.m_normal), m_tangent(master.m_tangent) 
	{
		for (int i = 0; i < MAX_BONES_PER_VERTEX; ++i)
		{
			m_bones[i] = master.m_boneIndices[i];
			m_boneWeights[i] = master.m_boneWeights[i];
		}
	}

	Vector3 m_position;	// Position of the Vertex
	Rgba	m_color;	// Color of the Vertex
	Vector2 m_texUVs;	// Texture UVs coordinates for this vertex

	Vector3 m_normal;	// Normal to the surface at this vertex
	Vector4 m_tangent;	// Tangent to the surface at this vertex, w = 1 signals the cross direction for the bitangent

	unsigned int 	m_bones[MAX_BONES_PER_VERTEX];			// Max number of bones this vertex can be weighted to
	float			m_boneWeights[MAX_BONES_PER_VERTEX];	// Weights corresponding to the bones this vertex is weighted to

	static const VertexAttribute	ATTRIBUTES[];
	static const VertexLayout		LAYOUT;
	static const unsigned int		NUM_ATTRIBUTES;
};


//-----------------------------------------------------------------------------------------------
// Byte-aligned voxel vertex
//
struct VertexVoxel
{
	// Constructors
	VertexVoxel() {};
	VertexVoxel(const Vector3& position, const Rgba& color)
		: m_position(position), m_color(color) {}

	VertexVoxel(const VertexMaster& master)
		: m_position(master.m_position), m_color(master.m_color) {}

	Vector3 m_position;	// Position of the Vertex
	Rgba	m_color;	// Color of the Vertex

	static const VertexAttribute	ATTRIBUTES[];
	static const VertexLayout		LAYOUT;
	static const unsigned int		NUM_ATTRIBUTES;
};
