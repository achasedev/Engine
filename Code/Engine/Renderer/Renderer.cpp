/************************************************************************/
/* Project: Game Engine
/* File: Renderer.cpp
/* Author: Andrew Chase
/* Date: September 3rd, 2017
/* Bugs: None
/* Description: Implementation of the Renderer class
/************************************************************************/
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


#define WIN32_LEAN_AND_MEAN			// Always #define this before #including <windows.h>
#include <windows.h>
#include <gl/gl.h>					// Include basic OpenGL constants and function declarations
#pragma comment( lib, "opengl32" )	// Link in the OpenGL32.lib static library
#include "ThirdParty/stb/stb_image.h"


const IntVector2 Renderer::FONT_SPRITE_LAYOUT = IntVector2(16, 16);
const char* Renderer::FONT_DIRECTORY = "Data/Fonts/";
const std::string Renderer::DEFAULT_FONT = std::string("SquirrelFixedFont");

int g_openGlPrimitiveTypes[ NUM_PRIMITIVE_TYPES ] =
{
	GL_POINTS,			// called PRIMITIVE_POINTS		in our engine
	GL_LINES,			// called PRIMITIVE_LINES		in our engine
	GL_TRIANGLES,		// called PRIMITIVE_TRIANGES	in our engine
	GL_QUADS			// called PRIMITIVE_QUADS		in our engine
};

//-----------------------------------------------------------------------------------------------
// Constructor - currently does nothing, as the loaded textures map is already initialized by default
// to an empty map
//
Renderer::Renderer()
	: m_defaultFont(CreateOrGetBitmapFont(DEFAULT_FONT.c_str()))
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

	// Free the loaded fonts
	std::map<std::string, BitmapFont*>::iterator fontItr = m_loadedFonts.begin();
	for (fontItr; fontItr != m_loadedFonts.end(); fontItr++)
	{
		delete fontItr->second;
	}
	m_loadedFonts.clear();
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
	Vertex3D_PCU vertex[2];

	vertex[0].m_position.x = startPos.x;
	vertex[0].m_position.y = startPos.y;

	vertex[1].m_position.x = endPos.x;
	vertex[1].m_position.y = endPos.y;

	DrawMeshImmediate(vertex, 2, PRIMITIVE_LINES);
}


//-----------------------------------------------------------------------------------------------
// Draws a line to the coordinate system give the positions and vertex colors
//
void Renderer::DrawLineColor(const Vector2& startPos, const Rgba& startColor, const Vector2& endPos, const Rgba& endColor)
{
	Vertex3D_PCU vertex[2];

	vertex[0].m_position.x = startPos.x;
	vertex[0].m_position.y = startPos.y;
	vertex[0].m_color = startColor;

	vertex[1].m_position.x = endPos.x;
	vertex[1].m_position.y = endPos.y;
	vertex[1].m_color = endColor;

	DrawMeshImmediate(vertex, 2, PRIMITIVE_LINES);
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
	// Set up the vertices
	Vertex3D_PCU vertex[4];

	// Bottom left
	vertex[0].m_position.x = boundsToDraw.mins.x;
	vertex[0].m_position.y = boundsToDraw.mins.y;
	vertex[0].m_position.z = 0.f;
	vertex[0].m_color = color;

	// Bottom right
	vertex[1].m_position.x = boundsToDraw.maxs.x;
	vertex[1].m_position.y = boundsToDraw.mins.y;
	vertex[1].m_position.z = 0.f;
	vertex[1].m_color = color;

	// Top right
	vertex[2].m_position.x = boundsToDraw.maxs.x;
	vertex[2].m_position.y = boundsToDraw.maxs.y;
	vertex[2].m_position.z = 0.f;
	vertex[2].m_color = color;

	// Top left
	vertex[3].m_position.x = boundsToDraw.mins.x;
	vertex[3].m_position.y = boundsToDraw.maxs.y;
	vertex[3].m_position.z = 0.f;
	vertex[3].m_color = color;

	DrawMeshImmediate(vertex, 4, PRIMITIVE_QUADS);
}


//-----------------------------------------------------------------------------------------------
// Draws a dotted circle to the coordinate system
//
void Renderer::DrawDottedCircle(const Vector2& centerPos, float radius, const Rgba& color)
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
		DrawLineColor(Vector2(startX, startY), color, Vector2(endX, endY), color);
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
// Enables 2D texturing
//
void Renderer::Enable2DTexture()
{
	glEnable(GL_TEXTURE_2D);
}


//-----------------------------------------------------------------------------------------------
// Binds the 2D texture given by textureID
//
void Renderer::Bind2DTexture(const Texture& texture)
{
	glBindTexture(GL_TEXTURE_2D, texture.m_textureID);
}


//-----------------------------------------------------------------------------------------------
// Disables 2D texturing
//
void Renderer::Disable2DTexture()
{
	glDisable(GL_TEXTURE_2D);
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
	if (m_loadedTextures.find(texturePath) != m_loadedTextures.end())
	{
		return m_loadedTextures[texturePath];
	}

	m_loadedTextures[texturePath] = new Texture(texturePath);

	return m_loadedTextures[texturePath];
}


//-----------------------------------------------------------------------------------------------
// Draws a textured AABB2 from the provided texture data
// textureUVs are the texture coordinates of the top left and bottom right of the box
//
void Renderer::DrawTexturedAABB2(const AABB2& bounds, const Texture& texture, const AABB2& textureUVs, const Rgba& tint)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture.m_textureID);

	// For readability
	Vector2 topLeftUVs = textureUVs.mins;
	Vector2 bottomRightUVs = textureUVs.maxs;

	//-----Set up the vertices-----
	Vertex3D_PCU vertex[4];

	// Bottom left
	vertex[0].m_position.x = bounds.mins.x;
	vertex[0].m_position.y = bounds.mins.y;
	vertex[0].m_position.z = 0.f;
	vertex[0].m_color = tint;
	vertex[0].m_texUVs.x = topLeftUVs.x;
	vertex[0].m_texUVs.y = bottomRightUVs.y;

	// Bottom right
	vertex[1].m_position.x = bounds.maxs.x;
	vertex[1].m_position.y = bounds.mins.y;
	vertex[1].m_position.z = 0.f;
	vertex[1].m_color = tint;
	vertex[1].m_texUVs.x = bottomRightUVs.x;
	vertex[1].m_texUVs.y = bottomRightUVs.y;

	// Top right
	vertex[2].m_position.x = bounds.maxs.x;
	vertex[2].m_position.y = bounds.maxs.y;
	vertex[2].m_position.z = 0.f;
	vertex[2].m_color = tint;
	vertex[2].m_texUVs.x = bottomRightUVs.x;
	vertex[2].m_texUVs.y = topLeftUVs.y;


	// Top left
	vertex[3].m_position.x = bounds.mins.x;
	vertex[3].m_position.y = bounds.maxs.y;
	vertex[3].m_position.z = 0.f;
	vertex[3].m_color = tint;
	vertex[3].m_texUVs.x = topLeftUVs.x;
	vertex[3].m_texUVs.y = topLeftUVs.y;

	//-----Vertices set up-----

	// Draw the AABB2
	DrawMeshImmediate(vertex, 4, PRIMITIVE_QUADS);

	glDisable(GL_TEXTURE_2D);
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
	SpriteSheet* glyphSheet = new SpriteSheet(*fontTexture, FONT_SPRITE_LAYOUT);
	m_loadedFonts[bitmapFontName] = new BitmapFont(*glyphSheet, 1.0f);

	return m_loadedFonts[bitmapFontName];
}


//-----------------------------------------------------------------------------------------------
// Draws text to the screen as a textured AABB2
//
void Renderer::DrawText2D(const std::string& text, const Vector2& drawMins, float cellHeight, BitmapFont* font/*=nullptr*/, Rgba color/*=Rgba::WHITE*/, float aspectScale/*=1.0f*/)
{
	

	if (font == nullptr)
	{
		font = m_defaultFont;
	}
	
	std::vector<std::string> textLines = Tokenize(text, '\n');

	// Draw each line
	for (int lineNumber = 0; lineNumber < static_cast<int>(textLines.size()); lineNumber++)
	{
		Vector2 glyphBottomLeft = Vector2(drawMins.x, (drawMins.y - lineNumber * cellHeight));

		std::string currLine = textLines[lineNumber];

		// Draw each letter of the current line
		for (int charIndex = 0; charIndex < static_cast<int>(currLine.length()); charIndex++)
		{
			char currentChar = currLine[charIndex];
			float glyphWidth = (font->GetGlyphAspect() * cellHeight) * aspectScale;
			Vector2 glyphTopRight = glyphBottomLeft + Vector2(glyphWidth, cellHeight);

			AABB2 drawBounds = AABB2(glyphBottomLeft, glyphTopRight);
			AABB2 glyphUVs = font->GetGlyphUVs(currentChar);
			DrawTexturedAABB2(drawBounds, font->m_spriteSheet.GetTexture(), glyphUVs, color);

			// Increment the next bottom left position
			glyphBottomLeft += Vector2(glyphWidth, 0.f);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the given text in the box using the alignment and draw mode settings
//
void Renderer::DrawTextInBox2D(const std::string& text, const AABB2& drawBox, const Vector2& alignment, float cellHeight, TextDrawMode drawMode, BitmapFont* font/*=nullptr*/, Rgba color/*=Rgba::WHITE*/, float aspectScale/*=1.0f*/)
{
	if (font == nullptr)
	{
		font = m_defaultFont;
	}

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
// Draws the given text in the box in overrun style
//
void Renderer::DrawTextInBox2D_Overrun(const std::string& text, const AABB2& drawBox, const Vector2& alignment, float cellHeight, BitmapFont* font/*=nullptr*/, Rgba color/*=Rgba::WHITE*/, float aspectScale/*=1.0f*/)
{
	if (font == nullptr)
	{
		font = m_defaultFont;
	}

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
void Renderer::DrawTextInBox2D_ShrinkToFit(const std::string& text, const AABB2& drawBox, const Vector2& alignment, float cellHeight, BitmapFont* font/*=nullptr*/, Rgba color/*=Rgba::WHITE*/, float aspectScale/*=1.0f*/)
{
	if (font == nullptr)
	{
		font = m_defaultFont;
	}

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
void Renderer::DrawTextInBox2D_WordWrap(const std::string& text, const AABB2& drawBox, const Vector2& alignment, float cellHeight, BitmapFont* font/*=nullptr*/, Rgba color/*=Rgba::WHITE*/, float aspectScale/*=1.0f*/)
{
	// Use default font if one isn't provided
	if (font == nullptr)
	{
		font = m_defaultFont;
	}

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
// Draws to the screen given the vertices and the draw primitive type
//
void Renderer::DrawMeshImmediate( const Vertex3D_PCU* vertexes, int numVertexes, PrimitiveType primitiveType )
{
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_COLOR_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	glVertexPointer(	3, GL_FLOAT,			sizeof( Vertex3D_PCU ), &vertexes[ 0 ].m_position );
	glColorPointer(		4, GL_UNSIGNED_BYTE,	sizeof( Vertex3D_PCU ), &vertexes[ 0 ].m_color );
	glTexCoordPointer(	2, GL_FLOAT,			sizeof( Vertex3D_PCU ), &vertexes[ 0 ].m_texUVs );

	GLenum glPrimitiveType = g_openGlPrimitiveTypes[ primitiveType ];
	glDrawArrays( glPrimitiveType, 0, numVertexes );

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
}
