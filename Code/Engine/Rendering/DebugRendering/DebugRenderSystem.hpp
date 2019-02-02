/************************************************************************/
/* File: DebugRenderSystem.hpp
/* Author: Andrew Chase
/* Date: March 29th, 2018
/* Description: System that controls all debug rendering tasks
/************************************************************************/
#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask.hpp"

class Camera;
class Matrix44;

class DebugRenderSystem
{
public:
	//-----Public Methods-----

	// Setup/shutdown
	static void Initialize(Camera* worldCamera = nullptr);
	static void Shutdown();

	void UpdateAndRender();

	static void SetWorldCamera(Camera* worldCamera);	// No setter for ScreenSpace camera, for now we ensure we use one with height = pixel height of screen

	static DebugRenderSystem*	GetInstance();
	static Camera*				GetScreenCamera();
	static Camera*				GetWorldCamera();
	static Vector3				GetDefaultSpawnLocation();

	static bool					AreTasksBeingUpdated();
	static bool					AreTasksBeingRendered();


	static void Pause();
	static void Resume();
	static void ShowRenderTasks(bool showTasks);
	static void ToggleShowRenderTasks();
	static void Clear();

	static DebugRenderMode ConvertStringToRenderMode(const std::string& text);


public:
	//-----Drawing Functions-----

	// Base functions

	// 2D
	static void Draw2DQuad(const AABB2& bounds, const DebugRenderOptions& options);
	static void Draw2DLine(const Vector2& startPosition, const Vector2& endPosition, const DebugRenderOptions& options, const Rgba& endStartColor, const Rgba& endEndColor, float lineWidth = 1.0f);
	static void Draw2DText(const std::string& text, const AABB2& bounds, const DebugRenderOptions& options, float textHeight = 50.f, const Vector2& alignment = Vector2::ZERO);

	// 3D
	static void DrawPoint(const Vector3& position, const DebugRenderOptions& options, float radius = 1.0f);
	static void Draw3DLine(const Vector3& startPosition, const Vector3& endPosition, const DebugRenderOptions& options, const Rgba& endStartColor, const Rgba& endEndColor, float lineWidth = 1.0f);
	static void Draw3DQuad(const Vector3& position, const Vector2& dimensions, const DebugRenderOptions& options, const Vector3& rightVector = Vector3::X_AXIS, const Vector3& upVector = Vector3::Y_AXIS);
	static void DrawBasis(const Matrix44& basis, const DebugRenderOptions& options, float scale = 1.0f);
	static void DrawUVSphere(const Vector3& position, const DebugRenderOptions& options,  float radius = 1.0f, unsigned int numSlices = 4, unsigned int numWedges = 8);
	static void DrawCube(const Vector3& position, const DebugRenderOptions& options, const Vector3& dimensions);

	// Helper functions

	// 2D
	static void Draw2DQuad(const AABB2& bounds, const Rgba& color, float lifetime);
	static void Draw2DLine(const Vector2& startPosition, const Vector2& endPostion, const Rgba& color, float lifeTime, float lineWidth = 1.0f);
	static void Draw2DText(const std::string& text, const AABB2& bounds, float lifetime, const Rgba& color = Rgba::WHITE, float textHeight = 50.f, const Vector2& alignment = Vector2::ZERO);

	// 3D
	static void DrawPoint(const Vector3& position, float lifeTime, const Rgba& color = Rgba::WHITE, float radius = 1.0f);
	static void Draw3DLine(const Vector3& startPosition, const Vector3& endPostion, const Rgba& color, float lifeTime, float lineWidth = 1.0f);
	static void Draw3DQuad(const Vector3& position, const Vector2& dimensions, float lifetime, const Rgba& color = Rgba::WHITE);
	static void DrawBasis(const Vector3& position, const Vector3& rotation, float lifeTime, float scale = 1.0f);
	static void DrawUVSphere(const Vector3& position, float lifetime, const Rgba& color = Rgba::WHITE, float radius = 1.0f);
	static void DrawCube(const Vector3& position, float lifetime, const Rgba& color = Rgba::WHITE, const Vector3& dimensions = Vector3::ONES);


public:
	//-----Public Data-----

	static constexpr float DEFAULT_LIFETIME = 10.f;
	static constexpr float DEFAULT_XRAY_COLOR_SCALE = 0.25f;


private:
	//-----Private Methods-----

	DebugRenderSystem() {}
	~DebugRenderSystem();
	DebugRenderSystem(const DebugRenderSystem& copy) = delete;

	// Called by UpdateAndRender()
	void Update();
	void Render() const;


private:
	//-----Private Data-----

	Camera* m_screenCamera; // For 2D drawing in pixel coordinates
	Camera* m_worldCamera;	// For 3D drawing in world space

	bool m_updatePaused = false;
	bool m_renderTasks = true;

	// All rendering tasks currently being drawn
	std::vector<DebugRenderTask*> m_tasks;

	// Singleton instance
	static DebugRenderSystem* s_instance;

	// Constants
	static constexpr float CAMERA_SPAWN_DISTANCE = 10.f;

};
