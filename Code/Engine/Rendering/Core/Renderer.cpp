/************************************************************************/
/* File: Renderer.cpp
/* Author: Andrew Chase
/* Date: September January 25th, 2017
/* Bugs: None
/* Description: Implementation of the Renderer class
/************************************************************************/
#include "Engine/Core/Time/Time.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Resources/Texture.hpp"
#include "Engine/Rendering/OpenGL/glFunctions.hpp"
#include "Engine/Rendering/Shaders/Shader.hpp"
#include "Engine/Rendering/Shaders/ShaderProgram.hpp"
#include "Engine/Rendering/Buffers/RenderBuffer.hpp"
#include "Engine/Rendering/Resources/Sampler.hpp"
#include "Engine/Rendering/Core/Camera.hpp"
#include "Engine/Rendering/Resources/Sprite.hpp"
#include "Engine/Core/DeveloperConsole/Command.hpp"
#include "Engine/Core/File.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Rendering/Materials/Material.hpp"
#include "Engine/Rendering/Materials/MaterialPropertyBlock.hpp"
#include "Engine/Rendering/Shaders/PropertyBlockDescription.hpp"
#include "Engine/Rendering/Core/Light.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ThirdParty/gl/glcorearb.h"
#include "ThirdParty/stb/stb_image.h"
#include "ThirdParty/stb/stb_image_write.h"

Renderer*			Renderer::s_instance = nullptr;

const IntVector2	Renderer::FONT_SPRITE_LAYOUT	= IntVector2(16, 16);
const char*			Renderer::FONT_DIRECTORY		= "Data\\Fonts\\";
const char*			Renderer::SHADER_DIRECTORY		= "Data\\Shaders\\";
const float			Renderer::UI_ORTHO_HEIGHT		= 1080.f;
AABB2				Renderer::s_UIOrthoBounds;


//********************Structs for Uniform Buffer Data********************

// Time
struct TimeBufferData
{
	TimeBufferData()
		: m_gameDeltaTime(0.f), m_gameTotalTime(0.f)
		, m_systemDeltaTime(0.f), m_systemTotalTime(0.f) {}

	float m_gameDeltaTime;
	float m_gameTotalTime;
	float m_systemDeltaTime;
	float m_systemTotalTime;
};

// Buffer for light data for all lights
struct LightBufferData
{
	Vector4 m_ambience;		// xyz color, w intensity
	LightData m_lights[MAX_NUMBER_OF_LIGHTS];
};


// Specular data, per object
struct SpecularBufferData
{
	float m_specularAmount;
	float m_specularPower;
	Vector2	m_padding0;
};


//-----------------------------------------------------------------------------------------------
// Constructor - the loaded textures map is already initialized by default to an empty map
// Sets the singleton instance to this renderer if none is created yet
//
Renderer::Renderer()
{
	// Ensure only one renderer exists
	GUARANTEE_OR_DIE(s_instance == nullptr, "Error: Renderer constructor called when a renderer already exists.");

	// Ensure a context already exists before creating this renderer (so gl functions handles are created and bound)
	GUARANTEE_OR_DIE(gHDC != nullptr, "Error: Renderer constructed without a gl context established first.");

	s_instance = this;

	// Calls all GL functions necessary to set up the renderer
	PostGLStartup();

	// Initializing member on Renderer - *****No GL functions are called in the lines below outside setcurrentcamera*****

	// setup the initial camera
	m_defaultCamera = new Camera();
	m_defaultCamera->SetColorTarget(m_defaultColorTarget); 
	m_defaultCamera->SetDepthTarget(m_defaultDepthTarget); 

	// Make the UI camera
	m_UICamera = new Camera();
	m_UICamera->SetColorTarget(m_defaultColorTarget);
	m_UICamera->SetDepthTarget(m_defaultDepthTarget);
	m_UICamera->SetProjection(Matrix44::MakeOrtho(Vector2::ZERO, Vector2(Window::GetInstance()->GetAspect() * UI_ORTHO_HEIGHT, UI_ORTHO_HEIGHT)));

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
	// Delete cameras
	delete m_defaultCamera;
	delete m_UICamera;
	delete m_effectsCamera;

	// Free the vao 
	glDeleteVertexArrays(1, &m_defaultVAO);
	GL_CHECK_ERROR();
}


//-----------------------------------------------------------------------------------------------
// Constructs the singleton Renderer instance
//
void Renderer::Initialize()
{
	GUARANTEE_OR_DIE(s_instance == nullptr, "Error: Renderer::Initialize() called when the Renderer instance exists.");
	new Renderer();

	// Static setup
	s_UIOrthoBounds = AABB2(Vector2::ZERO, Vector2(UI_ORTHO_HEIGHT * Window::GetInstance()->GetAspect(), UI_ORTHO_HEIGHT));
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
	// Leftover errors from the last frame?
	GL_CHECK_ERROR();

	// Set the default shader program to the current program reference
	SetCurrentCamera(nullptr);
	ClearScreen(Rgba(0,0,0,0));
	ClearDepth();

	// Update the time uniform buffer on the gpu
	UpdateTimeData();

	// Clear the lights, making the game reset them
	DisableAllLights();
}


//-----------------------------------------------------------------------------------------------
// Performs all Renderer procedures that occur at the end of the frame
//
void Renderer::EndFrame()
{
	// Copy the default frame buffer to the back buffer before swapping
	m_defaultCamera->FinalizeFrameBuffer();
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
void Renderer::ApplyImageEffect(ShaderProgram* program)
{
	UNUSED(program)
	UNIMPLEMENTED();
// 	DisableDepth();
// 
// 	// Set up the effects targets if they aren't already set up
// 	if (m_effectsSource == nullptr)
// 	{
// 		m_effectsSource = m_defaultColorTarget;
// 		if (m_effectsDestination == nullptr)
// 		{
// 			m_effectsDestination = new Texture();
// 			m_effectsDestination->CreateRenderTarget(m_effectsSource->GetDimensions().x, m_effectsSource->GetDimensions().y, TEXTURE_FORMAT_RGBA8);
// 		}
// 	}
// 
// 	// Draw using the effects camera - to the scratch target
// 	m_effectsCamera->SetColorTarget(m_effectsDestination);
// 	SetCurrentCamera(m_effectsCamera);
// 
// 	// Set the shader program to the one provided
// 	Shader* effectShader = new Shader(program);
// 	SetCurrentShader(effectShader);
// 
// 	// Bind the current source as a texture
// 	BindTexture(0, m_effectsSource->GetHandle());
// 	
// 	// Draw the previous buffer as an AABB2 across the entire new render target
// 	Draw2DQuad(AABB2::UNIT_SQUARE_CENTERED, AABB2::UNIT_SQUARE_OFFCENTER, Rgba::WHITE);
// 
// 	// Swap the pointers around for the next effect
// 	Texture* temp			= m_effectsSource;
// 	m_effectsSource			= m_effectsDestination;
// 	m_effectsDestination	= temp;
}


//-----------------------------------------------------------------------------------------------
// Cleans up the effect state after image effects have been applied
//
void Renderer::FinalizeImageEffects()
{
	UNIMPLEMENTED();
// 	// Null target means no effects have been applied, so nothing to finalize
// 	if (m_effectsSource == nullptr)
// 	{
// 		return;
// 	}
// 
// 	// An odd number of effects were applied, so ensure the default color target is the final result
// 	if (m_effectsSource != m_defaultColorTarget)
// 	{
// 		Texture::CopyTexture(m_effectsSource, m_defaultColorTarget);
// 		m_effectsDestination = m_effectsSource;
// 	}
// 
// 	// Signal we're done with our current effects processing
// 	m_effectsSource = nullptr;
// 
// 	SetCurrentCamera(nullptr);
// 	EnableDepth(DEPTH_TEST_LESS, true);
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
// Returns the default camera of the renderer
//
Camera* Renderer::GetDefaultCamera() const
{
	return m_defaultCamera;
}


//-----------------------------------------------------------------------------------------------
// Sets the flag to take a screenshot during the next Renderer::EndFrame()
//
void Renderer::SaveScreenshotAtEndOfFrame(const std::string& filename)
{
	TODO("Named screenshots, screenshot with console command");
	UNUSED(filename);
	m_saveScreenshotThisFrame = true;
}


//-----------------------------------------------------------------------------------------------
// Draws a textured AABB2 from the provided texture data
// textureUVs are the texture coordinates of the bottom left and top right of the box
//
void Renderer::Draw2DQuad(const AABB2& bounds, const AABB2& textureUVs, const Rgba& tint, Material* material /*= nullptr*/)
{
	// Clear state
	m_immediateBuilder.Clear();

	// Build the mesh
	m_immediateBuilder.BeginBuilding(PRIMITIVE_TRIANGLES, true);
	m_immediateBuilder.Push2DQuad(bounds, textureUVs, tint);
	m_immediateBuilder.FinishBuilding();
	m_immediateBuilder.UpdateMesh(m_immediateMesh);

	// Draw
	if (material == nullptr)
	{
		DrawMesh(&m_immediateMesh);
	}
	else
	{
		DrawMeshWithMaterial(&m_immediateMesh, material);
	}
}


//-----------------------------------------------------------------------------------------------
// Draws an oriented AABB2 in 3D space given the parameters
//
void Renderer::Draw3DQuad(const Vector3& position, const Vector2& dimensions, const AABB2& textureUVs, const Vector3& right /*= Vector3::DIRECTION_RIGHT*/, const Vector3& up /*= Vector3::DIRECTION_UP*/, const Rgba& tint /*= Rgba::WHITE*/, const Vector2& pivot /*= Vector2(0.5f, 0.5f)*/, Material* material /*= nullptr*/)
{
	// Clear state
	m_immediateBuilder.Clear();

	// Build the mesh
	m_immediateBuilder.BeginBuilding(PRIMITIVE_TRIANGLES, true);
	m_immediateBuilder.Push3DQuad(position, dimensions, textureUVs, tint, right, up, pivot);
	m_immediateBuilder.FinishBuilding();
	m_immediateBuilder.UpdateMesh(m_immediateMesh);

	// Draw
	if (material == nullptr)
	{
		DrawMesh(&m_immediateMesh);
	}
	else
	{
		DrawMeshWithMaterial(&m_immediateMesh, material);
	}
}


//-----------------------------------------------------------------------------------------------
// Draws a cube with the given corner positions and tint
//
void Renderer::DrawCube(const Vector3& center, const Vector3& dimensions, const Rgba& tint /*= Rgba::WHITE*/, const AABB2& topUVs /*= AABB2::UNIT_SQUARE_OFFCENTER*/, const AABB2& sideUVs /*= AABB2::UNIT_SQUARE_OFFCENTER*/, const AABB2& bottomUVs /*= AABB2::UNIT_SQUARE_OFFCENTER*/, Material* material /*= nulltpr*/)
{
	// Clear state
	m_immediateBuilder.Clear();

	// Build the mesh
	m_immediateBuilder.BeginBuilding(PRIMITIVE_TRIANGLES, true);
	m_immediateBuilder.PushCube(center, dimensions, tint, sideUVs, topUVs, bottomUVs);
	m_immediateBuilder.FinishBuilding();
	m_immediateBuilder.UpdateMesh(m_immediateMesh);

	// Draw
	if (material == nullptr)
	{
		DrawMesh(&m_immediateMesh);
	}
	else
	{
		DrawMeshWithMaterial(&m_immediateMesh, material);
	}
}


//-----------------------------------------------------------------------------------------------
// Draws a sphere with the given parameters
//
void Renderer::DrawSphere(const Vector3& position, float radius, unsigned int numWedges, unsigned int numSlices, const Rgba& color /*= Rgba::WHITE*/, Material* material /*= nulltpr*/)
{
	// Clear state
	m_immediateBuilder.Clear();

	// Build the mesh
	m_immediateBuilder.BeginBuilding(PRIMITIVE_TRIANGLES, true);
	m_immediateBuilder.PushUVSphere(position, radius, numWedges, numSlices, color);
	m_immediateBuilder.FinishBuilding();
	m_immediateBuilder.UpdateMesh(m_immediateMesh);

	// Draw
	if (material == nullptr)
	{
		DrawMesh(&m_immediateMesh);
	}
	else
	{
		DrawMeshWithMaterial(&m_immediateMesh, material);
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the given sprite at the position with tint
//
void Renderer::DrawSprite(const Sprite* sprite, const Vector3& position, const Rgba& tint /*= Rgba::WHITE*/, const Vector3& right /*= Vector3::DIRECTION_RIGHT*/, const Vector3& up /*= Vector3::DIRECTION_UP*/)
{
	UNUSED(sprite)
	UNUSED(position)
	UNUSED(tint)
	UNUSED(right)
	UNUSED(up)

	UNIMPLEMENTED();
// 	BindTexture(0, sprite->GetTexture().GetHandle());
// 	Vector2 spriteDimensions	= sprite->GetDimensions();
// 	Vector2 spritePivot			= sprite->GetPivot();
// 	AABB2 spriteUVs				= sprite->GetUVs();
// 
// 	// Find the min and max X values for the sprite AABB2 draw bounds
// 	float minX = -1.0f * (spritePivot.x * spriteDimensions.x);
// 	float maxX = minX + spriteDimensions.x;
// 
// 	// Find the min and max X values for the sprite AABB2 draw bounds
// 	float minY = -1.0f * (spritePivot.y * spriteDimensions.y);
// 	float maxY = minY + spriteDimensions.y;
// 
// 	Vector3 bottomLeft 		= position + minX * right + minY * up;
// 	Vector3 bottomRight 	= position + maxX * right + minY * up;
// 	Vector3 topLeft 		= position + minX * right + maxY * up;
// 	Vector3 topRight 		= position + maxX * right + maxY * up;
// 
// 	// Make the Vertex3D_PCU's (no indices)
// 
// 	Vertex3D_PCU vertices[6];
// 
// 	// First triangle
// 	vertices[0] = Vertex3D_PCU(bottomLeft, tint, spriteUVs.GetBottomLeft());
// 	vertices[1] = Vertex3D_PCU(bottomRight, tint, spriteUVs.GetBottomRight());
// 	vertices[2] = Vertex3D_PCU(topRight, tint, spriteUVs.GetTopRight());
// 	
// 	// Second triangle
// 	vertices[3] = vertices[0];
// 	vertices[4] = vertices[2];
// 	vertices[5] = Vertex3D_PCU(topLeft, tint, spriteUVs.GetTopLeft());
// 
// 	// Draw the sprite
// 	DrawMeshImmediate(vertices, 6, PRIMITIVE_TRIANGLES);
}


//-----------------------------------------------------------------------------------------------
// Draws text to the screen as a textured AABB2
//
void Renderer::DrawText2D(const std::string& text, const Vector2& drawMins, float cellHeight, BitmapFont* font, Rgba color/*=Rgba::WHITE*/, float aspectScale/*=1.0f*/)
{	
	ASSERT_OR_DIE(font != nullptr, Stringf("Error -  Renderer::DrawText2D was passed a null font."));

	// Check if there's anything to draw, if not then return
	if (text.size() == 0 || text.find_first_not_of(' ') == std::string::npos)
	{
		return;
	}

	// Build a mesh for the text
	m_immediateBuilder.Clear();
	m_immediateBuilder.BeginBuilding(PRIMITIVE_TRIANGLES, true);

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
			m_immediateBuilder.Push2DQuad(drawBounds, glyphUVs, color);

			// Increment the next bottom left position
			glyphBottomLeft += Vector2(glyphWidth, 0.f);
		}
	} 

	// Construct the Mesh
	m_immediateBuilder.FinishBuilding();
	m_immediateBuilder.UpdateMesh(m_immediateMesh);

	// Set the texture and draw
	Material fontMat = Material();
	fontMat.SetDiffuse(&font->GetSpriteSheet().GetTexture());
	fontMat.SetShader(AssetDB::CreateOrGetShader("UI"));

	DrawMeshWithMaterial(&m_immediateMesh, &fontMat);
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

	// Save a temp one to know what the latest screenshot was
	std::string tempName = "Data/Screenshots/Screenshot.png";

	// Write the image to file (image will be upsidedown, so flip on write)
	stbi_flip_vertically_on_write(1);
	stbi_write_png(tempName.c_str(), dimensions.x, dimensions.y, 4, buffer, 0);
	
	// Write with date and time to archive
	std::string archivedName = Stringf("Data/Screenshots/Screenshot_%s.png", GetSystemDateAndTime().c_str());
	stbi_write_png(archivedName.c_str(), dimensions.x, dimensions.y, 4, buffer, 0);

	// Reset flags and clean up
	m_saveScreenshotThisFrame = false;
	free(buffer);
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

	camera->FinalizeFrameBuffer(); // make sure the framebuffer is finished being setup; 

	// Update the uniform block for the camera
	camera->FinalizeUniformBuffer();

	// Need to update the binding, since each camera may point to a different GPU buffer
	BindUniformBuffer(CAMERA_BUFFER_BINDING, camera->GetUniformBufferHandle());

	m_currentCamera = camera;
}


//-----------------------------------------------------------------------------------------------
// Sets the game clock on the Renderer to the one specified
//
void Renderer::SetRendererGameClock(Clock* gameClock)
{
	m_gameClock = gameClock;
}


//-----------------------------------------------------------------------------------------------
// Adjust the intensity of the ambient 
//
void Renderer::AdjustAmbientIntensity(float deltaAmount)
{
	LightBufferData* buffer = m_lightUniformBuffer.GetCPUBufferAsType<LightBufferData>();
	float currAmbience = buffer->m_ambience.w;
	buffer->m_ambience.w = ClampFloatZeroToOne(currAmbience + deltaAmount);
}


//-----------------------------------------------------------------------------------------------
// Sets the ambient light intensity for the scene to render
//
void Renderer::SetAmbientIntensity(float newIntensity)
{
	LightBufferData* buffer = m_lightUniformBuffer.GetCPUBufferAsType<LightBufferData>();
	buffer->m_ambience.w = ClampFloatZeroToOne(newIntensity);
}


//-----------------------------------------------------------------------------------------------
// Sets the ambient light for the scene to render
//
void Renderer::SetAmbientLight(const Rgba& color)
{
	LightBufferData* buffer = m_lightUniformBuffer.GetCPUBufferAsType<LightBufferData>();
	color.GetAsFloats(buffer->m_ambience.x, buffer->m_ambience.y, buffer->m_ambience.z, buffer->m_ambience.w);
}


//-----------------------------------------------------------------------------------------------
// Sets the ambient light for the scene to render
//
void Renderer::SetAmbientLight(const Vector4& color)
{
	LightBufferData* buffer = m_lightUniformBuffer.GetCPUBufferAsType<LightBufferData>();
	buffer->m_ambience = color;
}


//-----------------------------------------------------------------------------------------------
// Enables a single point light at the given index
//
void Renderer::EnablePointLight(unsigned int index, const Vector3& position, const Rgba& color /*= Rgba::WHITE*/, const Vector3& attenuation /*= Vector3(1.f, 0.f, 0.f)*/)
{
	LightBufferData* buffer = m_lightUniformBuffer.GetCPUBufferAsType<LightBufferData>();
	LightData& currLight = buffer->m_lights[index];

	currLight.m_position = position;

	float red, green, blue, intensity;
	color.GetAsFloats(red, green, blue, intensity);
	currLight.m_color = Vector4(red, green, blue, intensity);

	currLight.m_attenuation = attenuation;

	// Don't need dots since this isn't a spot light, and directional factor = 1.f indicates this is a point light
	currLight.m_dotOuterAngle = -2.f;
	currLight.m_dotInnerAngle = -1.f;
	currLight.m_directionFactor = 1.0f;

	// Light direction is unused, as point lights emit light in all directions
	currLight.m_lightDirection = Vector3::ZERO;
}


//-----------------------------------------------------------------------------------------------
// Enables a single directional light at the given index
//
void Renderer::EnableDirectionalLight(unsigned int index, const Vector3& position, const Vector3& direction /*= Vector3::DIRECTION_DOWN*/, const Rgba& color /*= Rgba::WHITE*/, const Vector3& attenuation /*= Vector3(1.f, 0.f, 0.f)*/)
{
	LightBufferData* buffer = m_lightUniformBuffer.GetCPUBufferAsType<LightBufferData>();
	LightData& currLight = buffer->m_lights[index];

	currLight.m_position = position;
	currLight.m_lightDirection = direction;

	float red, green, blue, intensity;
	color.GetAsFloats(red, green, blue, intensity);
	currLight.m_color = Vector4(red, green, blue, intensity);

	currLight.m_attenuation = attenuation;

	// Don't need dots since this isn't a spot light, and directional factor = 0.f indicates this is a directional light
	currLight.m_dotOuterAngle = -2.0f;
	currLight.m_dotInnerAngle = -1.f;
	currLight.m_directionFactor = 0.f;
}


//-----------------------------------------------------------------------------------------------
// Enables a single directional light at the given index
//
void Renderer::EnableSpotLight(unsigned int index, const Vector3& position, const Vector3& direction, float outerAngle, float innerAngle, const Rgba& color /*= Rgba::WHITE*/, const Vector3& attenuation /*= Vector3(1.f, 0.f, 0.f)*/)
{
	LightBufferData* buffer = m_lightUniformBuffer.GetCPUBufferAsType<LightBufferData>();
	LightData& currLight = buffer->m_lights[index];

	currLight.m_position = position;
	currLight.m_lightDirection = direction;

	float red, green, blue, intensity;
	color.GetAsFloats(red, green, blue, intensity);
	currLight.m_color = Vector4(red, green, blue, intensity);

	currLight.m_attenuation = attenuation;

	// Don't need dots since this isn't a spot light, and directional factor = 1.f indicates this is a point light
	currLight.m_dotOuterAngle = CosDegrees(outerAngle * 0.5f);
	currLight.m_dotInnerAngle = CosDegrees(innerAngle * 0.5f);
	currLight.m_directionFactor = 1.0f;
}


//-----------------------------------------------------------------------------------------------
// Enables the lights specified in the given draw call
//
void Renderer::EnableLightsForDrawCall(const DrawCall* drawCall)
{
	int numLights = drawCall->GetNumLights();
	LightBufferData* buffer = m_lightUniformBuffer.GetCPUBufferAsType<LightBufferData>();

	for (int lightIndex = 0; lightIndex < MAX_NUMBER_OF_LIGHTS; ++lightIndex)
	{
		LightData& currLight = buffer->m_lights[lightIndex];

		// Disable all extra lights
		if (lightIndex >= numLights)
		{
			currLight.m_color.w = 0.f;
			currLight.m_attenuation = Vector3(0.f, 0.f, 1.f);
		}
		else
		{
			currLight = drawCall->GetLight(lightIndex)->GetLightData();

			// Also check for shadowMaps
			if (drawCall->GetLight(lightIndex)->IsShadowCasting())
			{
				BindTexture(SHADOW_TEXTURE_BINDING, drawCall->GetLight(lightIndex)->GetShadowTexture(), m_shadowSampler);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the intensity of all lights to 0, effectively disabling them
//
void Renderer::DisableAllLights()
{
	LightBufferData* buffer = m_lightUniformBuffer.GetCPUBufferAsType<LightBufferData>();

	for (int lightIndex = 0; lightIndex < MAX_NUMBER_OF_LIGHTS; ++lightIndex)
	{
		LightData& currLight = buffer->m_lights[lightIndex];
		currLight.m_color.w = 0.f;
		currLight.m_attenuation = Vector3(0.f, 0.f, 1.f);
	}
}


//-----------------------------------------------------------------------------------------------
// Binds the texture given by the texturePath to the given slot
//
void Renderer::BindTexture(unsigned int bindSlot, const std::string& filename)
{
	Texture* texture = AssetDB::CreateOrGetTexture(filename.c_str());
	BindTexture(bindSlot, texture);
}


//-----------------------------------------------------------------------------------------------
// Binds the texture and sampler together to the given slot
//
void Renderer::BindTexture(unsigned int bindSlot, const Texture* texture, const Sampler* sampler /*= nullptr*/)
{
	glActiveTexture(GL_TEXTURE0 + bindSlot);

	// Get the texture target type
	TextureType type = texture->GetTextureType();
	GLenum glType = ToGLType(type);

	glBindTexture(glType, texture->GetHandle());

	// nullptr defaults the sampler to the default one on the renderer
	if (sampler == nullptr)
	{
		sampler = m_defaultSampler;
	}

	glBindSampler(bindSlot, sampler->GetHandle());
}


//-----------------------------------------------------------------------------------------------
// Binds the material state to the renderer
//
void Renderer::BindMaterial(Material* material)
{
	glUseProgram(material->GetShader()->GetProgram()->GetHandle());

	// Bind all the textures/samplers
	for (int textureIndex = 0; textureIndex < MAX_TEXTURES_SAMPLERS; ++textureIndex)
	{
		const Texture* texture = material->GetTexture(textureIndex);

		if (texture != nullptr)
		{
			const Sampler* sampler = material->GetSampler(textureIndex);
			BindTexture(textureIndex, texture, sampler);
		}
	}

	// Bind the uniform property blocks
	int numBlocks = material->GetPropertyBlockCount();

	for (int blockIndex = 0; blockIndex < numBlocks; ++blockIndex)
	{
		MaterialPropertyBlock* block = material->GetPropertyBlock(blockIndex);

		// Since we're in a draw, ensure the GPU data is up-to-date
		block->CheckAndUpdateGPUData();

		unsigned int binding = block->GetDescription()->GetBlockBinding();
		BindUniformBuffer(binding, block->GetHandle());
	}
}


//-----------------------------------------------------------------------------------------------
// Binds a uniform buffer to the current shader program at the given slot
//
void Renderer::BindUniformBuffer(unsigned int bindSlot, unsigned int bufferHandle) const
{
	glBindBufferBase(GL_UNIFORM_BUFFER, bindSlot, bufferHandle);
	GL_CHECK_ERROR();
}


//-----------------------------------------------------------------------------------------------
// Binds a mesh's vertex layout of attributes to the specified program
//
void Renderer::BindMeshToProgram(const ShaderProgram* program, const Mesh* mesh) const
{
	glUseProgram(program->GetHandle());
	GL_CHECK_ERROR();

	// First bind the mesh information, vertices and indices
	glBindBuffer(GL_ARRAY_BUFFER, mesh->GetVertexBuffer()->GetHandle());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetIndexBuffer()->GetHandle());
	GL_CHECK_ERROR();

	const VertexLayout* vertexLayout = mesh->GetVertexLayout();
	unsigned int vertexStride = vertexLayout->GetStride();

	// Passing the data to the program
	GLuint programHandle = program->GetHandle();
	unsigned int numAttributes = vertexLayout->GetAttributeCount();

	for (unsigned int attribIndex = 0; attribIndex < numAttributes; ++attribIndex)
	{
		const VertexAttribute& attribute = vertexLayout->GetAttribute(attribIndex);

		// Try to find the attribute on the shader by its name
		int bind = glGetAttribLocation(programHandle, attribute.m_name.c_str());

		// If the attribute exists on the shader, then bind the data to it
		if (bind >= 0)
		{
			glEnableVertexAttribArray(bind);
			GL_CHECK_ERROR();

			glVertexAttribPointer(bind,					// Where the bind point is at
				attribute.m_elementCount,				// Number of components in this data type
				ToGLType(attribute.m_dataType),									// glType of this data
				attribute.m_isNormalized,				// Are they normalized
				vertexStride,							// stride of the vertex
				(GLvoid*)attribute.m_memberOffset		// offset into the layout this member is
			);
		}

		GL_CHECK_ERROR();
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the OpenGL render state to the state specified in the state struct passed
//
void Renderer::BindRenderState(const RenderState& state) const
{
	//-----Cull Mode-----
 	switch (state.m_cullMode)
 	{
 	case CULL_MODE_NONE:
 		glDisable(GL_CULL_FACE);
 		break;
 	case CULL_MODE_BACK:
 		glEnable(GL_CULL_FACE);
 		glCullFace(GL_BACK);
 		break;
 	case CULL_MODE_FRONT:
 		glEnable(GL_CULL_FACE);
 		glCullFace(GL_FRONT);
 		break;
 	default:
 		break;
 	}

	// Fill Mode
	glPolygonMode(GL_FRONT_AND_BACK, ToGLType(state.m_fillMode));
	GL_CHECK_ERROR();

	// Winding Order
	glFrontFace(ToGLType(state.m_windOrder));
	GL_CHECK_ERROR();

	// Blending
	glEnable(GL_BLEND);
	glBlendEquationSeparate(ToGLType(state.m_colorBlendOp), ToGLType(state.m_alphaBlendOp));
	glBlendFuncSeparate(ToGLType(state.m_colorSrcFactor), ToGLType(state.m_colorDstFactor), ToGLType(state.m_alphaSrcFactor), ToGLType(state.m_alphaDstFactor));
	GL_CHECK_ERROR();

	// Depth
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(ToGLType(state.m_depthTest));
	glDepthMask(state.m_shouldWriteDepth ? GL_TRUE : GL_FALSE);
	GL_CHECK_ERROR();
}


//-----------------------------------------------------------------------------------------------
// Binds the model matrix to the buffer binding
//
void Renderer::BindModelMatrix(const Matrix44& model)
{
	m_modelUniformBuffer.SetCPUAndGPUData(sizeof(model), &model);
}


//-----------------------------------------------------------------------------------------------
// Binds the given VAO
//
void Renderer::BindVAO(unsigned int vaoHandle)
{
	glBindVertexArray(vaoHandle);
	GL_CHECK_ERROR();
}


//-----------------------------------------------------------------------------------------------
// Updates the VAO by binding the mesh data to the program
//
void Renderer::UpdateVAO(unsigned int& vaoHandle, Mesh* mesh, Material* material)
{
	ASSERT_OR_DIE(mesh != nullptr && material != nullptr, Stringf("Error: Renderer::UpdateVAO() received null parameters."));

	if (glIsVertexArray(vaoHandle) == GL_FALSE)
	{
		glGenVertexArrays(1, &vaoHandle);
		GL_CHECK_ERROR();
	}
 
	glBindVertexArray(vaoHandle);

	const Shader* shader = material->GetShader();
	BindMeshToProgram(shader->GetProgram(), mesh);
}


//-----------------------------------------------------------------------------------------------
// Frees the Vertex Array Object on the gpu
//
void Renderer::DeleteVAO(unsigned int& vaoHandle) const
{
	glDeleteVertexArrays(1, &vaoHandle);
	GL_CHECK_ERROR();
}


//-----------------------------------------------------------------------------------------------
// Draws the given draw call
//
void Renderer::Draw(const DrawCall& drawCall)
{
	// Bind all the state
	BindVAO(drawCall.GetVAOHandle());
	BindMaterial(drawCall.GetMaterial()); 
	BindRenderState(drawCall.GetMaterial()->GetShader()->GetRenderState());

	// Copy light data from draw call
	SetAmbientLight(drawCall.GetAmbience());
	EnableLightsForDrawCall(&drawCall);

	// Update the GPU with the light data
	m_lightUniformBuffer.CheckAndUpdateGPUData();

	// Bind the frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_currentCamera->GetFrameBufferHandle()); 
	GL_CHECK_ERROR();

	// MODEL BINDING - If there's more than one model, do instance draws
	int matrixCount = drawCall.GetModelMatrixCount();
	if (matrixCount > 1)
	{
		// Buffer the model data
		m_modelInstanceBuffer.CopyToGPU(sizeof(Matrix44) * matrixCount, drawCall.GetModelMatrixBuffer());

		glBindBuffer(GL_ARRAY_BUFFER, m_modelInstanceBuffer.GetHandle());

		// Bind the model matrix to the program as a vertex attribute
		int bind = glGetAttribLocation(drawCall.GetMaterial()->GetShader()->GetProgram()->GetHandle(), "INSTANCE_MODEL_MATRIX");

		// If the attribute exists on the shader, then bind the data to it as 4 separate vector4's (OpenGL doesn't support larger object bindings)
		if (bind >= 0)
		{
			for (int offset = 0; offset < 4; offset++)
			{
				glEnableVertexAttribArray(bind + offset);
				GL_CHECK_ERROR();

				glVertexAttribPointer(bind + offset,		// Where the bind point is at, offsetting for each column of the matrix
					4,										// Number of components in this data type (4 for Vector4)
					GL_FLOAT,								// glType of this data, which is 4 floats
					GL_FALSE,								// Don't normalize
					sizeof(Matrix44),						// Stride by the size of a matrix, since we're doing a column at a time
					(GLvoid*)(offset * sizeof(Vector4))		// offset into the matrix for this column
				);
			}	

			// Make the bindings instanced, so they don't update per vertex
			for (int offset = 0; offset < 4; offset++)
			{
				glVertexAttribDivisor(bind + offset, 1);
			}
		}
		else
		{
			ConsoleWarningf("Warning: Renderer::Draw() attempted instanced draw with a shader that doesn't support instance draws");
		}

		// Instance draw using the instruction
		DrawInstruction instruction = drawCall.GetMesh()->GetDrawInstruction();
		if (instruction.m_usingIndices)
		{
			// Draw with indices
			glDrawElementsInstanced(ToGLType(instruction.m_primType), instruction.m_elementCount, GL_UNSIGNED_INT, 0, matrixCount);
		}
		else
		{
			// Draw without indices
			glDrawArraysInstanced(ToGLType(instruction.m_primType), instruction.m_startIndex, instruction.m_elementCount, matrixCount);
		}
	}
	else
	{
		// Just bind the singular model matrix as a uniform buffer
		BindModelMatrix(drawCall.GetModelMatrix(0)); 

		// Draw using the instruction
		DrawInstruction instruction = drawCall.GetMesh()->GetDrawInstruction();
		if (instruction.m_usingIndices)
		{
			// Draw with indices
			glDrawElements(ToGLType(instruction.m_primType), instruction.m_elementCount, GL_UNSIGNED_INT, 0);
		}
		else
		{
			// Draw without indices
			glDrawArrays(ToGLType(instruction.m_primType), instruction.m_startIndex, instruction.m_elementCount);
		}
		GL_CHECK_ERROR();
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the given mesh to screen
//
void Renderer::DrawMesh(Mesh* mesh)
{
	Material* material = AssetDB::CreateOrGetSharedMaterial("Default_Opaque");
	DrawMeshWithMaterial(mesh, material);
}


//-----------------------------------------------------------------------------------------------
// Draws the given mesh with the given material
// Assumes the model matrix is the identity
//
void Renderer::DrawMeshWithMaterial(Mesh* mesh, Material* material)
{
	RenderableDraw_t draw;
	draw.sharedMaterial = material;
	draw.mesh = mesh;
	draw.vaoHandle = m_defaultVAO;

	// Sets the pair, and also updates the default VAO
	m_immediateRenderable.ClearDraws();
	m_immediateRenderable.AddDraw(draw);

	DrawRenderable(&m_immediateRenderable);
}


//-----------------------------------------------------------------------------------------------
// Draws the renderable given
//
void Renderer::DrawRenderable(Renderable* renderable)
{
	int numDraws = renderable->GetDrawCountPerInstance();

	for (int drawIndex = 0; drawIndex < numDraws; ++drawIndex)
	{
		DrawCall dc;
		dc.SetDataFromRenderable(renderable, drawIndex);
		Draw(dc);
	}
}


//-----------------------------------------------------------------------------------------------
// Clears the depth buffer to the value specified
//
void Renderer::ClearDepth(float clearDepth /*= 1.0f*/)
{
	glDepthMask(GL_TRUE);
	glClearDepthf(clearDepth);
	glClear(GL_DEPTH_BUFFER_BIT);
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
// Returns the default sampler of the renderer
//
const Sampler* Renderer::GetDefaultSampler() const
{
	return m_defaultSampler;
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
	// Create and bind a default texture sampler
	m_defaultSampler = new Sampler();
	bool successful = m_defaultSampler->Initialize(SAMPLER_FILTER_NEAREST, EDGE_SAMPLING_REPEAT);
	GUARANTEE_OR_DIE(successful, Stringf("Error: Default Sampler could not be constructed successfully."));

	m_shadowSampler = new Sampler();
	m_shadowSampler->Initialize(SAMPLER_FILTER_LINEAR, EDGE_SAMPLING_CLAMP_TO_BORDER);

	// the default color and depth should match our output window
	// so get width/height however you need to.
	unsigned int windowWidth	= Window::GetInstance()->GetWidthInPixels(); 
	unsigned int windowHeight	= Window::GetInstance()->GetHeightInPixels();

	// create our output textures
	m_defaultColorTarget = CreateRenderTarget(windowWidth, windowHeight);
	m_defaultDepthTarget = CreateDepthTarget(windowWidth, windowHeight); 

	// Create the immediate renderable and the default VAO
	glGenVertexArrays(1, &m_defaultVAO); 
	glBindVertexArray(m_defaultVAO);

	RenderableDraw_t draw;
	draw.mesh = &m_immediateMesh;
	draw.sharedMaterial = AssetDB::CreateOrGetSharedMaterial("Default_Opaque");
	draw.vaoHandle = m_defaultVAO;
	m_immediateRenderable.AddDraw(draw);
	m_immediateRenderable.AddInstanceMatrix(Matrix44::IDENTITY);

	GL_CHECK_ERROR();

	// Set up initial GL state, using the state specified in the default shader
	Shader* defaultShader = AssetDB::GetShader("Default_Opaque");	// Should just "get" since the Default program is built in, already made

	BindRenderState(defaultShader->GetRenderState());

	// Setup Uniform buffers
	m_timeUniformBuffer.InitializeCPUBufferForType<TimeBufferData>();
	m_lightUniformBuffer.InitializeCPUBufferForType<LightBufferData>();
	m_modelUniformBuffer.SetCPUAndGPUData(sizeof(Matrix44), &Matrix44::IDENTITY);

	// Bind the UniformBuffers to the correct slots
	BindUniformBuffer(TIME_BUFFER_BINDING, m_timeUniformBuffer.GetHandle());
	BindUniformBuffer(LIGHT_BUFFER_BINDING, m_lightUniformBuffer.GetHandle());
	BindUniformBuffer(MODEL_BUFFER_BINDING, m_modelUniformBuffer.GetHandle());
}


//-----------------------------------------------------------------------------------------------
// Updates the GPU-side time data from the renderer
//
void Renderer::UpdateTimeData()
{
	Clock* master = Clock::GetMasterClock();

	TimeBufferData* timeData = m_timeUniformBuffer.GetCPUBufferAsType<TimeBufferData>();

	// In case we forgot to set the game clock on the renderer
	if (m_gameClock != nullptr)
	{
		timeData->m_gameDeltaTime	= m_gameClock->GetDeltaTime();
		timeData->m_gameTotalTime	= m_gameClock->GetTotalSeconds();
	}

	timeData->m_systemDeltaTime = master->GetDeltaTime();
	timeData->m_systemTotalTime = master->GetTotalSeconds();

	// CPU data set, now update the GPU
	m_timeUniformBuffer.CheckAndUpdateGPUData();
}


//-----------------------------------------------------------------------------------------------
// Draws to the screen given the vertices and the draw primitive type
//
void Renderer::DrawMeshImmediate(const Vertex3D_PCU* vertices, int vertexCount, PrimitiveType primitiveType /*= PRIMITIVE_TRIANGLES*/, const unsigned int* indices /*= nullptr*/, int indexCount /*= -1*/)
{
	m_immediateMesh.SetVertices(vertexCount, vertices);

	bool isUsingIndices = indices != nullptr;
	if (isUsingIndices)
	{
		m_immediateMesh.SetIndices(indexCount, indices);
	}
	
	DrawInstruction instruction;

	instruction.m_primType = primitiveType;
	instruction.m_startIndex = 0;
	instruction.m_usingIndices = isUsingIndices;
	instruction.m_elementCount = (isUsingIndices ? indexCount : vertexCount);

	m_immediateMesh.SetDrawInstruction(instruction);
	DrawMesh(&m_immediateMesh);
}


//-----------------------------------------------------------------------------------------------
// Draws a point at the given position with the given color and size
//
void Renderer::DrawPoint(const Vector3& position, const Rgba& color, float radius)
{
	Vertex3D_PCU vertices[14];

	vertices[0] = Vertex3D_PCU(position - Vector3::DIRECTION_RIGHT * radius, color, Vector2::ZERO);
	vertices[1] = Vertex3D_PCU(position + Vector3::DIRECTION_RIGHT * radius, color, Vector2::ZERO);

	vertices[2] = Vertex3D_PCU(position - Vector3::DIRECTION_UP * radius, color, Vector2::ZERO);
	vertices[3] = Vertex3D_PCU(position + Vector3::DIRECTION_UP * radius, color, Vector2::ZERO);

	vertices[4] = Vertex3D_PCU(position - Vector3::DIRECTION_FORWARD * radius, color, Vector2::ZERO);
	vertices[5] = Vertex3D_PCU(position + Vector3::DIRECTION_FORWARD * radius, color, Vector2::ZERO);

	vertices[6] = Vertex3D_PCU(position - Vector3(1.0f, 1.0f, 1.0f) * radius, color, Vector2::ZERO);
	vertices[7] = Vertex3D_PCU(position + Vector3(1.0f, 1.0f, 1.0f) * radius, color, Vector2::ZERO);

	vertices[8] = Vertex3D_PCU(position - Vector3(-1.0f, 1.0f, 1.0f) * radius, color, Vector2::ZERO);
	vertices[9] = Vertex3D_PCU(position + Vector3(-1.0f, 1.0f, 1.0f) * radius, color, Vector2::ZERO);

	vertices[10] = Vertex3D_PCU(position - Vector3(1.0f, 1.0f, -1.0f) * radius, color, Vector2::ZERO);
	vertices[11] = Vertex3D_PCU(position + Vector3(1.0f, 1.0f, -1.0f) * radius, color, Vector2::ZERO);

	vertices[12] = Vertex3D_PCU(position - Vector3(-1.0f, 1.0f, -1.0f) * radius, color, Vector2::ZERO);
	vertices[13] = Vertex3D_PCU(position + Vector3(-1.0f, 1.0f, -1.0f) * radius, color, Vector2::ZERO);

	DrawMeshImmediate(vertices, 14, PRIMITIVE_LINES);
}


//-----------------------------------------------------------------------------------------------
// Draws a line from startPos to endPos with the given colors
//
void Renderer::DrawLine(const Vector3& startPos, const Rgba& startColor, const Vector3& endPos, const Rgba& endColor, float width/*=1.0f*/)
{
	glLineWidth(width);

	Vertex3D_PCU vertices[2];

	vertices[0] = Vertex3D_PCU(startPos, startColor, Vector2::ZERO);
	vertices[1] = Vertex3D_PCU(endPos, endColor, Vector2::ZERO);

	DrawMeshImmediate(vertices, 2, PRIMITIVE_LINES);
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
	std::string filename = "screenshot.png";
	cmd.GetParam("f", filename, &filename);

	Renderer::GetInstance()->SaveScreenshotAtEndOfFrame(filename);
}