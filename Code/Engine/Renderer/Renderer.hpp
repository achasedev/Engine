/************************************************************************/
/* Project: Game Engine
/* File: Renderer.hpp
/* Author: Andrew Chase
/* Date: September 3rd, 2017
/* Bugs: None
/* Description: Class used to call OpenGL functions to draw to screen
/************************************************************************/
#pragma once
#include <string>
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/BitmapFont.hpp"


// Vertex structure
struct Vertex3D_PCU
{
	Vector3 m_position;	// Position of the Vertex
	Rgba m_color;		// Color of the Vertex
	Vector2 m_texUVs;	// Texture UV coordinates for this vertex
};


enum BlendMode
{
	BLEND_MODE_ERROR = -1,
	BLEND_MODE_ALPHA,
	BLEND_MODE_ADDITIVE,
	NUM_BLEND_MODES
};


enum PrimitiveType
{
	PRIMITIVE_POINTS,		// in OpenGL, for example, this becomes GL_POINTS
	PRIMITIVE_LINES,		// in OpenGL, for example, this becomes GL_LINES
	PRIMITIVE_TRIANGLES,		// in OpenGL, for example, this becomes GL_TRIANGLES
	PRIMITIVE_QUADS,		// in OpenGL, for example, this becomes GL_QUADS
	NUM_PRIMITIVE_TYPES
};

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
	//-----Public Methods-----

	Renderer();
	~Renderer();

	// Updating/Rendering
	void BeginFrame();	// Sets up the frame for rendering
	void EndFrame();	// Finishes render processes and swaps the buffers


	// Simple drawing
	void DrawLine(const Vector2& startPos, const Vector2& endPos);														// Draws a straight line between two points
	void DrawRegularPolygon(const Vector2& centerPos, int numSides, float orientationOffset, float radius);				// Draws a regular polygon
	void DrawAABB2(const AABB2& boundsToDraw, const Rgba& color);														// Draws a filled quad given the bounds as an AABB2
	void DrawDottedCircle(const Vector2& centerPos, float radius, const Rgba& color);									// Draws a dotted circle
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

	void Enable2DTexture();
	void Bind2DTexture(const Texture& texture);
	void Disable2DTexture();

	// Texturing
	Texture* CreateOrGetTexture(std::string texturePath);																								// Returns the existing texture stored, or creates one if it doesn't exist
	void DrawTexturedAABB2(const AABB2& bounds, const Texture& texture, const AABB2& textureUVs, const Rgba& tint);										// Draws a textured AABB2 with the given texture information

	// Text
	BitmapFont* CreateOrGetBitmapFont(const char* bitmapFontName);
	void DrawText2D(const std::string& text, const Vector2& drawMins, float cellHeight, BitmapFont* font=nullptr, Rgba color=Rgba::WHITE, float aspectScale=1.0f);
	void DrawTextInBox2D(const std::string& text, const AABB2& box, const Vector2& alignment, float cellHeight, TextDrawMode drawMode, BitmapFont* font=nullptr, Rgba color=Rgba::WHITE, float aspectScale=1.0f);


	// For ALL drawing - will be updated frequently
	void DrawMeshImmediate(const Vertex3D_PCU* verts, int numVerts, PrimitiveType drawPrimitive);

	
private:
	//-----Private Methods-----

	// DrawTextInBox2D helper functions
	void DrawTextInBox2D_Overrun(const std::string& text, const AABB2& box, const Vector2& alignment, float cellHeight, BitmapFont* font=nullptr, Rgba color=Rgba::WHITE, float aspectScale=1.0f);
	void DrawTextInBox2D_ShrinkToFit(const std::string& text, const AABB2& box, const Vector2& alignment, float cellHeight, BitmapFont* font=nullptr, Rgba color=Rgba::WHITE, float aspectScale=1.0f);
	void DrawTextInBox2D_WordWrap(const std::string& text, const AABB2& box, const Vector2& alignment, float cellHeight, BitmapFont* font=nullptr, Rgba color=Rgba::WHITE, float aspectScale=1.0f);


private:
	//-----Private Data-----

	std::map<std::string, Texture*> m_loadedTextures;				// Textures currently loaded and being used by this renderer
	std::map<std::string, BitmapFont*> m_loadedFonts;				// Fonts currently loaded and being drawn by this renderer
	BitmapFont* m_defaultFont;										// Default font used by this renderer

	const static IntVector2 FONT_SPRITE_LAYOUT;
	const static char* FONT_DIRECTORY;
	const static std::string DEFAULT_FONT;
};
