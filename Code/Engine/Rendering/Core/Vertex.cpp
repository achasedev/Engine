/************************************************************************/
/* File: Vertex.cpp
/* Author: Andrew Chase
/* Date: April 7th, 2018
/* Description: Definitions of Vertex informations
/************************************************************************/
#include "Engine/Rendering/Core/Vertex.hpp"
#include "Engine/Core/EngineCommon.hpp"

//--------------------VERTEX TYPES--------------------

//-----Vertex3D_PCU (position, color, UVs)-----
const VertexAttribute Vertex3D_PCU::ATTRIBUTES[] =
{
	VertexAttribute("POSITION", RDT_FLOAT, 3, false, offsetof(Vertex3D_PCU, m_position)),
	VertexAttribute("COLOR", RDT_UNSIGNED_BYTE, 4, true, offsetof(Vertex3D_PCU, m_color)), 
	VertexAttribute("UV", RDT_FLOAT, 2, false, offsetof(Vertex3D_PCU, m_texUVs)),
};

const unsigned int Vertex3D_PCU::NUM_ATTRIBUTES = (sizeof(ATTRIBUTES) / sizeof(VertexAttribute));
const VertexLayout Vertex3D_PCU::LAYOUT = VertexLayout(sizeof(Vertex3D_PCU), NUM_ATTRIBUTES, Vertex3D_PCU::ATTRIBUTES);


//-----VertexLit (position, color, UVs, Normal, Tangent)-----
const VertexAttribute VertexLit::ATTRIBUTES[] =
{
	VertexAttribute("POSITION",		RDT_FLOAT,			3,		false,		offsetof(VertexLit, m_position)),
	VertexAttribute("COLOR",		RDT_UNSIGNED_BYTE,	4,		true,		offsetof(VertexLit, m_color)), 
	VertexAttribute("UV",			RDT_FLOAT,			2,		false,		offsetof(VertexLit, m_texUVs)),
	VertexAttribute("NORMAL",		RDT_FLOAT,			3,		false,		offsetof(VertexLit, m_normal)),
	VertexAttribute("TANGENT",		RDT_FLOAT,			3,		false,		offsetof(VertexLit, m_tangent))
};

const unsigned int VertexLit::NUM_ATTRIBUTES = (sizeof(ATTRIBUTES) / sizeof(VertexAttribute));
const VertexLayout VertexLit::LAYOUT = VertexLayout(sizeof(VertexLit), NUM_ATTRIBUTES, VertexLit::ATTRIBUTES);


//-----VertexSkinned (position, color, UVs, Normal, Tangent, bones indices, bone weights)-----
const VertexAttribute VertexSkinned::ATTRIBUTES[] =
{
	VertexAttribute("POSITION",			RDT_FLOAT,			3,						false,		offsetof(VertexSkinned, m_position)),
	VertexAttribute("COLOR",			RDT_UNSIGNED_BYTE,	4,						true,		offsetof(VertexSkinned, m_color)), 
	VertexAttribute("UV",				RDT_FLOAT,			2,						false,		offsetof(VertexSkinned, m_texUVs)),
	VertexAttribute("NORMAL",			RDT_FLOAT,			3,						false,		offsetof(VertexSkinned, m_normal)),
	VertexAttribute("TANGENT",			RDT_FLOAT,			3,						false,		offsetof(VertexSkinned, m_tangent)),
	VertexAttribute("BONE_IDS",			RDT_UNSIGNED_INT,	MAX_BONES_PER_VERTEX,	false,		offsetof(VertexSkinned, m_bones)),
	VertexAttribute("BONE_WEIGHTS",		RDT_FLOAT,			MAX_BONES_PER_VERTEX,	false,		offsetof(VertexSkinned, m_boneWeights))
};

const unsigned int VertexSkinned::NUM_ATTRIBUTES = (sizeof(ATTRIBUTES) / sizeof(VertexAttribute));
const VertexLayout VertexSkinned::LAYOUT = VertexLayout(sizeof(VertexSkinned), NUM_ATTRIBUTES, VertexSkinned::ATTRIBUTES);
//--------------------END VERTEX TYPES--------------------


//-----Vertex Layout-----

//-----------------------------------------------------------------------------------------------
// Constructor
//
VertexLayout::VertexLayout(unsigned int stride, unsigned int numAttributes, const VertexAttribute* attributes)
	: m_vertexStride(stride)
	, m_numAttributes(numAttributes)
	, m_attributes(attributes)
{
}


//-----------------------------------------------------------------------------------------------
// Returns the number of attributes in the layout
//
unsigned int VertexLayout::GetAttributeCount() const
{
	return m_numAttributes;
}


//-----------------------------------------------------------------------------------------------
// Returns the attribute at the given index
//
const VertexAttribute& VertexLayout::GetAttribute(unsigned int index) const
{
	GUARANTEE_OR_DIE(index >= 0 && index <m_numAttributes, Stringf("Error: VertexLayout::GetAttribute index out of range, index was %i", index));

	return m_attributes[index];
}


//-----------------------------------------------------------------------------------------------
// Returns the stride of this vertex layout (size of the vertex type it defines)
//
unsigned int VertexLayout::GetStride() const
{
	return m_vertexStride;
}
