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
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Rendering/Meshes/Mesh.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Rendering/Shaders/Shader.hpp"
#include "Engine/Rendering/Core/DrawCall.hpp"
#include "Engine/Rendering/Resources/BitmapFont.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Rendering/Buffers/UniformBuffer.hpp"
// Defines
#define TIME_BUFFER_BINDING (0)		// Updated once per frame
#define CAMERA_BUFFER_BINDING (1)	// Updated ~once per frame
#define MODEL_BUFFER_BINDING (2)	// Updated per draw
#define LIGHT_BUFFER_BINDING (3)	// Updated one per frame
#define SKINNING_BONE_BINDING (4)

#define SHADOW_TEXTURE_BINDING (8)	// Slot for the shadow texture

// Class Predeclarations
class Camera;
class Sampler;
class Texture;
class FrameBuffer;
class ShaderProgram;
class Clock;
class Material;

// For TextInBox draw styles
enum TextDrawMode
{
	TEXT_DRAW_ERROR = -1,
	TEXT_DRAW_SHRINK_TO_FIT,
	TEXT_DRAW_OVERRUN,
	TEXT_DRAW_WORD_WRAP,
	NUM_TEXT_DRAW_MODES
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
	bool CopyFrameBuffer(FrameBuffer* destination, FrameBuffer* source);


public:
	//-----Renderer State-----

	// Camera
	void SetCurrentCamera(Camera* camera);

	// Time
	void SetRendererGameClock(Clock* gameClock);


private:
	//-----Lighting-----

	void AdjustAmbientIntensity(float deltaAmount);
	void SetAmbientIntensity(float newIntensity);
	void SetAmbientLight(const Rgba& color);
	void SetAmbientLight(const Vector4& color);

	void EnablePointLight(unsigned int index, const Vector3& position, const Rgba& color = Rgba::WHITE, const Vector3& attenuation = Vector3(1.f, 0.f, 0.f));
	void EnableDirectionalLight(unsigned int index, const Vector3& position, const Vector3& direction = Vector3::DIRECTION_DOWN, const Rgba& color = Rgba::WHITE, const Vector3& attenuation = Vector3(1.f, 0.f, 0.f));
	void EnableSpotLight(unsigned int index, const Vector3& position, const Vector3& direction, float outerAngle, float innerAngle, const Rgba& color = Rgba::WHITE, const Vector3& attenuation = Vector3(1.f, 0.f, 0.f));

	void EnableLightsForDrawCall(const DrawCall* drawCall);

	void DisableAllLights();


private:
	//-----Mutators on OpenGL state-----

	// Render State ---------------------------------------------------------------------------------------------------------------------------------

	void BindRenderState(const RenderState& state) const;

	// Material -------------------------------------------------------------------------------------------------------------------------------------

	void BindMaterial(Material* material);

	// Texture
	void BindTexture(unsigned int bindSlot, const std::string& filename);
	void BindTexture(unsigned int bindSlot, const Texture* texture, const Sampler* sampler = nullptr);

	// Uniforms
	void BindUniformBuffer(unsigned int bindSlot, unsigned int bufferHandle) const;

	// Model Matrix ---------------------------------------------------------------------------------------------------------------------------------

	void BindModelMatrix(const Matrix44& model);

	// VAO ------------------------------------------------------------------------------------------------------------------------------------------

	void BindMeshToProgram(const ShaderProgram* program, const Mesh* mesh) const;
	void BindVAO(unsigned int vaoHandle);


public:
	//-----Drawing-----

	// For ALL drawing
	void DrawMeshImmediate(const Vertex3D_PCU* verts, int numVerts, PrimitiveType drawPrimitive = PRIMITIVE_TRIANGLES, const unsigned int* indices = nullptr, int numIndices = -1);
	void DrawMesh(Mesh* mesh);
	void DrawMeshWithMaterial(Mesh* mesh, Material* material);
	void DrawRenderable(Renderable* renderable);
	void Draw(const DrawCall& drawCall);

	// Drawing convenience functions

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void DrawPoint(const Vector3& position, const Rgba& color, float radius);

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void DrawLine(const Vector3& startPos, const Rgba& startColor, const Vector3& endPos, const Rgba& endColor, float width = 1.0f);
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Draw2DQuad(const AABB2& bounds, const AABB2& textureUVs, const Rgba& tint, Material* material = nullptr);
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void DrawSprite(const Sprite* sprite, const Vector3& position, const Rgba& tint = Rgba::WHITE, const Vector3& right = Vector3::DIRECTION_RIGHT, const Vector3& up = Vector3::DIRECTION_UP);

	//--------------------------------------------------------------------------------------------------s---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Draw3DQuad(const Vector3& position, const Vector2& dimensions, const AABB2& textureUVs, const Vector3& right = Vector3::DIRECTION_RIGHT, const Vector3& up = Vector3::DIRECTION_UP, const Rgba& tint = Rgba::WHITE, const Vector2& pivot = Vector2(0.5f, 0.5f), Material* material = nullptr);
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void DrawCube(const Vector3& center, const Vector3& dimensions, const Rgba& tint = Rgba::WHITE, const AABB2& topUVs = AABB2::UNIT_SQUARE_OFFCENTER, const AABB2& sideUVs = AABB2::UNIT_SQUARE_OFFCENTER, const AABB2& bottomUVs = AABB2::UNIT_SQUARE_OFFCENTER, Material* material = nullptr);
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void DrawSphere(const Vector3& position, float radius, unsigned int numWedges, unsigned int numSlices, const Rgba& color = Rgba::WHITE, Material* material = nullptr);

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void DrawText2D(const std::string& text, const Vector2& drawMins, float cellHeight, BitmapFont* font, Rgba color=Rgba::WHITE, float aspectScale=1.0f);
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void DrawTextInBox2D(const std::string& text, const AABB2& box, const Vector2& alignment, float cellHeight, TextDrawMode drawMode, BitmapFont* font, Rgba color=Rgba::WHITE, float aspectScale=1.0f);
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ClearScreen(const Rgba& clearColor);
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ClearDepth(float clearDepth = 1.0f);


public:
	//-----Post Processed Effects-----

	void ApplyImageEffect(ShaderProgram* program);
	void FinalizeImageEffects();


public:
	//-----Utility-----

	// Render targets
	Texture*		CreateRenderTarget(unsigned int width, unsigned int height, TextureFormat format = TEXTURE_FORMAT_RGBA8);
	Texture*		CreateDepthTarget(unsigned int width, unsigned int height);

	// VAOs
	void			UpdateVAO(unsigned int& vaoHandle, Mesh* mesh, Material* material);
	void			DeleteVAO(unsigned int& vaoHandle) const;

	// Screenshots
	void			SaveScreenshotAtEndOfFrame(const std::string& filename);


public:
	//-----Accessors-----

	Texture*		GetDefaultColorTarget() const;
	Texture*		GetDefaultDepthTarget() const;

	Camera*			GetDefaultCamera() const;
	Camera*			GetUICamera() const;
	static AABB2	GetUIBounds();

	const Sampler*	GetDefaultSampler() const;


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

	// For updating time
	void UpdateTimeData();

	void SaveScreenshotToFile();		// Called during EndFrame()


public:
	//-----Public Data-----

	const static float UI_ORTHO_HEIGHT;		// Height of the UI ortho projection (1080)


private:
	//-----Private Data-----
	
	// Drawing state variables
	Mesh					m_immediateMesh;
	MeshBuilder				m_immediateBuilder;
	Renderable				m_immediateRenderable;

	Sampler*				m_defaultSampler = nullptr;
	Sampler*				m_shadowSampler = nullptr;

	Camera*					m_defaultCamera = nullptr;
	Camera*					m_currentCamera = nullptr;
	Camera*					m_UICamera = nullptr;

	Texture*				m_defaultColorTarget = nullptr;	// Targets are also already on the default camera
	Texture*				m_defaultDepthTarget = nullptr;	// Targets are also already on the default camera

	// For screenshots
	bool					m_saveScreenshotThisFrame = false;
	std::string				m_screenshotFilename;

	// For post-processed effects
	Camera*					m_effectsCamera;
	Texture*				m_effectsSource = nullptr;
	Texture*				m_effectsDestination = nullptr;

	// Time
	Clock* m_gameClock = nullptr;

	// Uniform buffers
	UniformBuffer			m_timeUniformBuffer;
	UniformBuffer			m_modelUniformBuffer;
	mutable RenderBuffer	m_modelInstanceBuffer;
	mutable UniformBuffer	m_lightUniformBuffer;

	// VAO
	GLuint m_defaultVAO;

	//-----Static Data-----
	const static IntVector2 FONT_SPRITE_LAYOUT;			// Default dimensions of the font texture
	const static char* FONT_DIRECTORY;					// Default directory where fonts are stored
	const static char* SHADER_DIRECTORY;				// Default directory where shaders are stored

	static AABB2 s_UIOrthoBounds;						// Bounds of the UI ortho ((0,0) to (UI height * aspect, UI height))

	static Renderer* s_instance;						// Reference to the current renderer instance (singleton class)
};
