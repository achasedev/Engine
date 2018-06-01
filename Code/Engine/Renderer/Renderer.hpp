/************************************************************************/
/* Project: Game Engine
/* File: Renderer.hpp
/* Author: Andrew Chase
/* Date: September 3rd, 2017
/* Bugs: None
/* Description: Class used to use OpenGL functions to draw to screen
/************************************************************************/
#pragma once
#include <string>
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/AABB2.hpp"

enum BlendMode
{
	BLEND_MODE_ERROR = -1,
	BLEND_MODE_ALPHA,
	BLEND_MODE_ADDITIVE,
	NUM_BLEND_MODES
};

class Renderer
{
public:

	Renderer();
	~Renderer();

	// Updating/Rendering
	void BeginFrame();	// Sets up the frame for rendering
	void EndFrame();	// Finishes render processes and swaps the buffers


	// Simple drawing
	void DrawLine(const Vector2& startPos, const Vector2& endPos);														// Draws a straight line between two points
	void DrawRegularPolygon(const Vector2& centerPos, int numSides, float orientationOffset, float radius);				// Draws a regular polygon
	void DrawAABB2(const AABB2& boundsToDraw, const Rgba& color);														// Draws a filled quad given the bounds as an AABB2
	void DrawDottedCircle(const Vector2& centerPos, float radius);														// Draws a dotted circle
	void DrawLineColor(const Vector2& startPos, const Rgba& startColor, const Vector2& endPos, const Rgba& endColor);	// Draws a colored straight line between two points


	// Changing GL states
	void SetDrawColor(const Rgba& newColor);								// Changes the draw color for future draws
	void ClearScreen(const Rgba& clearColor);								// Clears the canvas to a single color	
	void SetOrtho(const AABB2& worldBounds);								// Sets the drawing coordinate system to an XY-plane defined by worldBounds
	void PushMatrix();														// Pushes a new transformation matrix onto the OpenGL matrix stack
	void TranslateCoordinateSystem2D(float xPosition, float yPosition);		// Move the coordinate system over by xPosition and yPosition
	void TranslateCoordinateSystem2D(Vector2 translationVector);			// Moves the coordinate system over by translationVector
	void RotateCoordinateSystem2D(float orientationAngle);					// Rotate the coordinate system about the z-axis by orientationAngle	
	void ScaleCoordinateSystem(float scaleFactor);							// Scales the coordinate system by scaleFactor
	void PopMatrix();														// Removes and returns the matrix at the top of the OpenGL matrix stack
	void EnableBlendMacro();												// Enables the gl macro	
	void EnableSmoothLine();												// Enables the smooth line macro	
	void SetLineWidth(float lineWidth);										// Sets the draw line width
	void SetBlendMode(BlendMode nextMode);									// Sets the blending function for drawing

	// Texturing
	Texture* CreateOrGetTexture(std::string texturePath);																								// Returns the existing texture stored, or creates one if it doesn't exist
	void DrawTexturedAABB2(const AABB2& bounds, const Texture& texture, const Vector2& texCoordMins, const Vector2& texCoordMaxs, const Rgba& tint);	// Draws a textured AABB2 with the given texture information

public:

private:

private:

	std::map<std::string, Texture*> m_loadedTextures;					// Textures currently loaded and being used by this renderer
};
