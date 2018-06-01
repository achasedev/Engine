/************************************************************************/
/* File: Renderer.cpp
/* Author: Andrew Chase
/* Date: September January 25th, 2017
/* Bugs: None
/* Description: Implementation of the Renderer class
/************************************************************************/
#include <set>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/glFunctions.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Renderer/ShaderSource.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Core/Command.hpp"
#include "Engine/Core/File.hpp"
#include "Engine/Math/MathUtils.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image.h"
#include "ThirdParty/stb/stb_image_write.h"

Renderer*			Renderer::s_instance = nullptr;

const IntVector2	Renderer::FONT_SPRITE_LAYOUT	= IntVector2(16, 16);
const char*			Renderer::FONT_DIRECTORY		= "Data\\Fonts\\";
const char*			Renderer::SHADER_DIRECTORY		= "Data\\Shaders\\";
const float			Renderer::UI_ORTHO_HEIGHT		= 1080.f;
AABB2				Renderer::s_UIOrthoBounds;


// For converting primitive types to GL types
int g_openGlPrimitiveTypes[ NUM_PRIMITIVE_TYPES ] =
{
	GL_POINTS,			// called PRIMITIVE_POINTS		in our engine
	GL_LINES,			// called PRIMITIVE_LINES		in our engine
	GL_TRIANGLES,		// called PRIMITIVE_TRIANGES	in our engine
	GL_QUADS			// called PRIMITIVE_QUADS		in our engine
};

// For converting primitive types to GL types
int g_openGlDepthCompares[ NUM_COMPARES ] =
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


//-----------------------------------------------------------------------------------------------
// Constructor - the loaded textures map is already initialized by default to an empty map
// Sets the singleton instance to this renderer if none is created yet
//
Renderer::Renderer()
{
	// Ensure only one renderer exists
	GUARANTEE_OR_DIE(s_instance==nullptr, "Error: Renderer constructor called when a renderer already exists.");

	// Ensure a context already exists before creating this renderer (so gl functions handles are created and bound)
	GUARANTEE_OR_DIE(gHDC != nullptr, "Error: Renderer constructed without a gl context established first.");

	// Calls all GL functions necessary to set up the renderer
	PostGLStartup();

	// Initializing member on Renderer - No GL functions are called in the lines below

	// Set up the gpu render buffers
	m_vertexBuffer	= new RenderBuffer();
	m_indexBuffer	= new RenderBuffer();

	// Set the default shader program as the current program
	m_defaultShaderProgram	= CreateOrGetShaderProgram("Default");	// Should just "get" since the Default program is built in
	m_currentShaderProgram	= m_defaultShaderProgram;

	// setup the initial camera
	m_defaultCamera = new Camera();
	m_defaultCamera->SetColorTarget(m_defaultColorTarget); 
	m_defaultCamera->SetDepthTarget(m_defaultDepthTarget); 

	// Make the UI camera
	m_UICamera = new Camera();
	m_UICamera->SetColorTarget(m_defaultColorTarget);
	m_UICamera->SetDepthTarget(m_defaultDepthTarget);
	m_UICamera->SetProjection(Matrix44::MakeOrtho(Vector2::ZERO, Vector2(Window::GetInstance()->GetWindowAspect() * UI_ORTHO_HEIGHT, UI_ORTHO_HEIGHT)));

	// Make the Effects camera - all Identity transforms, we assign the color target in ApplyEffect()
	m_effectsCamera = new Camera();
	m_effectsCamera->SetColorTarget(m_defaultColorTarget); 

	SetCurrentCamera(m_defaultCamera);
}


//-----------------------------------------------------------------------------------------------
// Destructor - free all memory allocated to Renderer pointers
//
Renderer::~Renderer()
{
	//-----Free the loaded texture map-----
	std::map<std::string, Texture*>::iterator texItr = m_loadedTextures.begin();
	for(texItr; texItr != m_loadedTextures.end(); texItr++)
	{
		delete texItr->second;
	}
	m_loadedTextures.clear();


	//-----Free the loaded fonts-----
	std::map<std::string, BitmapFont*>::iterator fontItr = m_loadedFonts.begin();
	for (fontItr; fontItr != m_loadedFonts.end(); fontItr++)
	{
		delete fontItr->second;
	}
	m_loadedFonts.clear();


	//-----Free the loaded shader program map-----
	std::set<ShaderProgram*> valuesDeleted;
	std::map<std::string, ShaderProgram*>::iterator progItr = m_loadedShaderPrograms.begin();
	for(progItr; progItr != m_loadedShaderPrograms.end(); progItr++)
	{
		// Ensure we don't double-delete the same pointer!
		bool notDeleted = valuesDeleted.insert(progItr->second).second;
		if (notDeleted)
		{
			delete progItr->second;
		}
	}
	m_loadedShaderPrograms.clear();
}


//-----------------------------------------------------------------------------------------------
// Constructs the singleton Renderer instance
//
void Renderer::Initialize()
{
	GUARANTEE_OR_DIE(s_instance == nullptr, "Error: Renderer::Initialize() called when the Renderer instance exists.");
	s_instance = new Renderer();

	// Static setup
	s_UIOrthoBounds = AABB2(Vector2::ZERO, Vector2(UI_ORTHO_HEIGHT * Window::GetInstance()->GetWindowAspect(), UI_ORTHO_HEIGHT));
}


//-----------------------------------------------------------------------------------------------
// Deletes the Renderer instance
//
void Renderer::Shutdown()
{
	if (s_instance != nullptr)
	{
		delete s_instance;
		s_instance = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the singleton renderer instance
//
Renderer* Renderer::GetInstance()
{
	return s_instance;
}


//-----------------------------------------------------------------------------------------------
// Performs all Renderer procedures that occur at the beginning of the frame
//
void Renderer::BeginFrame()
{
	// Set the default shader program to the current program reference
	m_currentShaderProgram = m_defaultShaderProgram;
	SetCurrentCamera(nullptr);
	ClearScreen(Rgba::BLACK);
}


//-----------------------------------------------------------------------------------------------
// Performs all Renderer procedures that occur at the end of the frame
//
void Renderer::EndFrame()
{
	// Copy the default frame buffer to the back buffer before swapping
	m_defaultCamera->Finalize();
	CopyFrameBuffer( nullptr, &m_defaultCamera->m_frameBuffer ); 

	// "Present" the backbuffer by swapping in our color target buffer
	SwapBuffers(gHDC); 

	// Save off the (newly swapped) back buffer to file
	if (m_saveScreenshotThisFrame)
	{
		SaveScreenshotToFile();
	}
}


//-----------------------------------------------------------------------------------------------
// Clears the back buffer to a solid color
//
void Renderer::ClearScreen(const Rgba& clearColor)
{
	float red, green, blue, alpha;
	clearColor.GetAsFloats(red, green, blue, alpha);

	glClearColor(red, green, blue, alpha);
	glClear(GL_COLOR_BUFFER_BIT);
}


//-----------------------------------------------------------------------------------------------
// Applies a single shader draw effect to the entire default render target
//
void Renderer::ApplyImageEffect(ShaderProgram* shader)
{
	DisableDepth();

	// Set up the effects targets if they aren't already set up
	if (m_effectsSource == nullptr)
	{
		m_effectsSource = m_defaultColorTarget;
		if (m_effectsDestination == nullptr)
		{
			m_effectsDestination = new Texture();
			m_effectsDestination->CreateRenderTarget(m_effectsSource->GetDimensions().x, m_effectsSource->GetDimensions().y, TEXTURE_FORMAT_RGBA8);
		}
	}

	// Draw using the effects camera - to the scratch target
	m_effectsCamera->SetColorTarget(m_effectsDestination);
	SetCurrentCamera(m_effectsCamera);

	// Set the shader program to the one provided
	SetCurrentShaderProgram(shader);

	// Bind the current source as a texture
	BindTexture(0, m_effectsSource->GetHandle());
	
	// Draw the previous buffer as an AABB2 across the entire new render target
	DrawAABB2(AABB2::UNIT_SQUARE_CENTERED, AABB2::UNIT_SQUARE_OFFCENTER, Rgba::WHITE);

	// Swap the pointers around for the next effect
	Texture* temp			= m_effectsSource;
	m_effectsSource			= m_effectsDestination;
	m_effectsDestination	= temp;
}


//-----------------------------------------------------------------------------------------------
// Applies a single shader draw effect to the entire default render target
//
void Renderer::ApplyImageEffect(const std::string& shaderName)
{
	ShaderProgram* program = CreateOrGetShaderProgram(shaderName);
	ApplyImageEffect(program);
}


//-----------------------------------------------------------------------------------------------
// Cleans up the effect state after image effects have been applied
//
void Renderer::FinalizeImageEffects()
{
	// Null target means no effects have been applied, so nothing to finalize
	if (m_effectsSource == nullptr)
	{
		return;
	}

	// An odd number of effects were applied, so ensure the default color target is the final result
	if (m_effectsSource != m_defaultColorTarget)
	{
		Texture::CopyTexture(m_effectsSource, m_defaultColorTarget);
		m_effectsDestination = m_effectsSource;
	}

	// Signal we're done with our current effects processing
	m_effectsSource = nullptr;

	SetCurrentShaderProgram(nullptr);
	SetCurrentCamera(nullptr);
	EnableDepth(COMPARE_LESS, true);
}


//-----------------------------------------------------------------------------------------------
// Sets the blend mode to the one specified by the enumeration
//
void Renderer::SetBlendMode(BlendMode nextMode)
{
	switch (nextMode)
	{
	case BLEND_MODE_ERROR:
		break;
	case BLEND_MODE_ALPHA:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BLEND_MODE_ADDITIVE:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case NUM_BLEND_MODES:
		break;
	default:
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Enables the OpenGL macro GL_BLEND
//
void Renderer::EnableBlendMacro()
{
	glEnable(GL_BLEND);
}


//-----------------------------------------------------------------------------------------------
// Returns a pointer to the texture pointer corresponding to the given path.
// If no texture has been loaded for the given path yet, the texture is loaded and returned.
//
Texture* Renderer::CreateOrGetTexture(std::string texturePath)
{
	if (m_loadedTextures.find(texturePath) != m_loadedTextures.end())
	{
		return m_loadedTextures[texturePath];
	}

	Texture* newTexture = new Texture();
	newTexture->CreateFromFile(texturePath);
	m_loadedTextures[texturePath] = newTexture;

	return m_loadedTextures[texturePath];
}


//-----------------------------------------------------------------------------------------------
// Creates a render target to be used for intermediate rendering
//
Texture* Renderer::CreateRenderTarget(unsigned int width, unsigned int height, TextureFormat format /*= TEXTURE_FORMAT_RGBA8*/)
{
	Texture* texture = new Texture();
	texture->CreateRenderTarget(width, height, format);

	return texture;
}


//-----------------------------------------------------------------------------------------------
// Creates a depth stencil target texture and returns it
//
Texture* Renderer::CreateDepthTarget(unsigned int width, unsigned int height)
{
	return CreateRenderTarget(width, height, TEXTURE_FORMAT_D24S8);
}


//-----------------------------------------------------------------------------------------------
// Returns the color target used by the default camera
//
Texture* Renderer::GetDefaultColorTarget() const
{
	return m_defaultColorTarget;
}


//-----------------------------------------------------------------------------------------------
// Returns the depth target used by the default camera
//
Texture* Renderer::GetDefaultDepthTarget() const
{
	return m_defaultDepthTarget;
}


//-----------------------------------------------------------------------------------------------
// Sets the flag to take a screenshot during the next Renderer::EndFrame()
//
void Renderer::SaveScreenshotAtEndOfFrame(const std::string& filename)
{
	m_screenshotFilename = filename;
	m_saveScreenshotThisFrame = true;
}


//-----------------------------------------------------------------------------------------------
// Draws a textured AABB2 from the provided texture data
// textureUVs are the texture coordinates of the bottom left and top right of the box
//
void Renderer::DrawAABB2(const AABB2& bounds, const AABB2& textureUVs, const Rgba& tint)
{
	Vertex3D_PCU vertices[4];
	unsigned int indices[6];

	int numVertices = 0; 
	int numIndices = 0;

	// Construct the vertices/indices
	AppendAABB2Vertices2D(vertices, numVertices, indices, numIndices, bounds, textureUVs, tint);

	// Draw the AABB2
	DrawMeshImmediate(vertices, numVertices, PRIMITIVE_TRIANGLES, indices, numIndices);
}


//-----------------------------------------------------------------------------------------------
// Draws an oriented AABB2 in 3D space given the parameters
//
void Renderer::DrawAABB2_3D(const Vector3& position, const Vector2& dimensions, const AABB2& textureUVs, const Vector3& right /*= Vector3::DIRECTION_RIGHT*/, const Vector3& up /*= Vector3::DIRECTION_UP*/, const Rgba& tint /*= Rgba::WHITE*/, const Vector2& pivot /*= Vector2(0.5f, 0.5f)*/)
{
	Vertex3D_PCU vertices[4];
	unsigned int indices[6];
	int iCount = 0;
	int vCount = 0;

	AppendAABB2Vertices3D(vertices, vCount, indices, iCount, position, dimensions, textureUVs, right, up, tint, pivot);
	DrawMeshImmediate(vertices, vCount, PRIMITIVE_TRIANGLES, indices, iCount);
}


//-----------------------------------------------------------------------------------------------
// Appends the vertices for the AABB2 described into the passed vertex/index arrays
//
void Renderer::AppendAABB2Vertices2D(Vertex3D_PCU* vertexArray, int& vertexOffset, unsigned int* indexArray, int& indexOffset, const AABB2& bounds, const AABB2& textureUVs, const Rgba& tint /*= Rgba::WHITE*/)
{
	//-----Set up the vertices-----

	vertexArray[vertexOffset + 0] = Vertex3D_PCU(Vector3(bounds.mins.x, bounds.mins.y, 0.f), tint, textureUVs.GetBottomLeft());		// Bottom left
	vertexArray[vertexOffset + 1] = Vertex3D_PCU(Vector3(bounds.maxs.x, bounds.mins.y, 0.f), tint, textureUVs.GetBottomRight());	// Bottom right
	vertexArray[vertexOffset + 2] = Vertex3D_PCU(Vector3(bounds.maxs.x, bounds.maxs.y, 0.f), tint, textureUVs.GetTopRight());		// Top right
	vertexArray[vertexOffset + 3] = Vertex3D_PCU(Vector3(bounds.mins.x, bounds.maxs.y, 0.f), tint, textureUVs.GetTopLeft());		// Top left

	//-----Set up the indices-----

	// 3 - 2
	// | / |
	// 0 - 1

	indexArray[indexOffset + 0] = vertexOffset + 0;
	indexArray[indexOffset + 1] = vertexOffset + 1;
	indexArray[indexOffset + 2] = vertexOffset + 2;

	indexArray[indexOffset + 3] = vertexOffset + 0;
	indexArray[indexOffset + 4] = vertexOffset + 2;
	indexArray[indexOffset + 5] = vertexOffset + 3;

	// Update the offsets and return
	vertexOffset += 4;
	indexOffset += 6;
}


//-----------------------------------------------------------------------------------------------
// Appends the vertices for an oriented quad in 3D to a vertex array
//
void Renderer::AppendAABB2Vertices3D(Vertex3D_PCU* vertexArray, int& vertexOffset, unsigned int* indexArray, int& indexOffset, const Vector3& position,
	const Vector2& dimensions, const AABB2& textureUVs, const Vector3& right /*= Vector3::DIRECTION_RIGHT*/, const Vector3& up/*= Vector3::DIRECTION_UP*/, const Rgba& tint /*= Rgba::WHITE*/, const Vector2& pivot /*= Vector2(0.5f, 0.5f)*/)
{
	//-----Set up the vertices-----
	// Find the min and max X values for the AABB2 draw bounds
	float minX = -1.0f * (pivot.x * dimensions.x);
	float maxX = minX + dimensions.x;

	// Find the min and max X values for the sprite AABB2 draw bounds
	float minY = -1.0f * (pivot.y * dimensions.y);
	float maxY = minY + dimensions.y;

	Vector3 bottomLeft 		= position + minX * right + minY * up;
	Vector3 bottomRight 	= position + maxX * right + minY * up;
	Vector3 topLeft 		= position + minX * right + maxY * up;
	Vector3 topRight 		= position + maxX * right + maxY * up;

	vertexArray[vertexOffset + 0] = Vertex3D_PCU(bottomLeft,	tint, textureUVs.GetBottomLeft());		// Bottom left
	vertexArray[vertexOffset + 1] = Vertex3D_PCU(bottomRight,	tint, textureUVs.GetBottomRight());		// Bottom right
	vertexArray[vertexOffset + 2] = Vertex3D_PCU(topRight,		tint, textureUVs.GetTopRight());		// Top right
	vertexArray[vertexOffset + 3] = Vertex3D_PCU(topLeft,		tint, textureUVs.GetTopLeft());			// Top left

	//-----Set up the indices-----

	indexArray[indexOffset + 0] = vertexOffset + 0;
	indexArray[indexOffset + 1] = vertexOffset + 1;
	indexArray[indexOffset + 2] = vertexOffset + 2;

	indexArray[indexOffset + 3] = vertexOffset + 0;
	indexArray[indexOffset + 4] = vertexOffset + 2;
	indexArray[indexOffset + 5] = vertexOffset + 3;

	// Update the offsets and return
	vertexOffset += 4;
	indexOffset += 6;
}


//-----------------------------------------------------------------------------------------------
// Draws a cube with the given corner positions and tint
//
void Renderer::DrawCube(const Vector3& center, const Vector3& dimensions, const Rgba& tint /*= Rgba::WHITE*/, /* Draws a cube (cuboid) with the given corner positions and tint */ const AABB2& topUVs /*= AABB2::UNIT_SQUARE_OFFCENTER*/, const AABB2& sideUVs /*= AABB2::UNIT_SQUARE_OFFCENTER*/, const AABB2& bottomUVs /*= AABB2::UNIT_SQUARE_OFFCENTER*/)
{
	Vertex3D_PCU vertices[24];
	unsigned int indices[36];

	int vertexCount = 0;
	int indexCount = 0;

	AppendCubeVertices(vertices, vertexCount, indices, indexCount, center, dimensions, tint, topUVs, sideUVs, bottomUVs);

	// Draw the Cube
	DrawMeshImmediate(vertices, vertexCount, PRIMITIVE_TRIANGLES, indices, indexCount);
}


//-----------------------------------------------------------------------------------------------
// Draws the given sprite at the position with tint
//
void Renderer::DrawSprite(const Sprite* sprite, const Vector3& position, const Rgba& tint /*= Rgba::WHITE*/, const Vector3& right /*= Vector3::DIRECTION_RIGHT*/, const Vector3& up /*= Vector3::DIRECTION_UP*/)
{
	BindTexture(0, sprite->GetTexture().GetHandle());
	Vector2 spriteDimensions	= sprite->GetDimensions();
	Vector2 spritePivot			= sprite->GetPivot();
	AABB2 spriteUVs				= sprite->GetUVs();

	// Find the min and max X values for the sprite AABB2 draw bounds
	float minX = -1.0f * (spritePivot.x * spriteDimensions.x);
	float maxX = minX + spriteDimensions.x;

	// Find the min and max X values for the sprite AABB2 draw bounds
	float minY = -1.0f * (spritePivot.y * spriteDimensions.y);
	float maxY = minY + spriteDimensions.y;

	Vector3 bottomLeft 		= position + minX * right + minY * up;
	Vector3 bottomRight 	= position + maxX * right + minY * up;
	Vector3 topLeft 		= position + minX * right + maxY * up;
	Vector3 topRight 		= position + maxX * right + maxY * up;

	// Make the Vertex3D_PCU's (no indices)

	Vertex3D_PCU vertices[6];

	// First triangle
	vertices[0] = Vertex3D_PCU(bottomLeft, tint, spriteUVs.GetBottomLeft());
	vertices[1] = Vertex3D_PCU(bottomRight, tint, spriteUVs.GetBottomRight());
	vertices[2] = Vertex3D_PCU(topRight, tint, spriteUVs.GetTopRight());
	
	// Second triangle
	vertices[3] = vertices[0];
	vertices[4] = vertices[2];
	vertices[5] = Vertex3D_PCU(topLeft, tint, spriteUVs.GetTopLeft());

	// Draw the sprite
	DrawMeshImmediate(vertices, 6, PRIMITIVE_TRIANGLES);
}


//-----------------------------------------------------------------------------------------------
// Appends the vertices needed to draw the passed cube to the arrays passed and increments the offsets
//
void Renderer::AppendCubeVertices(Vertex3D_PCU* vertexArray, int& vertexOffset, unsigned int* indexArray, int& indexOffset, 
	const Vector3& center, const Vector3& dimensions, const Rgba& tint /*= Rgba::WHITE*/, const AABB2& topUVs /*= AABB2::UNIT_SQUARE_OFFCENTER*/, 
	const AABB2& sideUVs /*= AABB2::UNIT_SQUARE_OFFCENTER*/, const AABB2& bottomUVs /*= AABB2::UNIT_SQUARE_OFFCENTER*/)
{
	// Set up the corner vertices
	Vector3 frontBottomLeft;
	frontBottomLeft.x = center.x - dimensions.x * 0.5f;
	frontBottomLeft.y = center.y - dimensions.y * 0.5f;
	frontBottomLeft.z = center.z - dimensions.z * 0.5f;

	Vector3 backTopRight;
	backTopRight.x = center.x + dimensions.x * 0.5f;
	backTopRight.y = center.y + dimensions.y * 0.5f;
	backTopRight.z = center.z + dimensions.z * 0.5f;

	// Assemble the vertices in the vertex buffer

	//-------------------------------------Front face--------------------------------------------------------

	vertexArray[vertexOffset + 0] = Vertex3D_PCU(frontBottomLeft,												tint, sideUVs.GetBottomLeft());			// Bottom Left
	vertexArray[vertexOffset + 1] = Vertex3D_PCU(Vector3(backTopRight.x, frontBottomLeft.y, frontBottomLeft.z), tint, sideUVs.GetBottomRight());		// Bottom Right
	vertexArray[vertexOffset + 2] = Vertex3D_PCU(Vector3(backTopRight.x, backTopRight.y, frontBottomLeft.z),	tint, sideUVs.GetTopRight());			// Top Right
	vertexArray[vertexOffset + 3] = Vertex3D_PCU(Vector3(frontBottomLeft.x, backTopRight.y, frontBottomLeft.z), tint, sideUVs.GetTopLeft());			// Top Left

	//-------------------------------------Back face--------------------------------------------------------

	vertexArray[vertexOffset + 4]	= Vertex3D_PCU(Vector3(backTopRight.x, frontBottomLeft.y, backTopRight.z),		tint, sideUVs.GetBottomLeft());		// Bottom Left 
	vertexArray[vertexOffset + 5]	= Vertex3D_PCU(Vector3(frontBottomLeft.x, frontBottomLeft.y, backTopRight.z),	tint, sideUVs.GetBottomRight());	// Bottom Right
	vertexArray[vertexOffset + 6]	= Vertex3D_PCU(Vector3(frontBottomLeft.x, backTopRight.y, backTopRight.z),		tint, sideUVs.GetTopRight());		// Top Right
	vertexArray[vertexOffset + 7]	= Vertex3D_PCU(backTopRight,													tint, sideUVs.GetTopLeft());		// Top Left

	//-------------------------------------Left face--------------------------------------------------------

	vertexArray[vertexOffset + 8]	= Vertex3D_PCU(Vector3(frontBottomLeft.x, frontBottomLeft.y, backTopRight.z),	tint, sideUVs.GetBottomLeft());		// Bottom Left
	vertexArray[vertexOffset + 9]	= Vertex3D_PCU(frontBottomLeft,													tint, sideUVs.GetBottomRight());	// Bottom Right
	vertexArray[vertexOffset + 10]	= Vertex3D_PCU(Vector3(frontBottomLeft.x, backTopRight.y, frontBottomLeft.z),	tint, sideUVs.GetTopRight());		// Top Right
	vertexArray[vertexOffset + 11]	= Vertex3D_PCU(Vector3(frontBottomLeft.x, backTopRight.y, backTopRight.z),		tint, sideUVs.GetTopLeft());		// Top Left

	//-------------------------------------Right face--------------------------------------------------------

	vertexArray[vertexOffset + 12]	= Vertex3D_PCU(Vector3(backTopRight.x, frontBottomLeft.y, frontBottomLeft.z),	tint, sideUVs.GetBottomLeft());		// Bottom Left
	vertexArray[vertexOffset + 13]	= Vertex3D_PCU(Vector3(backTopRight.x, frontBottomLeft.y, backTopRight.z),		tint, sideUVs.GetBottomRight());	// Bottom Right
	vertexArray[vertexOffset + 14]	= Vertex3D_PCU(backTopRight,													tint, sideUVs.GetTopRight());		// Top Right
	vertexArray[vertexOffset + 15]	= Vertex3D_PCU(Vector3(backTopRight.x, backTopRight.y, frontBottomLeft.z),		tint, sideUVs.GetTopLeft());		// Top Left

	//-------------------------------------Top face--------------------------------------------------------

	vertexArray[vertexOffset + 16]	= Vertex3D_PCU(Vector3(frontBottomLeft.x, backTopRight.y, frontBottomLeft.z),	tint, topUVs.GetBottomLeft());		// Bottom Left
	vertexArray[vertexOffset + 17]	= Vertex3D_PCU(Vector3(backTopRight.x, backTopRight.y, frontBottomLeft.z),		tint, topUVs.GetBottomRight());		// Bottom Right
	vertexArray[vertexOffset + 18]	= Vertex3D_PCU(backTopRight,													tint, topUVs.GetTopRight());		// Top Right
	vertexArray[vertexOffset + 19]	= Vertex3D_PCU(Vector3(frontBottomLeft.x, backTopRight.y, backTopRight.z),		tint, topUVs.GetTopLeft());			// Top Left

	//-------------------------------------Bottom face--------------------------------------------------------

	vertexArray[vertexOffset + 20]	= Vertex3D_PCU(Vector3(frontBottomLeft.x, frontBottomLeft.y, backTopRight.z),	tint, bottomUVs.GetBottomLeft());	// Bottom Left
	vertexArray[vertexOffset + 21]	= Vertex3D_PCU(Vector3(backTopRight.x, frontBottomLeft.y, backTopRight.z),		tint, bottomUVs.GetBottomRight());	// Bottom Right
	vertexArray[vertexOffset + 22]	= Vertex3D_PCU(Vector3(backTopRight.x, frontBottomLeft.y, frontBottomLeft.z),	tint, bottomUVs.GetTopRight());		// Top Right
	vertexArray[vertexOffset + 23]	= Vertex3D_PCU(frontBottomLeft,													tint, bottomUVs.GetTopLeft());		// Top Left


	// Now add to the index buffer
	for (int sideIndex = 0; sideIndex < 6; sideIndex++)
	{
		int sideOffset = sideIndex * 6;

		// First triangle
		indexArray[sideOffset + indexOffset]		= vertexOffset + (sideIndex * 4);
		indexArray[sideOffset + indexOffset + 1]	= vertexOffset + (sideIndex * 4) + 1;
		indexArray[sideOffset + indexOffset + 2]	= vertexOffset + (sideIndex * 4) + 2;

		// Second triangle
		indexArray[sideOffset + indexOffset + 3]	= vertexOffset + (sideIndex * 4);
		indexArray[sideOffset + indexOffset + 4]	= vertexOffset + (sideIndex * 4) + 2;
		indexArray[sideOffset + indexOffset + 5]	= vertexOffset + (sideIndex * 4) + 3;
	}

	// Update the offsets to reflect the new end positions in the arrays
	vertexOffset += 24;
	indexOffset += 36;
}


//-----------------------------------------------------------------------------------------------
// Draws text to the screen as a textured AABB2
//
BitmapFont* Renderer::CreateOrGetBitmapFont(const char* bitmapFontName)
{
	if (m_loadedFonts.find(bitmapFontName) != m_loadedFonts.end())
	{
		return m_loadedFonts[bitmapFontName];
	}
	      
	Texture* fontTexture = CreateOrGetTexture(Stringf("%s%s.png", FONT_DIRECTORY, bitmapFontName));
	SpriteSheet* glyphSheet = new SpriteSheet(bitmapFontName, *fontTexture, FONT_SPRITE_LAYOUT);
	m_loadedFonts[bitmapFontName] = new BitmapFont(*glyphSheet, 1.0f);

	return m_loadedFonts[bitmapFontName];
}


//-----------------------------------------------------------------------------------------------
// Draws text to the screen as a textured AABB2
//
void Renderer::DrawText2D(const std::string& text, const Vector2& drawMins, float cellHeight, BitmapFont* font, Rgba color/*=Rgba::WHITE*/, float aspectScale/*=1.0f*/)
{	
	ASSERT_OR_DIE(font != nullptr, Stringf("Error -  Renderer::DrawText2D was passed a null font."));

	// Set up the text "mesh"
	Vertex3D_PCU* vertices = (Vertex3D_PCU*) malloc(sizeof(Vertex3D_PCU) * 4 * text.size());
	unsigned int* indices = (unsigned int*) malloc(sizeof(unsigned int) * 6 * text.size());

	int numVertices = 0;
	int numIndices = 0;

	// Break the text up by the new line characters
	std::vector<std::string> textLines = Tokenize(text, '\n');

	for (int lineNumber = 0; lineNumber < static_cast<int>(textLines.size()); lineNumber++)
	{
		Vector2 glyphBottomLeft = Vector2(drawMins.x, (drawMins.y - lineNumber * cellHeight));
		std::string currLine = textLines[lineNumber];

		// Append each letter of the current line to the text "mesh"
		for (int charIndex = 0; charIndex < static_cast<int>(currLine.length()); charIndex++)
		{
			char currentChar = currLine[charIndex];			
			float glyphWidth = (font->GetGlyphAspect() * cellHeight) * aspectScale;

			// Don't draw spaces!
			if (currentChar == ' ') 
			{ 
				glyphBottomLeft += Vector2(glyphWidth, 0.f);
				continue; 
			}

			Vector2 glyphTopRight = glyphBottomLeft + Vector2(glyphWidth, cellHeight);

			AABB2 drawBounds = AABB2(glyphBottomLeft, glyphTopRight);
			AABB2 glyphUVs = font->GetGlyphUVs(currentChar);
			AppendAABB2Vertices2D(vertices, numVertices, indices, numIndices, drawBounds, glyphUVs, color);

			// Increment the next bottom left position
			glyphBottomLeft += Vector2(glyphWidth, 0.f);
		}
	} 

	// Set the texture and draw
	BindTexture(0, font->m_spriteSheet.GetTexture().GetHandle());
	DrawMeshImmediate(vertices, numVertices, PRIMITIVE_TRIANGLES, indices, numIndices);

	// Free memory
	free(vertices);
	free(indices);
}


//-----------------------------------------------------------------------------------------------
// Draws the given text in the box using the alignment and draw mode settings
//
void Renderer::DrawTextInBox2D(const std::string& text, const AABB2& drawBox, const Vector2& alignment, float cellHeight, TextDrawMode drawMode, BitmapFont* font, Rgba color/*=Rgba::WHITE*/, float aspectScale/*=1.0f*/)
{
	switch (drawMode)
	{
	case TEXT_DRAW_SHRINK_TO_FIT:	{ DrawTextInBox2D_ShrinkToFit(text, drawBox, alignment, cellHeight, font, color, aspectScale); } break;
	case TEXT_DRAW_OVERRUN:			{ DrawTextInBox2D_Overrun(text, drawBox, alignment, cellHeight, font, color, aspectScale); } break;
	case TEXT_DRAW_WORD_WRAP:		{ DrawTextInBox2D_WordWrap(text, drawBox, alignment, cellHeight, font, color, aspectScale); } break;
	default:
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Creates the Built-in shader programs from the source code defined in ShaderProgram.hpp
// Should only be called once per renderer during PostStartup()
//
void Renderer::CreateBuiltInShaderPrograms()
{
	// Creating the invalid program - assumes it compiles correctly
	ShaderProgram* invalidProgram = new ShaderProgram();
	invalidProgram->LoadProgramFromSources(INVALID_VS, INVALID_FS);
	m_loadedShaderPrograms[ShaderProgram::INVALID_SHADER_NAME] = invalidProgram;

	// Default
	ShaderProgram* defaultProgram = new ShaderProgram();
	bool loadSuccessful = defaultProgram->LoadProgramFromSources(DEFAULT_VS, DEFAULT_FS);

	// If default failed then assign it the invalid shader in the map
	if (!loadSuccessful)
	{
		defaultProgram->LoadProgramFromSources(INVALID_VS, INVALID_FS);
	}

	m_loadedShaderPrograms[ShaderProgram::DEFAULT_SHADER_NAME] = defaultProgram;
}


//-----------------------------------------------------------------------------------------------
// Creates the textures that are part of the engine source
//
void Renderer::CreateBuiltInTextures()
{
	Texture* whiteTexture = new Texture();
	whiteTexture->CreateFromImage(&Image::IMAGE_WHITE);

	m_loadedTextures["White"] = whiteTexture;
}


//-----------------------------------------------------------------------------------------------
// Fetches the final back buffer state of the GPU and write to file
//
void Renderer::SaveScreenshotToFile()
{
	// Get and setup buffer info
	IntVector2 dimensions = m_defaultColorTarget->GetDimensions();
	int numTexels = dimensions.x * dimensions.y;
	char* buffer = (char*) malloc(sizeof(char) * numTexels * 4);	// 4 components, request the screenshot in RGBA format

	// Read the back buffer
	glReadPixels((GLint)0, (GLint)0, (GLint)dimensions.x, (GLint)dimensions.y, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	GL_CHECK_ERROR();

	//-----Now write the data to file-----
	
	// Check to see if the directory exists (will make it if it doesn't exist, do nothing otherwise)
	CreateDirectoryA("Data/Screenshots", NULL);

	// Format the screenshot name
	std::string localFilepath = Stringf("Data/Screenshots/%s.png", m_screenshotFilename.c_str());

	// Write the image to file (image will be upsidedown, so flip on write)
	stbi_flip_vertically_on_write(1);
	stbi_write_png(localFilepath.c_str(), dimensions.x, dimensions.y, 4, buffer, 0);
	
	// Reset flags and clean up
	m_saveScreenshotThisFrame = false;
	m_screenshotFilename.clear();
	free(buffer);
}


//-----------------------------------------------------------------------------------------------
// Returns the shader program given by the programPath if it is already loaded, or attempts to make
// a new shader program otherwise.
// All built-i
//
ShaderProgram* Renderer::CreateOrGetShaderProgram(const std::string& shaderName)
{
	if (m_loadedShaderPrograms.find(shaderName) != m_loadedShaderPrograms.end())
	{
		return m_loadedShaderPrograms[shaderName];
	}

	// Program not already loaded, so attempt to load and compile it
	ShaderProgram* newProgram = new ShaderProgram(); 
	std::string rootName = Stringf("%s%s", SHADER_DIRECTORY, shaderName.c_str());
	bool loadSuccessful = newProgram->LoadProgramFromFiles(rootName.c_str());

	// If the program could not be compiled or linked correctly, then assign it the invalid shader
	if (!loadSuccessful)
	{
		newProgram->LoadProgramFromSources(INVALID_VS, INVALID_FS);
	}

	// Store and return a reference to it
	m_loadedShaderPrograms[shaderName] = newProgram;
	return m_loadedShaderPrograms[shaderName];
}


//-----------------------------------------------------------------------------------------------
// Sets the current shader program to the one specified
//
void Renderer::SetCurrentShaderProgram(const ShaderProgram* program)
{
	if (program == nullptr)
	{
		program = m_defaultShaderProgram;
	}

	m_currentShaderProgram = program;
}


//-----------------------------------------------------------------------------------------------
// Sets the current shader program to the one specified by name
// Will attempt to load it from file if not found
//
void Renderer::SetCurrentShaderProgram(const std::string& programName)
{
	m_currentShaderProgram = CreateOrGetShaderProgram(programName);
}


//-----------------------------------------------------------------------------------------------
// Sets the default camera to the one passed
//
void Renderer::SetCurrentCamera(Camera* camera)
{
	// passing in nullptr resets the current camera to the default one
	if (camera == nullptr) {
		camera = m_defaultCamera; 
	}

	camera->Finalize(); // make sure the framebuffer is finished being setup; 
	m_currentCamera = camera;
}


//-----------------------------------------------------------------------------------------------
// Sets the projection matrix to an orthographic one with the given width and height, centered
// at (0,0)
//
void Renderer::SetProjectionOrtho(float width, float height, float nearZ, float farZ)
{
	m_currentCamera->SetProjection(Matrix44::MakeOrtho(-width / 2.f, width / 2.f, -height / 2.f, height / 2.f, nearZ, farZ));
}


//-----------------------------------------------------------------------------------------------
// Sets the projection matrix of the current camera to the one given
//
void Renderer::SetProjectionMatrix(const Matrix44& projectionMatrix)
{
	m_currentCamera->SetProjection(projectionMatrix);
}


//-----------------------------------------------------------------------------------------------
// Sets the view matrix of the current camera to the one given
//
void Renderer::SetViewMatrix(const Matrix44& viewMatrix)
{
	m_currentCamera->SetViewMatrix(viewMatrix);
}


//-----------------------------------------------------------------------------------------------
// Sets the default camera's camera and view matrix to look at the target from position with the given up
//
void Renderer::SetLookAt(const Vector3& position, const Vector3& target, const Vector3& up /*= Vector3::DIRECTION_UP*/)
{
	m_currentCamera->LookAt(position, target, up);
}


//-----------------------------------------------------------------------------------------------
// Sets the current model matrix on the renderer, for all DrawMeshImmediate calls
//
void Renderer::SetModelMatrix(const Matrix44& matrix)
{
	m_currentModelMatrix = matrix;
}


//-----------------------------------------------------------------------------------------------
// Binds the texture to the given slot
//
void Renderer::BindTexture(unsigned int bindSlot, unsigned int textureHandle)
{
	glActiveTexture(GL_TEXTURE0 + bindSlot);
	glBindTexture(GL_TEXTURE_2D, textureHandle);

	glBindSampler(bindSlot, m_defaultSampler->GetHandle());
}


//-----------------------------------------------------------------------------------------------
// Binds the texture given by the texturePath to the given slot
//
void Renderer::BindTexture(unsigned int bindSlot, const std::string& texturePath)
{
	Texture* texture = CreateOrGetTexture(texturePath);
	BindTexture(bindSlot, texture->GetHandle());
}

//-----------------------------------------------------------------------------------------------
// Binds a uniform to the current shader program
//
void Renderer::BindUniformFloat(const std::string& uniformName, float uniformValue) const
{
	GLint programHandle = m_currentShaderProgram->GetHandle();
	glUseProgram(programHandle);

	GLint location = glGetUniformLocation(programHandle, uniformName.c_str());
	if (location >= 0) 
	{
		glUniform1f(location, uniformValue);	// Acts on the currently bound program, hence it's after glUseProgram
	}
}


//-----------------------------------------------------------------------------------------------
// Enables the depth buffer on draw calls, and sets the compare operation to the one corresponding
// to compareMethod, and whether or not it should write to the buffer
//
void Renderer::EnableDepth(DepthCompare compareMethod, bool shouldWrite)
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(g_openGlDepthCompares[compareMethod]);

	glDepthMask(shouldWrite ? GL_TRUE : GL_FALSE);
}


//-----------------------------------------------------------------------------------------------
// Disables depth by disabling writing to the depth buffer, and writing always regardless of depth
//
void Renderer::DisableDepth()
{
	EnableDepth(COMPARE_ALWAYS, false);
}


//-----------------------------------------------------------------------------------------------
// Clears the depth buffer to the value specified
//
void Renderer::ClearDepth(float clearDepth /*= 1.0f*/)
{
	glClearDepthf(clearDepth);
	glClear(GL_DEPTH_BUFFER_BIT);
}


//-----------------------------------------------------------------------------------------------
// Reloads and compiles all shader programs from file that are already in the renderer's map
// of shader programs
//
void Renderer::ReloadShaders()
{
	std::map<std::string, ShaderProgram*>::iterator progItr = m_loadedShaderPrograms.begin();
	for(progItr; progItr != m_loadedShaderPrograms.end(); progItr++)
	{
		// Check to ensure that we don't attempt to load a built-in shader
		ShaderProgram* currProgram = progItr->second;
		if (currProgram->GetSourceFileName() == "")
		{
			continue;
		}

		// Attempt the reload
		bool reloadSuccessful = currProgram->LoadProgramFromFiles(currProgram->GetSourceFileName().c_str());

		// Again, if load was unsuccessful assign it the invalid shader
		if (!reloadSuccessful)
		{
			currProgram->LoadProgramFromSources(INVALID_VS, INVALID_FS);
		}
	}
}


//------------------------------------------------------------------------------------------
// Returns the default UI camera of the renderer
//
Camera* Renderer::GetUICamera() const
{
	return m_UICamera;
}


//-----------------------------------------------------------------------------------------------
// Returns the bounds of the UI ortho projection, based on the window aspect
//
AABB2 Renderer::GetUIBounds()
{
	return s_UIOrthoBounds;
}


//-----------------------------------------------------------------------------------------------
// Draws the given text in the box in overrun style
//
void Renderer::DrawTextInBox2D_Overrun(const std::string& text, const AABB2& drawBox, const Vector2& alignment, float cellHeight, BitmapFont* font, Rgba color/*=Rgba::WHITE*/, float aspectScale/*=1.0f*/)
{

	// Tokenize and get dimensions for padding calculation
	std::vector<std::string> textLines = Tokenize(text, '\n');
	Vector2 boxDimensions = drawBox.GetDimensions();

	// yPadding
	float totalHeight = cellHeight * static_cast<float>(textLines.size());
	float yPadding = boxDimensions.y - totalHeight;

	// Calculate xPadding per-line
	for (int lineNumber = 0; lineNumber < static_cast<int>(textLines.size()); lineNumber++)
	{
		float xPadding = (drawBox.maxs.x - drawBox.mins.x) - (font->GetStringWidth(textLines[lineNumber], cellHeight, aspectScale));

		// Set up draw position, compensating for the fact that DrawText2D works on a bottom-left is (0,0) coordinate system, here top-left is (0,0)
		Vector2 drawPosition;
		drawPosition.x = drawBox.mins.x + (xPadding * alignment.x);
		drawPosition.y = drawBox.maxs.y - (yPadding * alignment.y) - ((lineNumber + 1) * cellHeight);	// Decrease by cell height once for every line already drawn

		// Still draw one line at a time, since we need to recalculate x-alignment per line
		DrawText2D(textLines[lineNumber], drawPosition, cellHeight, font, color, aspectScale);
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the given text in the box in Shrink-to-fit style
//
void Renderer::DrawTextInBox2D_ShrinkToFit(const std::string& text, const AABB2& drawBox, const Vector2& alignment, float cellHeight, BitmapFont* font, Rgba color/*=Rgba::WHITE*/, float aspectScale/*=1.0f*/)
{
	std::vector<std::string> textLines = Tokenize(text, '\n');
	Vector2 boxDimensions = drawBox.GetDimensions();

	// Get the height
	float totalHeight = cellHeight * static_cast<float>(textLines.size());

	// Get the width - longest line for the smallest xScale
	int longestLineNumber = -1;
	float longestLineLength = -1;
	for (int lineNumber = 0; lineNumber < static_cast<int>(textLines.size()); lineNumber++)
	{
		float currentLineLength = font->GetStringWidth(textLines[lineNumber], cellHeight, aspectScale);

		if (longestLineNumber == -1 || currentLineLength > longestLineLength)
		{
			longestLineNumber = lineNumber;
			longestLineLength = currentLineLength;
		}
	}

	// Calculate the final scale - taking the minimum of the necessary x and y scales to fit in the box
	float xScale = boxDimensions.x / longestLineLength;
	float yScale = boxDimensions.y / totalHeight;

	// Clamp to 1.0 so we don't stretch/scale up
	float finalScale = (xScale > yScale ? yScale : xScale);
	if (finalScale > 1.0f)
	{
		finalScale = 1.0f;
	}

	// Adjust the cell height to fit and then draw
	cellHeight *= finalScale;

	// Draw using overrun, since it takes into consideration per line alignment, and we know we won't go outside box now
	DrawTextInBox2D_Overrun(text, drawBox, alignment, cellHeight, font, color, aspectScale);
}


//-----------------------------------------------------------------------------------------------
// Draws the given text in the box in word wrap style
//
void Renderer::DrawTextInBox2D_WordWrap(const std::string& text, const AABB2& drawBox, const Vector2& alignment, float cellHeight, BitmapFont* font, Rgba color/*=Rgba::WHITE*/, float aspectScale/*=1.0f*/)
{
	//-----State variables-----
	std::string wordWrappedText;						// The final result string
	Vector2 boxDimensions = drawBox.GetDimensions();	// The dimensions of the box
	std::string partialLine;							// The partially-constructed line, used to increment length until it no longer fits
	int numLines = 0;									// Number of lines in the wordWrappedText result

	std::vector<std::string> newLineSeparates = Tokenize(text, '\n');	// Text tokenized by new line characters


	// Iterate across all of the lines separated by '\n'
	for (int newLinesSeparatesIndex = 0; newLinesSeparatesIndex < static_cast<int>(newLineSeparates.size()); newLinesSeparatesIndex++)
	{
		std::string currLine = newLineSeparates[newLinesSeparatesIndex];		// The current line we're on
		std::vector<std::string> currLineTokens = Tokenize(currLine, ' ');		// The tokens in this line (separated by spaces)

		// Iterate across the tokens in the current line
		for (int currLineTokenIndex = 0; currLineTokenIndex < static_cast<int>(currLineTokens.size()); currLineTokenIndex++)
		{
			std::string currToken = currLineTokens[currLineTokenIndex];

			//-----Proposed string, copy used to see what the next appended length will be-----
			std::string proposedString = partialLine;
			if (proposedString.length() > 0)
			{
				proposedString.push_back(' ');
			}
			proposedString.append(currToken);
			float proposedLineLength = font->GetStringWidth(proposedString, cellHeight, aspectScale);
			
			// Length check, if we're out of the box
			if (proposedLineLength > boxDimensions.x)
			{
				// Adding spaces in between words only
				if (wordWrappedText.length() > 0 && wordWrappedText[wordWrappedText.length() - 1] != '\n')
				{
					wordWrappedText.push_back(' ');
				}

				// Edge case - the proposed string was a single token that was larger than the box, so append and continue
				if (partialLine.length() == 0)
				{
					wordWrappedText.append(proposedString);
				}
				// Else append everything up to the token that put us over, and put the token on the next partialLine
				else
				{	
					wordWrappedText.append(partialLine);
					partialLine.clear();
					partialLine.append(currToken);
				}

				// Increment number of lines
				wordWrappedText.push_back('\n');
				numLines++;
			}
			// Else the proposed line was still in the box, so append the current token to the partial line and continue
			else
			{
				// Add a space only if there are already words in partialLine
				if (partialLine.length() > 0)
				{
					partialLine.push_back(' ');
				}
				partialLine.append(currToken);
			}
		}

		// End of a '\n' separated line, so force a new line and continue to the next '\n' separated line
		wordWrappedText.append(partialLine);
		wordWrappedText.push_back('\n');
		partialLine.clear();
		numLines++;
	}

	// Done parsing, check for leftover lines and append appropriately
	if (partialLine.length() > 0)
	{
		if (wordWrappedText.length() > 0  && wordWrappedText[wordWrappedText.length() - 1] != '\n')
		{
			wordWrappedText.push_back(' ');
		}
		wordWrappedText.append(partialLine);
		numLines++;
	}

	// Check to see if we need to scale down the letters to fit
	float textHeight = static_cast<float>(numLines) * cellHeight;
	
	if (textHeight > boxDimensions.y)
	{
		float scale = boxDimensions.y / textHeight;
		cellHeight *= scale;
	}

	// Draw all the lines with the correct alignment
	DrawTextInBox2D_Overrun(wordWrappedText, drawBox, alignment, cellHeight, font, color, aspectScale);
}


//-----------------------------------------------------------------------------------------------
// Sets up the renderer's members that require GL functions
// Called only from the Renderer's constructor
//
void Renderer::PostGLStartup()
{
	// Load the built in shaders automatically
	CreateBuiltInShaderPrograms();

	// Load the built in textures automatically
	CreateBuiltInTextures();

	// Create and bind a default texture sampler
	m_defaultSampler = new Sampler();
	bool successful = m_defaultSampler->Initialize();
	GUARANTEE_OR_DIE(successful, Stringf("Error: Default Sampler could not be constructed successfully."));

	// the default color and depth should match our output window
	// so get width/height however you need to.
	unsigned int windowWidth	= Window::GetInstance()->GetWidthInPixels(); 
	unsigned int windowHeight	= Window::GetInstance()->GetHeightInPixels();

	// create our output textures
	m_defaultColorTarget = CreateRenderTarget(windowWidth, windowHeight);
	m_defaultDepthTarget = CreateDepthTarget(windowWidth, windowHeight); 

	// Bind a vertex array object - to be used later
	glGenVertexArrays(1, &m_defaultVAO); 
	glBindVertexArray(m_defaultVAO);

	// Enable blending
	EnableBlendMacro();
	SetBlendMode(BLEND_MODE_ALPHA);
}


//-----------------------------------------------------------------------------------------------
// Draws to the screen given the vertices and the draw primitive type
//
void Renderer::DrawMeshImmediate(const Vertex3D_PCU* vertexBuffer, int numVertices, PrimitiveType primitiveType /*= PRIMITIVE_TRIANGLES*/, const unsigned int* indexBuffer /*= nullptr*/, int numIndices /*= -1*/)
{
	// first, copy the memory to the gpu buffer
	m_vertexBuffer->CopyToGPU(sizeof(Vertex3D_PCU) * numVertices, vertexBuffer, GL_ARRAY_BUFFER); 

	if (indexBuffer != nullptr)
	{
		m_indexBuffer->CopyToGPU(sizeof(int) * numIndices, indexBuffer, GL_ELEMENT_ARRAY_BUFFER);
	}

	// Tell GL what shader program to use.
	GLuint program_handle = m_currentShaderProgram->GetHandle(); 

	//-----DESCRIBING VERTEX DATA TO GPU-----

	// Next, bind the buffer we want to use (buffer the renderer dumped vertex data into); 
	glBindBuffer( GL_ARRAY_BUFFER, m_vertexBuffer->GetHandle() ); 
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer->GetHandle());

	// Describe the buffer - first, figure out where the shader is expecting
	// position and uvs to be.
	GLint pos_bind = glGetAttribLocation(program_handle, "POSITION");
	
	// next, bind where position is in our buffer to that location; 
	if (pos_bind >= 0) 
	{
		// enable this location
		glEnableVertexAttribArray(pos_bind);

		// describe the data for position...
		glVertexAttribPointer(pos_bind,						// where?
			3,												// how many (vec3 has 3 floats)
			GL_FLOAT,										// type? (vec3 is 3 floats)
			GL_FALSE,										// Should data be normalized
			sizeof(Vertex3D_PCU),							// stride (how far between each vertex)
			(GLvoid*)offsetof(Vertex3D_PCU, m_position));	// From the start of a vertex, where is this data?
	}

	// Do the same for Color
	GLint color_bind = glGetAttribLocation(program_handle, "COLOR");
	if (color_bind >= 0) {
		// enable this location
		glEnableVertexAttribArray(color_bind);

		// describe the data
		glVertexAttribPointer(color_bind, // where?
			4,                           // how many (RGBA is 4 unsigned chars)
			GL_UNSIGNED_BYTE,            // type? (RGBA is 4 unsigned chars)
			GL_TRUE,                     // Normalize components, maps 0-255 to 0-1.
			sizeof(Vertex3D_PCU),             // stride (how far between each vertex)
			(GLvoid*)offsetof(Vertex3D_PCU, m_color)); // From the start of a vertex, where is this data?
	}

	// Do the same for UV's
	GLint uv_bind = glGetAttribLocation(program_handle, "UV");
	if (uv_bind >= 0)
	{
		glEnableVertexAttribArray(uv_bind);
		glVertexAttribPointer(uv_bind,						
			2,												
			GL_FLOAT,										
			GL_FALSE,										
			sizeof(Vertex3D_PCU),							
			(GLvoid*)offsetof(Vertex3D_PCU, m_texUVs));	
	}

	//----------END VERTEX DATA----------


	// Binding uniforms
	glUseProgram(program_handle); 

	GLint projLocation = glGetUniformLocation(program_handle, "PROJECTION");
	if (projLocation >= 0) 
	{
		Matrix44 projectionMatrix = m_currentCamera->GetProjectionMatrix();
		glUniformMatrix4fv(projLocation, 1, GL_FALSE, (GLfloat*)&projectionMatrix);	// Acts on the currently bound program, hence it's after glUseProgram
	}

	GLint viewLocation = glGetUniformLocation(program_handle, "VIEW");
	if (viewLocation >= 0) 
	{
		Matrix44 viewMatrix = m_currentCamera->GetViewMatrix();
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, (GLfloat*)&viewMatrix);	// Acts on the currently bound program, hence it's after glUseProgram
	}

	GLint modelLocation = glGetUniformLocation(program_handle, "MODEL");
	if (modelLocation >= 0) 
	{
		Matrix44 modelMatrix = m_currentModelMatrix;
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, (GLfloat*)&modelMatrix);	// Acts on the currently bound program, hence it's after glUseProgram
	}

	// Bind the frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_currentCamera->GetFrameBufferHandle()); 

	if (indexBuffer != nullptr)
	{
		// Draw with indices
		glDrawElements(g_openGlPrimitiveTypes[primitiveType], numIndices, GL_UNSIGNED_INT, 0);
	}
	else
	{
		// Draw without indices
		glDrawArrays(g_openGlPrimitiveTypes[primitiveType], 0, numVertices);
	}
}


//-----------------------------------------------------------------------------------------------
// Draws a line from startPos to endPos with the given colors
//
void Renderer::DrawLine(const Vector3& startPos, const Rgba& startColor, const Vector3& endPos, const Rgba& endColor, float width)
{
	UNUSED(startPos);
	UNUSED(endPos);
	UNUSED(startColor);
	UNUSED(endColor);
	UNUSED(width);

	UNIMPLEMENTED();
}


//-----------------------------------------------------------------------------------------------
// Copies the FrameBuffer data from source to destination
//
bool Renderer::CopyFrameBuffer( FrameBuffer *destination, FrameBuffer *source )
{
	// we need at least the src.
	if (source == nullptr) 
	{
		return false; 
	}

	// Get the handles - NULL refers to the "default" or back buffer FBO
	GLuint sourceFBO = source->GetHandle();
	GLuint destinationDBO = NULL; 
	if (destination != nullptr) 
	{
		destinationDBO = destination->GetHandle(); 
	}

	// can't copy onto ourselves
	if (destinationDBO == sourceFBO) 
	{
		return false; 
	}

	// the GL_READ_FRAMEBUFFER is where we copy from
	glBindFramebuffer( GL_READ_FRAMEBUFFER, sourceFBO ); 

	// what are we copying to?
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, destinationDBO ); 

	// blit it over - get the size
	// (we'll assume dst matches for now - but to be safe,
	// you should get dst_width and dst_height using either
	// dst or the window depending if dst was nullptr or not
	unsigned int width = source->GetWidth();     
	unsigned int height = source->GetHeight(); 

	// Copy it over
	glBlitFramebuffer( 0, 0,  // src start pixel
		width, height,        // src size
		0, 0,                 // dst start pixel
		width, height,        // dst size
		GL_COLOR_BUFFER_BIT,  // what are we copying (just colour)
		GL_NEAREST );         // resize filtering rule (in case src/dst don't match)

	// Make sure it succeeded
	GL_CHECK_ERROR(); 

	// Cleanup after ourselves
	glBindFramebuffer( GL_READ_FRAMEBUFFER, NULL ); 
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, NULL ); 

	return GLSucceeded();
}


//---------------------Console commands----------------------------

void Command_Screenshot(Command& cmd)
{
	std::string filename = cmd.GetNextString();
	
	if (filename.size() == 0)
	{
		filename = "screenshot.png";
	}

	Renderer::GetInstance()->SaveScreenshotAtEndOfFrame(filename);
}