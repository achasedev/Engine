/************************************************************************/
/* Project: Game Engine
/* File: Renderer.cpp
/* Author: Andrew Chase
/* Date: September 3rd, 2017
/* Bugs: None
/* Description: Implementation of rendering functions used by Asteroids
/************************************************************************/
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"

#define WIN32_LEAN_AND_MEAN			// Always #define this before #including <windows.h>
#include <windows.h>
#include <gl/gl.h>					// Include basic OpenGL constants and function declarations
#pragma comment( lib, "opengl32" )	// Link in the OpenGL32.lib static library
#include "ThirdParty/stb/stb_image.h"

//-----------------------------------------------------------------------------------------------
// Constructor - currently does nothing, as the loaded textures map is already initialized by default
// to an empty map
//
Renderer::Renderer()
{
	
}


//-----------------------------------------------------------------------------------------------
// Destructor - free all memory allocated to Renderer pointers
//
Renderer::~Renderer()
{
	// Free the loaded texture map
	std::map<std::string, Texture*>::iterator texItr = m_loadedTextures.begin();
	for(texItr; texItr != m_loadedTextures.end(); texItr++)
	{
		delete texItr->second;
	}
	m_loadedTextures.clear();
}


//-----------------------------------------------------------------------------------------------
// Performs all Renderer procedures that occur at the beginning of the frame
//
void Renderer::BeginFrame()
{
}


//-----------------------------------------------------------------------------------------------
// Performs all Renderer procedures that occur at the end of the frame
//
void Renderer::EndFrame()
{
	// "Present" the backbuffer by swapping the front (visible) and back (working) screen buffers
	HWND activeWindow = GetActiveWindow();
	HDC displayContext = GetDC(activeWindow);
	SwapBuffers(displayContext); // Note: call this once at the end of each frame
}


//-----------------------------------------------------------------------------------------------
// Draws a line to the coordinate system give the positions and vertex colors
//
void Renderer::DrawLine(const Vector2& startPos, const Vector2& endPos)
{
	glBegin( GL_LINES );

	glVertex2f(startPos.x, startPos.y);
	glVertex2f(endPos.x, endPos.y);

	glEnd();
}


//-----------------------------------------------------------------------------------------------
// Draws a line to the coordinate system give the positions and vertex colors
//
void Renderer::DrawLineColor(const Vector2& startPos, const Rgba& startColor, const Vector2& endPos, const Rgba& endColor)
{
	glBegin( GL_LINES );

	SetDrawColor(startColor);
	glVertex2f(startPos.x, startPos.y);

	SetDrawColor(endColor);
	glVertex2f(endPos.x, endPos.y);

	// Set the color back to white (default)
	SetDrawColor(Rgba::WHITE);
	glEnd();
}


//-----------------------------------------------------------------------------------------------
// Draws a regular polygon to the coordinate system
//
void Renderer::DrawRegularPolygon(const Vector2& centerPos, int numSides, float orientationOffset, float radius) 
{
	// Draw numSides number of lines to draw the polygon
	for (int i = 0; i < numSides; i++) 
	{
		// Calculate start and end degree bounds for this side
		float degreesPerSide = (360.f / numSides);
		float startDegrees = (i * degreesPerSide);
		float endDegrees = (startDegrees + degreesPerSide);

		// calculate the start and end components for the two endpoints of this line
		float startX = centerPos.x + (radius * CosDegrees((startDegrees + orientationOffset)));
		float startY = centerPos.y + (radius * SinDegrees((startDegrees + orientationOffset)));
		float endX = centerPos.x + (radius * CosDegrees((endDegrees + orientationOffset)));
		float endY = centerPos.y + (radius * SinDegrees((endDegrees + orientationOffset)));

		// Draw a line between these two points
		DrawLineColor(Vector2(startX, startY), Rgba::WHITE, Vector2(endX, endY), Rgba::WHITE);
	}
}


//-----------------------------------------------------------------------------------------------
// Draws a filled quad given the bounds as an AABB2
//
void Renderer::DrawAABB2(const AABB2& boundsToDraw, const Rgba& color)
{
	SetDrawColor(color);
	glBegin(GL_QUADS);

	// Bottom left
	glVertex2f(boundsToDraw.mins.x, boundsToDraw.mins.y);

	// Bottom right
	glVertex2f(boundsToDraw.maxs.x, boundsToDraw.mins.y);

	// Top right
	glVertex2f(boundsToDraw.maxs.x, boundsToDraw.maxs.y);

	// Top left
	glVertex2f(boundsToDraw.mins.x, boundsToDraw.maxs.y);

	glEnd();

	// Set color back to white
	SetDrawColor(Rgba::WHITE);
}

//-----------------------------------------------------------------------------------------------
// Draws a dotted circle to the coordinate system
//
void Renderer::DrawDottedCircle(const Vector2& centerPos, float radius)
{
	// Draw numSides number of lines to draw the polygon
	for (int i = 0; i < 50; i++) 
	{
		if ((i % 2) != 0)
		{
			continue;
		}
		// Calculate start and end degree bounds for this side
		float degreesPerSide = (360.f / 50.f);
		float startDegrees = (i * degreesPerSide);
		float endDegrees = (startDegrees + degreesPerSide);

		// calculate the start and end components for the two endpoints of this line
		float startX = centerPos.x + (radius * CosDegrees(startDegrees));
		float startY = centerPos.y + (radius * SinDegrees(startDegrees));
		float endX = centerPos.x + (radius * CosDegrees(endDegrees));
		float endY = centerPos.y + (radius * SinDegrees(endDegrees));

		// Draw a line between these two points
		DrawLine(Vector2(startX, startY), Vector2(endX, endY));
	}
}


//-----------------------------------------------------------------------------------------------
// Changes the draw color for future draws
//
void Renderer::SetDrawColor(const Rgba& newColor)
{
	glColor4ub(newColor.r, newColor.g, newColor.b, newColor.a);
}


//-----------------------------------------------------------------------------------------------
// Clears the back buffer to a solid color
//
void Renderer::ClearScreen(const Rgba& clearColor)
{
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT);
}


//-----------------------------------------------------------------------------------------------
// Sets the coordinate system of the canvas to a simple XY-plane with x-values ranging from
// 0-maxXValue and y-values ranging from 0-MaxYValue
//
void Renderer::SetOrtho(const AABB2& worldBounds)
{
	glLoadIdentity();
	glOrtho(worldBounds.mins.x, worldBounds.maxs.x, worldBounds.mins.y, worldBounds.maxs.y, 0.f, 1.f );
}


//-----------------------------------------------------------------------------------------------
// Pushes a new transformation matrix on top of the matrix stack
//
void Renderer::PushMatrix()
{
	glPushMatrix();
}


//-----------------------------------------------------------------------------------------------
// Adds a translation to the top transformation matrix, so the coordinate system is now at
// (xPosition, yPosition, zPosition)
//
void Renderer::TranslateCoordinateSystem2D(float xPosition, float yPosition)
{
	glTranslatef(xPosition, yPosition, 0.f);
}


//-----------------------------------------------------------------------------------------------
// Moves the coordinate system over by translationVector
//
void Renderer::TranslateCoordinateSystem2D(Vector2 translationVector)
{
	glTranslatef(translationVector.x, translationVector.y, 0.f);
}


//-----------------------------------------------------------------------------------------------
// Rotates the coordinate system about the z-axis (for 2D rotations) by orientationAngle
//
void Renderer::RotateCoordinateSystem2D(float orientationAngle)
{
	glRotatef(orientationAngle, 0.f, 0.f, 1.0f);
}


//-----------------------------------------------------------------------------------------------
// Scales the coordinate system uniformly by scaleFactor
//
void Renderer::ScaleCoordinateSystem(float scaleFactor)
{
	glScalef(scaleFactor, scaleFactor, 1.0f);
}


//-----------------------------------------------------------------------------------------------
// Pops the top matrix of the transformation matrix stack
//
void Renderer::PopMatrix()
{
	glPopMatrix();
}


//-----------------------------------------------------------------------------------------------
// Sets the draw line width to the specified 'lineWidth'
//
void Renderer::SetLineWidth(float lineWidth)
{
	glLineWidth(lineWidth);
}


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
// Enables the OpenGL macro GL_LINE_SMOOTH
//
void Renderer::EnableSmoothLine()
{
	glEnable(GL_LINE_SMOOTH);
}


//-----------------------------------------------------------------------------------------------
// Returns a pointer to the texture pointer corresponding to the given path.
// If no texture has been loaded for the given path yet, the texture is loaded and returned.
//
Texture* Renderer::CreateOrGetTexture(std::string texturePath)
{
	if (m_loadedTextures[texturePath] != nullptr)
	{
		return m_loadedTextures[texturePath];
	}

	m_loadedTextures[texturePath] = new Texture(texturePath);
	
	// ToDo: Put guarantee check here for the texture file being found

	return m_loadedTextures[texturePath];
}


//-----------------------------------------------------------------------------------------------
// Draws a textured AABB2 from the provided texture data
// Texture coordinates are the coordinates of the texture at the min and max of bounds (bottom left
// and top right)
//
void Renderer::DrawTexturedAABB2(const AABB2& bounds, const Texture& texture, const Vector2& texCoordAtMins, const Vector2& texCoordAtMaxs, const Rgba& tint)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture.m_textureID);

	SetDrawColor(tint);

	glBegin(GL_QUADS);
	{
		// Bottom Left
		glTexCoord2f(texCoordAtMins.x, texCoordAtMins.y);
		glVertex2f(bounds.mins.x, bounds.mins.y);

		// Bottom right
		glTexCoord2f(texCoordAtMaxs.x, texCoordAtMins.y);
		glVertex2f(bounds.maxs.x, bounds.mins.y);

		// Top Right
		glTexCoord2f(texCoordAtMaxs.x, texCoordAtMaxs.y);
		glVertex2f(bounds.maxs.x, bounds.maxs.y);

		// Top Left
		glTexCoord2f(texCoordAtMins.x, texCoordAtMaxs.y);
		glVertex2f(bounds.mins.x, bounds.maxs.y);
	}
	glEnd();

	SetDrawColor(Rgba::WHITE);

	glDisable(GL_TEXTURE_2D);
}
