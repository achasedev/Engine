/************************************************************************/
/* File: ShaderSource.hpp
/* Author: Andrew Chase
/* Date: February 23rd, 2018
/* Description: Source file for built-in shader code
				*Should only be included by ShaderProgram.cpp*
/************************************************************************/
#pragma once

//-----------------------------------------------------------------------------------------------
// Default vertex shader - applies a view and projection matrix to the point, moving from world to clip space
//
static const char* DEFAULT_VS = R"(

#version 420 core												

uniform mat4 MODEL;																	
uniform mat4 PROJECTION;										
uniform mat4 VIEW;												
in vec3 POSITION;												
in vec4 COLOR;													
in vec2 UV;														
																
out vec2 passUV;												
out vec4 passColor;												
																													
void main( void )												
{																										
	vec4 world_pos = vec4( POSITION, 1 ); 						
	vec4 clip_pos = PROJECTION * VIEW * MODEL * world_pos; 				
																
	passUV = UV;												
	passColor = COLOR;											
	gl_Position = clip_pos; 
									
})";											 

//-----------------------------------------------------------------------------------------------
// Default fragment shader- samples a single texture and blends in a color tint
//
static const char* DEFAULT_FS = R"(

#version 420 core											
																										
in vec2 passUV;												
in vec4 passColor;											
															  										
layout(binding = 0) uniform sampler2D gTexDiffuse;			
																											
out vec4 outColor; 											
															
// Entry Point												
void main( void )											
{																																			
	vec4 diffuse = texture(gTexDiffuse, passUV);	
	outColor = diffuse * passColor;	 				
})";


//-----------------------------------------------------------------------------------------------
// Invalid - just a passthrough vs
//
static const char* INVALID_VS = R"(

#version 420 core								 
												 
in vec3 POSITION;								 
												 						 
void main( void )								 
{												 
   // Pass through								 
   gl_Position = vec4( POSITION, 1 ); 
			 
})";	


//-----------------------------------------------------------------------------------------------
// Invalid - sets all color to magenta
//
static const char* INVALID_FS = R"(

#version 420 core								
																					
out vec4 outColor; 								
																					
void main( void )								
{												
	// Always output magenta					
	outColor = vec4( 1, 0, 1, 1 ); 		
})";
