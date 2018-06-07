/************************************************************************/
/* File: DebugRenderSystem.cpp
/* Author: Andrew Chase
/* Date: March 29th, 2018
/* Description: System that controls all debug rendering tasks
/************************************************************************/
#include "Engine/Core/Window.hpp"
#include "Engine/Core/DeveloperConsole/Command.hpp"
#include "Engine/Rendering/Core/Camera.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask_Cube.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask_Point.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask_Basis.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask_Line2D.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask_Line3D.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask_Quad2D.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask_Quad3D.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask_Text2D.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask_UVSphere.hpp"

// Singleton instance
DebugRenderSystem*	DebugRenderSystem::s_instance = nullptr;

// Commands
void Command_DebugRenderClear(Command& cmd);
void Command_DebugRenderPause(Command& cmd);
void Command_DebugRenderResume(Command& cmd);
void Command_DebugRenderShow(Command& cmd);

void Command_DebugRender2DLine(Command& cmd);
void Command_DebugRender2DQuad(Command& cmd);
void Command_DebugRender2DText(Command& cmd);

void Command_DebugRenderCube(Command& cmd);
void Command_DebugRenderPoint(Command& cmd);
void Command_DebugRenderSphere(Command& cmd);
void Command_DebugRender3DLine(Command& cmd);
void Command_DebugRenderBasis(Command& cmd);
void Command_DebugRender3DQuad(Command& cmd);

//-----------------------------------------------------------------------------------------------
// Destructor
//
DebugRenderSystem::~DebugRenderSystem()
{
	delete m_screenCamera;
	m_screenCamera = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Sets up the system
//
void DebugRenderSystem::Initialize(Camera* worldCamera /*nullptr*/)
{
	if (s_instance == nullptr)
	{
		s_instance = new DebugRenderSystem();

		Renderer* renderer = Renderer::GetInstance();

		// Set up the world camera
		if (worldCamera != nullptr)
		{
			s_instance->m_worldCamera = worldCamera;
		}
		else
		{
			// If one isn't specified, just use the default renderer camera
			s_instance->m_worldCamera = Renderer::GetInstance()->GetDefaultCamera();
		}

		// Set up the UI camera
		s_instance->m_screenCamera = new Camera();
		s_instance->m_screenCamera->SetColorTarget(renderer->GetDefaultColorTarget());
		s_instance->m_screenCamera->SetDepthTarget(renderer->GetDefaultDepthTarget());

		Matrix44 orthoMat = Matrix44::MakeOrtho(Vector2::ZERO, Window::GetInstance()->GetDimensions());
		s_instance->m_screenCamera->SetProjection(orthoMat);

		// Register the commands to the DevConsole
		Command::Register("debug_pause",	"Pauses the DebugRenderSystem Update",							Command_DebugRenderPause);
		Command::Register("debug_resume",	"Resumes the DebugRenderSystem Update",							Command_DebugRenderResume);
		Command::Register("debug_show",		"Shows/hides the debug rendering based on the bool parameter",	Command_DebugRenderShow);
		Command::Register("debug_clear",	"Clears the current DebugRenderSystem tasks",					Command_DebugRenderClear);

		Command::Register("debug_2dline",	"Draws a DebugRender2DLine with the specified params",			Command_DebugRender2DLine);
		Command::Register("debug_2dquad",	"Draws a DebugRender2DQuad with the specified params",			Command_DebugRender2DQuad);
		Command::Register("debug_2dtext",	"Draws a DebugRender2DText with the specified params",			Command_DebugRender2DText);

		Command::Register("debug_cube",		"Draws a DebugRenderCube with the specified params",			Command_DebugRenderCube);
		Command::Register("debug_point",	"Draws a DebugRenderPoint with the specified params",			Command_DebugRenderPoint);
		Command::Register("debug_sphere",	"Draws a DebugRenderSphere with the specified params",			Command_DebugRenderSphere);
		Command::Register("debug_3dline",	"Draws a DebugRender3DLine with the specified params",			Command_DebugRender3DLine);
		Command::Register("debug_basis",	"Draws a DebugRenderBasis with the specified params",			Command_DebugRenderBasis);
		Command::Register("debug_3dquad",	"Draws a DebugRender3DQuad with the specified params",			Command_DebugRender3DQuad);
	}
}


//-----------------------------------------------------------------------------------------------
// Shuts down and cleans up the system
//
void DebugRenderSystem::Shutdown()
{
	delete s_instance;
	s_instance = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Calls update on all current tasks
//
void DebugRenderSystem::Update()
{
	if (m_updateTasks)
	{
		return;
	}

	// First check for finished tasks
	for (int taskIndex = 0; taskIndex < (int) m_tasks.size(); ++taskIndex)
	{
		if (m_tasks[taskIndex]->IsFinished())
		{
			delete m_tasks[taskIndex];
			m_tasks.erase(m_tasks.begin() + taskIndex);

			taskIndex--;
		}
		else
		{
			m_tasks[taskIndex]->Update();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Draws all current tasks to screen
//
void DebugRenderSystem::Render() const
{
	if (m_renderTasks)
	{
		for (int taskIndex = 0; taskIndex < (int) m_tasks.size(); ++taskIndex)
		{
			m_tasks[taskIndex]->Render();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the 3D camera for rendering to the one specified
//
void DebugRenderSystem::SetWorldCamera(Camera* worldCamera)
{
	s_instance->m_worldCamera = worldCamera;
}


//-----------------------------------------------------------------------------------------------
// Returns the DebugRenderSystem singleton instance
//
DebugRenderSystem* DebugRenderSystem::GetInstance()
{
	return s_instance;
}


//-----------------------------------------------------------------------------------------------
// Returns the camera used for 2D renders
//
Camera* DebugRenderSystem::GetScreenCamera()
{
	return s_instance->m_screenCamera;
}


//-----------------------------------------------------------------------------------------------
// Returns the camera used for 3D renders
//
Camera* DebugRenderSystem::GetWorldCamera()
{
	return s_instance->m_worldCamera;
}


//-----------------------------------------------------------------------------------------------
// Returns a default spawn location in front of the DebugRenderSystem's current world camera
//
Vector3 DebugRenderSystem::GetDefaultSpawnLocation()
{
	Vector3 cameraPosition = s_instance->m_worldCamera->GetPosition();
	Vector3 cameraForward = s_instance->m_worldCamera->GetForwardVector();

	return cameraPosition + cameraForward * CAMERA_SPAWN_DISTANCE;
}


//-----------------------------------------------------------------------------------------------
// Returns whether the tasks are currently being updated each frame (times decremented)
//
bool DebugRenderSystem::AreTasksBeingUpdated()
{
	return !s_instance->m_updateTasks;
}


//-----------------------------------------------------------------------------------------------
// Returns whether the tasks are currently being rendered each frame
//
bool DebugRenderSystem::AreTasksBeingRendered()
{
	return s_instance->m_renderTasks;
}


//-----------------------------------------------------------------------------------------------
// Pauses the update on all current tasks
//
void DebugRenderSystem::Pause()
{
	s_instance->m_updateTasks = true;
}


//-----------------------------------------------------------------------------------------------
// Resumes the update on all current tasks
//
void DebugRenderSystem::Resume()
{
	s_instance->m_updateTasks = false;
}


//-----------------------------------------------------------------------------------------------
// Pauses the update on all current tasks
//
void DebugRenderSystem::ShowRenderTasks(bool showTasks)
{
	s_instance->m_renderTasks = showTasks;
}


//-----------------------------------------------------------------------------------------------
// Toggles whether or not the debug rendering should render
//
void DebugRenderSystem::ToggleShowRenderTasks()
{
	s_instance->m_renderTasks = !s_instance->m_renderTasks;
}


//-----------------------------------------------------------------------------------------------
// Clears all current tasks on the current task list
//
void DebugRenderSystem::Clear()
{
	for (int taskIndex = 0; taskIndex < (int) s_instance->m_tasks.size(); ++taskIndex)
	{
		delete s_instance->m_tasks[taskIndex];
	}

	s_instance->m_tasks.clear();
}


//-----------------------------------------------------------------------------------------------
// Returns the DebugRenderMode associated with text
// If the text doesn't match any modes, then it defaults to use_depth
//
DebugRenderMode DebugRenderSystem::ConvertStringToRenderMode(const std::string& text)
{
	if		(text == "ignore_depth")	{ return DEBUG_RENDER_IGNORE_DEPTH; }
	else if (text == "hidden")			{ return DEBUG_RENDER_HIDDEN; }
	else if (text == "xray")			{ return DEBUG_RENDER_XRAY; }
	else
	{
		return DEBUG_RENDER_USE_DEPTH;	// Default
	}
}


//-----------------------------------------------------------------------------------------------
// Draws a debug render point, base function
//
void DebugRenderSystem::DrawPoint(const Vector3& position, const DebugRenderOptions& options, float radius /*= 1.0f*/)
{
	DebugRenderTask_Point* point = new DebugRenderTask_Point(position, options, radius);
	s_instance->m_tasks.push_back(point);
}


//-----------------------------------------------------------------------------------------------
// Draws a debug render point, helper function
//
void DebugRenderSystem::DrawPoint(const Vector3& position, float lifeTime, const Rgba& color /*= Rgba::WHITE*/, float radius /*= 1.0f*/)
{
	DebugRenderOptions options = DebugRenderOptions();
	options.m_lifetime = lifeTime;
	options.m_startColor = color;
	options.m_endColor = color;

	DrawPoint(position, options, radius);
}


//-----------------------------------------------------------------------------------------------
// Draws a debug render line, base function
//
void DebugRenderSystem::Draw3DLine(const Vector3& startPosition, const Vector3& endPosition, const DebugRenderOptions& options, const Rgba& endStartColor, const Rgba& endEndColor, float lineWidth /*= 1.0f*/)
{
	DebugRenderTask_Line3D* line = new DebugRenderTask_Line3D(startPosition, endPosition, options, endStartColor, endEndColor, lineWidth);
	s_instance->m_tasks.push_back(line);
}


//-----------------------------------------------------------------------------------------------
// Draws a debug render line, helper function
//
void DebugRenderSystem::Draw3DLine(const Vector3& startPosition, const Vector3& endPostion, const Rgba& color, float lifeTime, float lineWidth /*= 1.0f*/)
{
	DebugRenderOptions options;

	options.m_startColor = color;
	options.m_endColor = color;
	options.m_lifetime = lifeTime;
	
	Draw3DLine(startPosition, endPostion, options, color, color, lineWidth);
}


//-----------------------------------------------------------------------------------------------
// Draws a debug render 3D quad, base function
//
void DebugRenderSystem::Draw3DQuad(const Vector3& position, const Vector2& dimensions, const DebugRenderOptions& options, const Vector3& rightVector /*= Vector3::DIRECTION_RIGHT*/, const Vector3& upVector /*= Vector3::DIRECTION_UP*/)
{
	DebugRenderTask_Quad3D* quad = new DebugRenderTask_Quad3D(position, dimensions, options, rightVector, upVector);
	s_instance->m_tasks.push_back(quad);
}


//-----------------------------------------------------------------------------------------------
// Draws a debug render quad, helper function
//
void DebugRenderSystem::Draw3DQuad(const Vector3& position, const Vector2& dimensions, float lifetime, const Rgba& color /*= Rgba::WHITE*/)
{
	DebugRenderOptions options;

	options.m_startColor = color;
	options.m_endColor = color;
	options.m_lifetime = lifetime;

	Draw3DQuad(position, dimensions, options);
}


//-----------------------------------------------------------------------------------------------
// Draws a debug render basis, base function
//
void DebugRenderSystem::DrawBasis(const Matrix44& basis, const DebugRenderOptions& options, float scale /*= 1.0f*/)
{
	DebugRenderTask_Basis* task = new DebugRenderTask_Basis(basis, options, scale);
	s_instance->m_tasks.push_back(task);
}


//-----------------------------------------------------------------------------------------------
// Draws a debug render basis, helper function
//
void DebugRenderSystem::DrawBasis(const Vector3& position, const Vector3& rotation, float lifeTime, float scale /*= 1.0f*/)
{
	DebugRenderOptions options;
	options.m_lifetime = lifeTime;
	
	Matrix44 basis = Matrix44::MakeTranslation(position);
	basis.Append(Matrix44::MakeRotation(rotation));

	DrawBasis(basis, options, scale);
}


//-----------------------------------------------------------------------------------------------
// Draws a debug render sphere, base function
//
void DebugRenderSystem::DrawUVSphere(const Vector3& position, const DebugRenderOptions& options, float radius /*= 1.0f*/, unsigned int numSlices /*= 4*/, unsigned int numWedges /*= 8*/)
{
	DebugRenderTask_UVSphere* sphere = new DebugRenderTask_UVSphere(position, options, radius, numSlices, numWedges);
	s_instance->m_tasks.push_back(sphere);
}


//-----------------------------------------------------------------------------------------------
// Draws a debug render sphere, helper function
//
void DebugRenderSystem::DrawUVSphere(const Vector3& position, float lifetime, const Rgba& color /*= Rgba::WHITE*/, float radius /*= 1.0f*/)
{
	DebugRenderOptions options;

	options.m_startColor = color;
	options.m_endColor = color;
	options.m_lifetime = lifetime;

	DrawUVSphere(position, options, radius, 4, 8);
}


//-----------------------------------------------------------------------------------------------
// Draws a solid debug render cube, base function
//
void DebugRenderSystem::DrawCube(const Vector3& position, const DebugRenderOptions& options, const Vector3& dimensions)
{
	DebugRenderTask_Cube* cube = new DebugRenderTask_Cube(position, options, dimensions);
	s_instance->m_tasks.push_back(cube);
}


//-----------------------------------------------------------------------------------------------
// Draws a solid debug render cube, helper function
//
void DebugRenderSystem::DrawCube(const Vector3& position, float lifetime, const Rgba& color /*= Rgba::WHITE*/, const Vector3& dimensions /*= Vector3::ONES*/)
{
	DebugRenderOptions options;

	options.m_lifetime = lifetime;
	options.m_startColor = color;
	options.m_endColor = color;

	DrawCube(position, options, dimensions);
}


//-----------------------------------------------------------------------------------------------
// Draws a screenspace quad, base function
//
void DebugRenderSystem::Draw2DQuad(const AABB2& bounds, const DebugRenderOptions& options)
{
	DebugRenderTask_Quad2D* quad = new DebugRenderTask_Quad2D(bounds, options);
	s_instance->m_tasks.push_back(quad);
}


//-----------------------------------------------------------------------------------------------
// Draws a screenspace quad, helper function
//
void DebugRenderSystem::Draw2DQuad(const AABB2& bounds, const Rgba& color, float lifetime)
{
	DebugRenderOptions options;

	options.m_lifetime = lifetime;
	options.m_startColor = color;
	options.m_endColor = color;

	Draw2DQuad(bounds, options);
}


//-----------------------------------------------------------------------------------------------
// Draws a screenspace line, base function
//
void DebugRenderSystem::Draw2DLine(const Vector2& startPosition, const Vector2& endPosition, const DebugRenderOptions& options, const Rgba& endStartColor, const Rgba& endEndColor, float lineWidth /*= 1.0f*/)
{
	DebugRenderTask_Line2D* line = new DebugRenderTask_Line2D(startPosition, endPosition, options, endStartColor, endEndColor, lineWidth);
	s_instance->m_tasks.push_back(line);
}


//-----------------------------------------------------------------------------------------------
// Draws a screenspace line, helper function
//
void DebugRenderSystem::Draw2DLine(const Vector2& startPosition, const Vector2& endPostion, const Rgba& color, float lifeTime, float lineWidth /*= 1.0f*/)
{
	DebugRenderOptions options;

	options.m_startColor = color;
	options.m_endColor = color;
	options.m_lifetime = lifeTime;

	Draw2DLine(startPosition, endPostion, options, color, color, lineWidth);
}


//-----------------------------------------------------------------------------------------------
// Draws screenspace text, base function
//
void DebugRenderSystem::Draw2DText(const std::string& text, const AABB2& bounds, const DebugRenderOptions& options, float textHeight /*= 50.f*/)
{
	DebugRenderTask_Text2D* textTask = new DebugRenderTask_Text2D(text, bounds, options, textHeight);
	s_instance->m_tasks.push_back(textTask);
}


//-----------------------------------------------------------------------------------------------
// Draws screenspace text, helper function
//
void DebugRenderSystem::Draw2DText(const std::string& text, const AABB2& bounds, float lifetime, const Rgba& color /*= Rgba::WHITE*/, float textHeight /*= 50.f*/)
{
	DebugRenderOptions options;

	options.m_startColor = color;
	options.m_endColor = color;
	options.m_lifetime = lifetime;

	Draw2DText(text, bounds, options, textHeight);
}

//------C Functions-----


//-----------------------------------------------------------------------------------------------
// Clears the DebugRenderSystem task list
// USAGE: debug_clear
//
void Command_DebugRenderClear(Command& cmd)
{
	UNUSED(cmd);
	DebugRenderSystem::Clear();
	ConsolePrintf(Rgba::GREEN, "DebugRenderSystem task list cleared.");
}


//-----------------------------------------------------------------------------------------------
// Pauses update on all tasks
// USAGE: debug_pause
//
void Command_DebugRenderPause(Command& cmd)
{
	UNUSED(cmd);
	DebugRenderSystem::Pause();
	ConsolePrintf(Rgba::GREEN, "DebugRenderSystem update paused.");
}


//-----------------------------------------------------------------------------------------------
// Resumes update on all tasks
// USAGE: debug_resume
//
void Command_DebugRenderResume(Command& cmd)
{
	UNUSED(cmd);
	DebugRenderSystem::Resume();
	ConsolePrintf(Rgba::GREEN, "DebugRender update resumed.");
}


//-----------------------------------------------------------------------------------------------
// Enables/disables debug rendering
// USAGE: debug_show <True/False, optional>
//
void Command_DebugRenderShow(Command& cmd)
{
	bool showTasks = !DebugRenderSystem::AreTasksBeingRendered();
	bool paramSpecified = cmd.GetParam("b", showTasks);

	// No param, so just toggle
	if (!paramSpecified)
	{
		DebugRenderSystem::ToggleShowRenderTasks();
		ConsolePrintf(Rgba::GREEN, "Debug rendering toggled.");
	}
	else
	{
		DebugRenderSystem::ShowRenderTasks(showTasks);

		if (showTasks)	{ ConsolePrintf(Rgba::GREEN, "Debug rendering enabled.");  }
		else			{ ConsolePrintf(Rgba::GREEN, "Debug rendering disabled."); }	
	}
}


//-----------------------------------------------------------------------------------------------
// Creates a debug render task with the specified parameters
// USAGE: debug_cube -p <position> -d <dimensions> -sc <startcolor> -ec <endcolor> -l <lifetime> -w <true/false, for wireframe> -rm <use_depth/ignore_depth/hidden/xray>
//
void Command_DebugRenderCube(Command& cmd)
{
	// Get the parameters
	float lifetime;
	Rgba endColor;
	Rgba startColor;
	Vector3 dimensions = Vector3(1.0f, 1.0f, 1.0f);
	Vector3 position = DebugRenderSystem::GetDefaultSpawnLocation();
	std::string renderModeText = "use_depth";
	bool isWireFrame = false;

	cmd.GetParam("p", position, &position);
	cmd.GetParam("d", dimensions, &dimensions);
	cmd.GetParam("sc", startColor, &Rgba::WHITE);
	cmd.GetParam("ec", endColor, &startColor);
	cmd.GetParam("l", lifetime, &DebugRenderSystem::DEFAULT_LIFETIME);
	cmd.GetParam("rm", renderModeText, &renderModeText);
	cmd.GetParam("w", isWireFrame, &isWireFrame);

	// Push the cube
	DebugRenderOptions options;
	options.m_startColor = startColor;
	options.m_endColor = endColor;
	options.m_lifetime = lifetime;
	options.m_renderMode = DebugRenderSystem::ConvertStringToRenderMode(renderModeText);
	options.m_isWireFrame = isWireFrame;

	DebugRenderSystem::DrawCube(position, options, dimensions);
}


//-----------------------------------------------------------------------------------------------
// Creates a debug render task with the specified parameters
// USAGE: debug_point -p <position> -r <radius> -sc <startcolor> -ec <endcolor> -l <lifetime> -rm <use_depth/ignore_depth/hidden/xray>
//
void Command_DebugRenderPoint(Command& cmd)
{
	// Get the parameters
	float lifetime;
	Rgba endColor;
	Rgba startColor;
	float radius = 1.f;
	Vector3 position = DebugRenderSystem::GetDefaultSpawnLocation();
	std::string renderModeText = "use_depth";

	cmd.GetParam("p", position, &position);
	cmd.GetParam("r", radius, &radius);
	cmd.GetParam("sc", startColor, &Rgba::WHITE);
	cmd.GetParam("ec", endColor, &startColor);
	cmd.GetParam("l", lifetime, &DebugRenderSystem::DEFAULT_LIFETIME);
	cmd.GetParam("rm", renderModeText, &renderModeText);

	// Push the point
	DebugRenderOptions options;
	options.m_startColor = startColor;
	options.m_endColor = endColor;
	options.m_lifetime = lifetime;
	options.m_renderMode = DebugRenderSystem::ConvertStringToRenderMode(renderModeText);

	DebugRenderSystem::DrawPoint(position, options, radius);
}


//-----------------------------------------------------------------------------------------------
// Creates a debug render task with the specified parameters
// USAGE: debug_sphere -p <position> -r <radius> -sc <startcolor> -ec <endcolor> -l <lifetime> -ns <number of slices> -nw <number of wedges>  -w <true/false, for wireframe> -rm <use_depth/ignore_depth/hidden/xray>
//
void Command_DebugRenderSphere(Command& cmd)
{
	// Get the parameters
	float lifetime;
	Rgba endColor;
	Rgba startColor;
	float radius = 1.f;
	Vector3 position = DebugRenderSystem::GetDefaultSpawnLocation();
	std::string renderModeText = "use_depth";
	bool isWireFrame = false;

	unsigned int numSlices = 4;
	unsigned int numWedges = 8;

	cmd.GetParam("p", position, &position);
	cmd.GetParam("r", radius, &radius);
	cmd.GetParam("sc", startColor, &Rgba::WHITE);
	cmd.GetParam("ec", endColor, &startColor);
	cmd.GetParam("l", lifetime, &DebugRenderSystem::DEFAULT_LIFETIME);
	cmd.GetParam("ns", numSlices, &numSlices);
	cmd.GetParam("nw", numWedges, &numWedges);
	cmd.GetParam("rm", renderModeText, &renderModeText);
	cmd.GetParam("w", isWireFrame, &isWireFrame);

	// Push the sphere
	DebugRenderOptions options;
	options.m_startColor = startColor;
	options.m_endColor = endColor;
	options.m_lifetime = lifetime;
	options.m_renderMode = DebugRenderSystem::ConvertStringToRenderMode(renderModeText);
	options.m_isWireFrame = isWireFrame;

	DebugRenderSystem::DrawUVSphere(position, options, radius, numSlices, numWedges);
}


//-----------------------------------------------------------------------------------------------
// Creates a debug render task with the specified parameters
// USAGE: debug_3dline -sp <startposition> -ep <endposition> -w <width> -sc <startcolor> -ec <endcolor> -l <lifetime> -rm <use_depth/ignore_depth/hidden/xray>
//
void Command_DebugRender3DLine(Command& cmd)
{
	// Get the parameters
	float lifetime;
	Rgba endColor;
	Rgba startColor;
	float width = 1.f;
	Vector3 startPosition = DebugRenderSystem::GetDefaultSpawnLocation();
	Vector3 endPosition = DebugRenderSystem::GetWorldCamera()->GetPosition();
	std::string renderModeText = "use_depth";

	cmd.GetParam("sp", startPosition, &startPosition);
	cmd.GetParam("ep", endPosition, &endPosition);
	cmd.GetParam("w", width, &width);
	cmd.GetParam("sc", startColor, &Rgba::WHITE);
	cmd.GetParam("ec", endColor, &startColor);
	cmd.GetParam("l", lifetime, &DebugRenderSystem::DEFAULT_LIFETIME);
	cmd.GetParam("rm", renderModeText, &renderModeText);

	// Push the line
	DebugRenderOptions options;
	options.m_startColor = startColor;
	options.m_endColor = endColor;
	options.m_lifetime = lifetime;
	options.m_renderMode = DebugRenderSystem::ConvertStringToRenderMode(renderModeText);

	DebugRenderSystem::Draw3DLine(startPosition, endPosition, options, startColor, endColor, width);
}


//-----------------------------------------------------------------------------------------------
// Creates a debug render task with the specified parameters
// USAGE: debug_basis -p <position> -r <euler rotation> -s <scale> -l <lifetime>
//
void Command_DebugRenderBasis(Command& cmd)
{
	// Get the parameters
	float lifetime;
	float scale = 1.f;
	Vector3 position = DebugRenderSystem::GetDefaultSpawnLocation();
	Vector3 rotation = Vector3(0.f,0.f,0.f);

	cmd.GetParam("p", position, &position);
	cmd.GetParam("s", scale, &scale);
	cmd.GetParam("l", lifetime, &DebugRenderSystem::DEFAULT_LIFETIME);
	cmd.GetParam("r", rotation, &rotation);

	// Push the basis
	DebugRenderSystem::DrawBasis(position, rotation, lifetime, scale);
}


//-----------------------------------------------------------------------------------------------
// Creates a debug render task with the specified parameters
// USAGE: debug_3dquad -p <position> -d <dimensions> -r <right vector> -u <up vector> -sc <startcolor> -ec <endcolor> -l <lifetime> -w <true/false, for wireframe> -rm <use_depth/ignore_depth/hidden/xray>
//
void Command_DebugRender3DQuad(Command& cmd)
{
	// Get the parameters
	float lifetime;

	Rgba startColor;
	Rgba endColor;

	Vector2 dimensions	= Vector2(1.0f, 1.0f);
	Vector3 position	= DebugRenderSystem::GetDefaultSpawnLocation();
	
	Vector3 rightVector = Vector3::DIRECTION_RIGHT;
	Vector3 upVector	= Vector3::DIRECTION_UP;

	std::string renderModeText = "use_depth";
	bool isWireFrame = false;

	cmd.GetParam("p", position, &position);
	cmd.GetParam("d", dimensions, &dimensions);
	cmd.GetParam("l", lifetime, &DebugRenderSystem::DEFAULT_LIFETIME);
	cmd.GetParam("r", rightVector, &rightVector);
	cmd.GetParam("u", upVector, &upVector);
	cmd.GetParam("sc", startColor, &Rgba::WHITE);
	cmd.GetParam("ec", endColor, &startColor);
	cmd.GetParam("rm", renderModeText, &renderModeText);
	cmd.GetParam("w", isWireFrame, &isWireFrame);

	DebugRenderOptions options;
	options.m_startColor = startColor;
	options.m_endColor = endColor;
	options.m_lifetime = lifetime;
	options.m_renderMode = DebugRenderSystem::ConvertStringToRenderMode(renderModeText);
	options.m_isWireFrame = isWireFrame;

	// Push the basis
	DebugRenderSystem::Draw3DQuad(position, dimensions, options, rightVector, upVector);
}


//-----------------------------------------------------------------------------------------------
// Creates a debug render task with the specified parameters
// USAGE: debug_2dline -sp <startposition> -ep <endposition> -w <width> -sc <startcolor> -ec <endcolor> -l <lifetime>
//
void Command_DebugRender2DLine(Command& cmd)
{
	// Get the parameters
	float lifetime;
	Rgba endColor;
	Rgba startColor;
	float width = 1.f;
	Vector2 startPosition = Vector2::ZERO;
	Vector2 endPosition = Vector2(Window::GetInstance()->GetDimensions()) * 0.5f;

	cmd.GetParam("sp", startPosition, &startPosition);
	cmd.GetParam("ep", endPosition, &endPosition);
	cmd.GetParam("w", width, &width);
	cmd.GetParam("sc", startColor, &Rgba::WHITE);
	cmd.GetParam("ec", endColor, &startColor);
	cmd.GetParam("l", lifetime, &DebugRenderSystem::DEFAULT_LIFETIME);

	DebugRenderOptions options;
	options.m_startColor = startColor;
	options.m_endColor = endColor;
	options.m_lifetime = lifetime;

	// Push the line
	DebugRenderSystem::Draw2DLine(startPosition, endPosition, options, startColor, endColor, width);
}


//-----------------------------------------------------------------------------------------------
// Creates a debug render task with the specified parameters
// USAGE: debug_2dquad -bl <bottomleft> -tr <topright> -sc <startcolor> -ec <endcolor> -l <lifetime>
//
void Command_DebugRender2DQuad(Command& cmd)
{
	// Get the parameters
	float lifetime;
	Rgba endColor;
	Rgba startColor;
	Vector2 bottomLeft = Vector2::ZERO;
	Vector2 topRight = Vector2(Window::GetInstance()->GetDimensions()) * 0.5f;

	cmd.GetParam("bl", bottomLeft, &bottomLeft);
	cmd.GetParam("tr", topRight, &topRight);
	cmd.GetParam("sc", startColor, &Rgba::WHITE);
	cmd.GetParam("ec", endColor, &startColor);
	cmd.GetParam("l", lifetime, &DebugRenderSystem::DEFAULT_LIFETIME);

	AABB2 bounds = AABB2(bottomLeft, topRight);

	DebugRenderOptions options;
	options.m_startColor = startColor;
	options.m_endColor = endColor;
	options.m_lifetime = lifetime;

	// Push the quad
	DebugRenderSystem::Draw2DQuad(bounds, options);
}


//-----------------------------------------------------------------------------------------------
// Creates a debug render task with the specified parameters
// USAGE: debug_2dquad -bl <bottomleft> -tr <topright> -sc <startcolor> -ec <endcolor> -l <lifetime> -t <text> -h <textheight>
// Vector2 format is 0,5 
// Do not use parentheses
//
void Command_DebugRender2DText(Command& cmd)
{
	std::string text = "--No Text Specified--";
	float textHeight = 50.f;
	float lifetime;
	Rgba endColor;
	Rgba startColor;

	cmd.GetParam("t", text, &text);
	cmd.GetParam("h", textHeight, &textHeight);
	cmd.GetParam("sc", startColor, &Rgba::WHITE);
	cmd.GetParam("ec", endColor, &startColor);
	cmd.GetParam("l", lifetime, &DebugRenderSystem::DEFAULT_LIFETIME);

	// Get the positions after the cell height, to calculate bounds
	IntVector2 windowDimensions = Window::GetInstance()->GetDimensions();
	Vector2 bottomLeft = Vector2(0.f, (float) windowDimensions.y - textHeight);
	Vector2 topRight = Vector2(windowDimensions);

	cmd.GetParam("bl", bottomLeft, &bottomLeft);
	cmd.GetParam("tr", topRight, &topRight);

	AABB2 bounds = AABB2(bottomLeft, topRight);

	DebugRenderOptions options;
	options.m_startColor = startColor;
	options.m_endColor = endColor;
	options.m_lifetime = lifetime;

	// Push the text
	DebugRenderSystem::Draw2DText(text, bounds, options, textHeight);
}
