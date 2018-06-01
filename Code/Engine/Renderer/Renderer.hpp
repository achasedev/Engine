/************************************************************************/
/* File: Renderer.hpp
/* Author: Andrew Chase
/* Date: January 25th, 2017
/* Bugs: None
/* Description: Class used to call OpenGL functions to draw to screen
/************************************************************************/
#pragma once
#include <string>
#include <vector>
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "ThirdParty/gl/glcorearb.h"

class ShaderProgram;
class Sampler;
class Texture;
class RenderBuffer;
class Camera;
class FrameBuffer;

// Vertex structure
struct Vertex3D_PCU
{
	// Constructors
	Vertex3D_PCU() {};
	Vertex3D_PCU(const Vector3& position, const Rgba& color, const Vector2& texUVs)
		: m_position(position), m_color(color), m_texUVs(texUVs) {}

	Vector3 m_position;	// Position of the Vertex
	Rgba	m_color;	// Color of the Vertex
	Vector2 m_texUVs;	// Texture UV coordinates for this vertex
};

// For GL Blending
enum BlendMode
{
	BLEND_MODE_ERROR = -1,
	BLEND_MODE_ALPHA,
	BLEND_MODE_ADDITIVE,
	NUM_BLEND_MODES
};

// For GL drawing types
enum PrimitiveType
{
	PRIMITIVE_POINTS,		// in OpenGL, for example, this becomes GL_POINTS
	PRIMITIVE_LINES,		// in OpenGL, for example, this becomes GL_LINES
	PRIMITIVE_TRIANGLES,	// in OpenGL, for example, this becomes GL_TRIANGLES
	PRIMITIVE_QUADS,		// in OpenGL, for example, this becomes GL_QUADS
	NUM_PRIMITIVE_TYPES
};

// For TextInBox draw styles
enum TextDrawMode
{
	TEXT_DRAW_ERROR = -1,
	TEXT_DRAW_SHRINK_TO_FIT,
	TEXT_DRAW_OVERRUN,
	TEXT_DRAW_WORD_WRAP,
	NUM_TEXT_DRAW_MODES
};

// For GL Depth tests
enum DepthCompare
{
	COMPARE_NEVER,       // GL_NEVER
	COMPARE_LESS,        // GL_LESS
	COMPARE_LEQUAL,      // GL_LEQUAL
	COMPARE_GREATER,     // GL_GREATER
	COMPARE_GEQUAL,      // GL_GEQUAL
	COMPARE_EQUAL,       // GL_EQUAL
	COMPARE_NOT_EQUAL,   // GL_NOTEQUAL
	COMPARE_ALWAYS,      // GL_ALWAYS
	NUM_COMPARES		 // Count
};


class Renderer
{
public:
	//-----Structure-----

	// Initialization
	static void Initialize();
	static void Shutdown();

	static Renderer* GetInstance();

	// Updating/Rendering
	void BeginFrame();		// Sets up the frame for rendering
	void EndFrame();		// Finishes render processes and swaps the buffers

	// Finalizing
	bool CopyFrameBuffer( FrameBuffer* destination, FrameBuffer* source);


public:
	//-----Renderer State-----

	// Shader
	void SetCurrentShaderProgram(const ShaderProgram* program);
	void SetCurrentShaderProgram(const std::string& programName);

	// Camera
	void SetCurrentCamera(Camera* camera);

	void SetProjectionOrtho(float width, float height, float nearZ, float farZ);
	void SetProjectionMatrix(const Matrix44& projectionMatrix);
	void SetViewMatrix(const Matrix44& viewMatrix);
	void SetLookAt(const Vector3& position, const Vector3& target, const Vector3& up = Vector3::DIRECTION_UP);

	// Matrix
	void SetModelMatrix(const Matrix44& matrix);


public:
	//-----OpenGL State-----

	// Depth
	void EnableDepth(DepthCompare compareMethod, bool shouldWrite);
	void DisableDepth();
	void ClearDepth(float clearDepth = 1.0f);

	// Blending
	void EnableBlendMacro();												// Enables the gl macro	
	void SetBlendMode(BlendMode nextMode);									// Sets the blending function for drawing

	// Texture
	void BindTexture(unsigned int bindSlot, unsigned int textureHandle);
	void BindTexture(unsigned int bindSlot, const std::string& texturePath);

	// Uniform Binding
	void BindUniformFloat(const std::string& uniformName, float uniformValue) const;


public:
	//-----Drawing-----

	// For ALL drawing
	void DrawMeshImmediate(const Vertex3D_PCU* verts, int numVerts, PrimitiveType drawPrimitive = PRIMITIVE_TRIANGLES, const unsigned int* indices = nullptr, int numIndices = -1);

	// Drawing convenience functions
	void DrawLine(const Vector3& startPos, const Rgba& startColor, const Vector3& endPos, const Rgba& endColor, float width);		// Draws a colored straight line between two points
	void DrawAABB2(const AABB2& bounds, const AABB2& textureUVs, const Rgba& tint);													// Draws a textured AABB2 with the given texture information
	void DrawAABB2_3D(const Vector3& position, const Vector2& dimensions, const AABB2& textureUVs, const Vector3& right = Vector3::DIRECTION_RIGHT, const Vector3& up = Vector3::DIRECTION_UP, const Rgba& tint = Rgba::WHITE, const Vector2& pivot = Vector2(0.5f, 0.5f));
	void DrawCube(const Vector3& center, const Vector3& dimensions, const Rgba& tint = Rgba::WHITE,									// Draws a cube (cuboid) with the given corner positions and tint
		const AABB2& topUVs = AABB2::UNIT_SQUARE_OFFCENTER, const AABB2& sideUVs = AABB2::UNIT_SQUARE_OFFCENTER, const AABB2& bottomUVs = AABB2::UNIT_SQUARE_OFFCENTER);
	void DrawSprite(const Sprite* sprite, const Vector3& position, const Rgba& tint = Rgba::WHITE, const Vector3& right = Vector3::DIRECTION_RIGHT, const Vector3& up = Vector3::DIRECTION_UP);

	// "Mesh Building"
	void AppendAABB2Vertices2D(Vertex3D_PCU* vertexArray, int& vertexOffset, unsigned int* indexArray, int& indexOffset, const AABB2& bounds, const AABB2& textureUVs, const Rgba& tint = Rgba::WHITE);
	void AppendAABB2Vertices3D(Vertex3D_PCU* vertexArray, int& vertexOffset, unsigned int* indexArray, int& indexOffset, const Vector3& position,
		const Vector2& dimensions, const AABB2& textureUVs, const Vector3& right = Vector3::DIRECTION_RIGHT, const Vector3& up = Vector3::DIRECTION_UP, const Rgba& tint = Rgba::WHITE, const Vector2& pivot = Vector2(0.5f, 0.5f));
	void AppendCubeVertices(Vertex3D_PCU* vertexArray, int& vertexOffset, unsigned int* indexArray, int& indexOffet, const Vector3& center, const Vector3& dimensions, const Rgba& tint = Rgba::WHITE,						
		const AABB2& topUVs = AABB2::UNIT_SQUARE_OFFCENTER, const AABB2& sideUVs = AABB2::UNIT_SQUARE_OFFCENTER, const AABB2& bottomUVs = AABB2::UNIT_SQUARE_OFFCENTER);

	// Font drawing
	void DrawText2D(const std::string& text, const Vector2& drawMins, float cellHeight, BitmapFont* font, Rgba color=Rgba::WHITE, float aspectScale=1.0f);
	void DrawTextInBox2D(const std::string& text, const AABB2& box, const Vector2& alignment, float cellHeight, TextDrawMode drawMode, BitmapFont* font, Rgba color=Rgba::WHITE, float aspectScale=1.0f);

	// Changing GL states (most unimplemented from updating to GL 4.2+)
	void ClearScreen(const Rgba& clearColor);								// Clears the canvas to a single color	


public:
	//-----Post Processed Effects-----

	void ApplyImageEffect(ShaderProgram* shader);
	void ApplyImageEffect(const std::string& shaderName);

	void FinalizeImageEffects();


public:
	//-----Resource Management-----

	Texture*		CreateOrGetTexture(std::string texturePath);	
	BitmapFont*		CreateOrGetBitmapFont(const char* bitmapFontName);
	ShaderProgram*	CreateOrGetShaderProgram(const std::string& shaderName);

	void			ReloadShaders();


public:
	//-----Utility-----

	// Render targets
	Texture*		CreateRenderTarget(unsigned int width, unsigned int height, TextureFormat format = TEXTURE_FORMAT_RGBA8);
	Texture*		CreateDepthTarget(unsigned int width, unsigned int height);

	// Screenshots
	void			SaveScreenshotAtEndOfFrame(const std::string& filename);


public:
	//-----Accessors-----

	Texture*		GetDefaultColorTarget() const;
	Texture*		GetDefaultDepthTarget() const;

	Camera*			GetUICamera() const;
	static AABB2	GetUIBounds();


private:
	//-----Private Methods-----	

	// Constructor/Destructor - singleton so no construction allowed outside of Initialize()
	Renderer();
	~Renderer();
	Renderer(const Renderer& copy) = delete;

	// DrawTextInBox2D helper functions
	void DrawTextInBox2D_Overrun(const std::string& text, const AABB2& box, const Vector2& alignment, float cellHeight, BitmapFont* font, Rgba color=Rgba::WHITE, float aspectScale=1.0f);
	void DrawTextInBox2D_ShrinkToFit(const std::string& text, const AABB2& box, const Vector2& alignment, float cellHeight, BitmapFont* font, Rgba color=Rgba::WHITE, float aspectScale=1.0f);
	void DrawTextInBox2D_WordWrap(const std::string& text, const AABB2& box, const Vector2& alignment, float cellHeight, BitmapFont* font, Rgba color=Rgba::WHITE, float aspectScale=1.0f);

	// For setting up the renderer after the OpenGL context is made
	void PostGLStartup();

	// Helper functions
	void CreateBuiltInShaderPrograms();	// Called during Initialize()
	void CreateBuiltInTextures();		// Called during Initialize()

	void SaveScreenshotToFile();		// Called during EndFrame()


public:
	//-----Public Data-----

	const static float UI_ORTHO_HEIGHT;		// Height of the UI ortho projection (1080)


private:
	//-----Private Data-----
	
	// Drawing state variables
	RenderBuffer*			m_vertexBuffer;
	RenderBuffer*			m_indexBuffer;
	const ShaderProgram*	m_defaultShaderProgram = nullptr;
	const ShaderProgram*	m_currentShaderProgram = nullptr;
	Sampler*				m_defaultSampler = nullptr;

	Camera*					m_defaultCamera = nullptr;
	Camera*					m_currentCamera = nullptr;
	Camera*					m_UICamera = nullptr;

	Texture*				m_defaultColorTarget = nullptr;	// Targets are also already on the default camera
	Texture*				m_defaultDepthTarget = nullptr;	// Targets are also already on the default camera

	Matrix44				m_currentModelMatrix;

	// For screenshots
	bool					m_saveScreenshotThisFrame = false;
	std::string				m_screenshotFilename;

	// For post-processed effects
	Camera*					m_effectsCamera;
	Texture*				m_effectsSource = nullptr;
	Texture*				m_effectsDestination = nullptr;

	// Loaded assets
	std::map<std::string, Texture*>			m_loadedTextures;		// Textures stored by file PATH
	std::map<std::string, BitmapFont*>		m_loadedFonts;			// Fonts stored by file NAME
	std::map<std::string, ShaderProgram*>	m_loadedShaderPrograms;	// Shader programs stored by NAME

	// VAO
	GLuint m_defaultVAO;

	//-----Static Data-----
	const static IntVector2 FONT_SPRITE_LAYOUT;			// Default dimensions of the font texture
	const static char* FONT_DIRECTORY;					// Default directory where fonts are stored
	const static char* SHADER_DIRECTORY;				// Default directory where shaders are stored

	static AABB2 s_UIOrthoBounds;						// Bounds of the UI ortho ((0,0) to (UI height * aspect, UI height))

	static Renderer* s_instance;						// Reference to the current renderer instance (singleton class)
};
