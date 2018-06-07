/************************************************************************/
/* File: glTypes.cpp
/* Author: Andrew Chase
/* Date: April 7th
/* Description: Implementation of GL type conversion functions
/************************************************************************/
#include "Engine/Rendering/OpenGL/glTypes.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "ThirdParty/gl/glcorearb.h"
#include "ThirdParty/gl/glext.h"
#include "ThirdParty/gl/wglext.h"

//-----------------------------------------------------------------------------------------------
// Depth
//
unsigned int g_openGLDepthModes[ NUM_DEPTH_TESTS ] =
{
	GL_NEVER,       // called COMPARE_NEVER			in our engine
	GL_LESS,        // called COMPARE_LESS			in our engine
	GL_LEQUAL,      // called COMPARE_LEQUAL		in our engine
	GL_GREATER,     // called COMPARE_GREATER		in our engine
	GL_GEQUAL,      // called COMPARE_GEQUAL		in our engine
	GL_EQUAL,       // called COMPARE_EQUAL			in our engine
	GL_NOTEQUAL,	// called COMPARE_NOT_EQUAL		in our engine
	GL_ALWAYS,      // called COMPARE_ALWAYS		in our engine
};

unsigned int ToGLType(DepthTest mode) { return g_openGLDepthModes[mode]; }

//-----------------------------------------------------------------------------------------------
// Data types for attribute binding
//
unsigned int g_openGLDataTypes[NUM_RDTS] =
{
	GL_FLOAT,
	GL_UNSIGNED_BYTE
};

unsigned int ToGLType(RenderDataType type) { return g_openGLDataTypes[type]; }


//-----------------------------------------------------------------------------------------------
// Draw primitive types
//
unsigned int g_openGlPrimitiveTypes[NUM_PRIMITIVE_TYPES] =
{
	GL_POINTS,			// called PRIMITIVE_POINTS		in our engine
	GL_LINES,			// called PRIMITIVE_LINES		in our engine
	GL_TRIANGLES,		// called PRIMITIVE_TRIANGES	in our engine
	GL_QUADS			// called PRIMITIVE_QUADS		in our engine
};

unsigned int ToGLType(PrimitiveType type) { return g_openGlPrimitiveTypes[type]; }


//-----------------------------------------------------------------------------------------------
// Fill modes
//
unsigned int g_openGLFillModes[NUM_FILL_MODES] =
{
	GL_FILL,
	GL_LINE
};

unsigned int ToGLType(FillMode mode) { return g_openGLFillModes[mode]; }


//-----------------------------------------------------------------------------------------------
// Wind order
//
unsigned int g_openGLWindOrders[NUM_WIND_ORDERS] =
{
	GL_CW,
	GL_CCW
};

unsigned int ToGLType(WindOrder order) { return g_openGLWindOrders[order]; }


//-----------------------------------------------------------------------------------------------
// Blending
//
unsigned int g_openGLBlendOps[NUM_BLEND_OPS] = 
{
	GL_FUNC_ADD,
	GL_FUNC_SUBTRACT,
	GL_FUNC_REVERSE_SUBTRACT,
	GL_MIN,
	GL_MAX,
};

unsigned int g_openGLBlendFactors[NUM_BLEND_FACTORS] =
{
	GL_ZERO,
	GL_ONE,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA
};

unsigned int ToGLType(BlendOp op) { return g_openGLBlendOps[op]; }
unsigned int ToGLType(BlendFactor factor) { return g_openGLBlendFactors[factor]; }



//-----------------------------------------------------------------------------------------------
// Returns the size of the given GL type
//
unsigned int GetGLTypeSize(unsigned int type)
{
	GLenum glType = type;

	unsigned int size = 0;
	switch (glType)
	{
	case GL_FLOAT_VEC2:
		size = 2 * 4;
		break;
	case GL_FLOAT_VEC3:
		size = 3 * 4;
		break;
	case GL_FLOAT_VEC4:
		size = 4 * 4;
		break;
	case GL_FLOAT:
	case GL_INT:
	case GL_UNSIGNED_INT:
		size = 4;
		break;
	case GL_FLOAT_MAT4:
		size = 64;
		break;
	default:
		ERROR_AND_DIE("Error: GetGLTypeSize() received bad GL enum.");
	}

	return size;
}


//-----------------------------------------------------------------------------------------------
// Sampler Filtering
//
unsigned int g_openGLSamplerFilters[NUM_SAMPLER_FILTERS] =
{
	GL_NEAREST,
	GL_LINEAR,
	GL_NEAREST_MIPMAP_NEAREST,
	GL_LINEAR_MIPMAP_NEAREST,
	GL_NEAREST_MIPMAP_LINEAR,
	GL_LINEAR_MIPMAP_LINEAR
};

unsigned int ToGLType(SamplerFilter filter) { return g_openGLSamplerFilters[filter]; }


//-----------------------------------------------------------------------------------------------
// Edge Sampling
//
unsigned int g_openGLEdgeSamplings[NUM_EDGE_SAMPLINGS] =
{
	GL_REPEAT,
	GL_MIRRORED_REPEAT,
	GL_CLAMP_TO_EDGE,
	GL_CLAMP_TO_BORDER,
	GL_MIRROR_CLAMP_TO_EDGE
};

unsigned int ToGLType(EdgeSampling sampling) { return g_openGLEdgeSamplings[sampling]; }


//-----------------------------------------------------------------------------------------------
// Texture Formatting
//

// Texture Data
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// |-------------------------------------------------------------------------------------------------------| //
// |      Image Format       |    Internal Format      |        Channels         |      Pixel Layout       | //
// |-------------------------|-------------------------|-------------------------|-------------------------| //
// |     R8 (1 channel)      |          GL_R8          |         GL_RED          |     GL_UNSIGNED_BYTE    | //
// |-------------------------|-------------------------|-------------------------|-------------------------| //
// |    RG8 (2 Channel)      |         GL_RG8          |         GL_RG           |     GL_UNSIGNED_BYTE    | //
// |-------------------------|-------------------------|-------------------------|-------------------------| //
// |    RGB8 (3 Channel)     |         GL_RGB8         |         GL_RGB          |     GL_UNSIGNED_BYTE    | //
// |-------------------------|-------------------------|-------------------------|-------------------------| //
// |    RGBA8 (4 Channel)    |        GL_RGBA8         |         GL_RGBA         |     GL_UNSIGNED_BYTE    | //
// |-------------------------|-------------------------|-------------------------|-------------------------| //
// |D24S8 (Depth24/Stencil8) |   GL_DEPTH24_STENCIL8   |     GL_DEPTH_STENCIL    |   GL_UNSIGNED_INT_24_8  | //
// |-------------------------|-------------------------|-------------------------|-------------------------| //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Internal Format
int g_openGLInternalFormats[NUM_TEXTURE_FORMATS] =
{
	GL_R8,
	GL_RG8,
	GL_RGB8,
	GL_RGBA8,
	GL_DEPTH24_STENCIL8
};

// Channels
int g_openGLChannels[NUM_TEXTURE_FORMATS] =
{
	GL_RED,
	GL_RG,
	GL_RGB,
	GL_RGBA,
	GL_DEPTH_STENCIL
};

// Pixel Layouts
int g_openGLPixelLayouts[NUM_TEXTURE_FORMATS] = 
{
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_INT_24_8
};

unsigned int ToGLInternalFormat(TextureFormat format) { return g_openGLInternalFormats[format]; }
unsigned int ToGLChannel(TextureFormat format) { return g_openGLChannels[format]; }
unsigned int ToGLPixelLayout(TextureFormat format) { return g_openGLPixelLayouts[format]; }


//-----------------------------------------------------------------------------------------------
// Texture Target Structure
//
int g_openGLTextureTypes[NUM_TEXTURE_TYPES]
{
	GL_TEXTURE_2D,
	GL_TEXTURE_CUBE_MAP
};

unsigned int ToGLType(TextureType type) { return g_openGLTextureTypes[type]; }


//-----------------------------------------------------------------------------------------------
// Texture Cube Sides
//
int g_openGLTextureSides[NUM_TEXCUBE_SIDES] = 
{
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

unsigned int ToGLType(TexCubeSide side) { return g_openGLTextureSides[side]; }