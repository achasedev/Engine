/************************************************************************/
/* File: glTypes.hpp
/* Author: Andrew Chase
/* Date: April 7th
/* Description: Collection of all Engine data types representing GL types
/************************************************************************/
#pragma once

//-----------------------------------------------------------------------------------------------
// Depth
//
enum DepthTest
{
	DEPTH_TEST_NEVER,       // GL_NEVER
	DEPTH_TEST_LESS,        // GL_LESS
	DEPTH_TEST_LEQUAL,      // GL_LEQUAL
	DEPTH_TEST_GREATER,     // GL_GREATER
	DEPTH_TEST_GEQUAL,      // GL_GEQUAL
	DEPTH_TEST_EQUAL,       // GL_EQUAL
	DEPTH_TEST_NOT_EQUAL,   // GL_NOTEQUAL
	DEPTH_TEST_ALWAYS,      // GL_ALWAYS
	NUM_DEPTH_TESTS		 // Count
};

unsigned int ToGLType(DepthTest mode);


//-----------------------------------------------------------------------------------------------
// Data types for attribute binding
//
enum RenderDataType
{
	RDT_FLOAT,
	RDT_UNSIGNED_BYTE,
	RDT_UNSIGNED_INT,
	NUM_RDTS
};

unsigned int ToGLType(RenderDataType type);


//-----------------------------------------------------------------------------------------------
// Draw primitive types
//
enum PrimitiveType
{
	PRIMITIVE_POINTS,		// in OpenGL, for example, this becomes GL_POINTS
	PRIMITIVE_LINES,		// in OpenGL, for example, this becomes GL_LINES
	PRIMITIVE_TRIANGLES,	// in OpenGL, for example, this becomes GL_TRIANGLES
	PRIMITIVE_QUADS,		// in OpenGL, for example, this becomes GL_QUADS
	NUM_PRIMITIVE_TYPES
};

unsigned int ToGLType(PrimitiveType type);


//-----------------------------------------------------------------------------------------------
// Cull modes
//
enum CullMode
{
	CULL_MODE_NONE,
	CULL_MODE_BACK,
	CULL_MODE_FRONT,
	NUM_CULL_MODES
};

// No conversion function needed


//-----------------------------------------------------------------------------------------------
// Fill modes
//
enum FillMode
{
	FILL_MODE_SOLID,
	FILL_MODE_WIRE,
	NUM_FILL_MODES
};

unsigned int ToGLType(FillMode mode);


//-----------------------------------------------------------------------------------------------
// Wind order
//
enum WindOrder
{
	WIND_CLOCKWISE,
	WIND_COUNTER_CLOCKWISE,
	NUM_WIND_ORDERS
};

unsigned int ToGLType(WindOrder order);


//-----------------------------------------------------------------------------------------------
// Blending
//
enum BlendOp
{
	BLEND_OP_ADD,
	BLEND_OP_SUBTRACT,
	BLEND_OP_REVERSE_SUBTRACT,
	BLEND_OP_MIN,
	BLEND_OP_MAX,
	NUM_BLEND_OPS
};

enum BlendFactor
{
	BLEND_FACTOR_ZERO,
	BLEND_FACTOR_ONE,
	BLEND_FACTOR_SOURCE_ALPHA,
	BLEND_FACTOR_ONE_MINUS_SOURCE_ALPHA,
	BLEND_FACTOR_DESTINATION_ALPHA,
	BLEND_FACTOR_ONE_MINUS_DESTINATION_ALPHA,
	NUM_BLEND_FACTORS
};

unsigned int ToGLType(BlendOp op);
unsigned int ToGLType(BlendFactor factor);

unsigned int GetGLTypeSize(unsigned int type);


//-----------------------------------------------------------------------------------------------
// Sampler filtering
//
enum SamplerFilter
{
	SAMPLER_FILTER_NEAREST,
	SAMPLER_FILTER_LINEAR,
	SAMPLER_FILTER_NEAREST_MIPMAP_NEAREST,
	SAMPLER_FILTER_LINEAR_MIPMAP_NEAREST,
	SAMPLER_FILTER_NEAREST_MIPMAP_LINEAR,
	SAMPLER_FILTER_LINEAR_MIPMAP_LINEAR,
	NUM_SAMPLER_FILTERS
};

unsigned int ToGLType(SamplerFilter filter);


//-----------------------------------------------------------------------------------------------
// Edge value sampling
//
enum EdgeSampling
{
	EDGE_SAMPLING_REPEAT,
	EDGE_SAMPLING_MIRRORED_REPEAT,
	EDGE_SAMPLING_CLAMP_TO_EDGE,
	EDGE_SAMPLING_CLAMP_TO_BORDER,
	EDGE_SAMPLING_MIRROR_CLAMP_TO_EDGE,
	NUM_EDGE_SAMPLINGS
};

unsigned int ToGLType(EdgeSampling sampling);


//-----------------------------------------------------------------------------------------------
// Texture format data
//
enum TextureFormat
{
	TEXTURE_FORMAT_R8,
	TEXTURE_FORMAT_RG8,
	TEXTURE_FORMAT_RGB8,
	TEXTURE_FORMAT_RGBA8,
	TEXTURE_FORMAT_D24S8,
	NUM_TEXTURE_FORMATS
};

unsigned int ToGLInternalFormat(TextureFormat format);
unsigned int ToGLChannel(TextureFormat format);
unsigned int ToGLPixelLayout(TextureFormat format);


//-----------------------------------------------------------------------------------------------
// Texture target structure
//
enum TextureType
{
	TEXTURE_TYPE_2D,
	TEXTURE_TYPE_CUBE_MAP,
	NUM_TEXTURE_TYPES
};

unsigned int ToGLType(TextureType type);


//-----------------------------------------------------------------------------------------------
// Texture Cube Sides
//
enum TexCubeSide
{
	TEXCUBE_RIGHT, 
	TEXCUBE_LEFT,  
	TEXCUBE_TOP,   
	TEXCUBE_BOTTOM,
	TEXCUBE_FRONT, 
	TEXCUBE_BACK,  
	NUM_TEXCUBE_SIDES
};

unsigned int ToGLType(TexCubeSide side);