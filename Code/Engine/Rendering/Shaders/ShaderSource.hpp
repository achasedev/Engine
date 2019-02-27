/************************************************************************/
/* File: ShaderSource.hpp
/* Author: Andrew Chase
/* Date: February 23rd, 2018
/* Description: Source file for built-in shader code
				*Should only be included by ShaderProgram.cpp or Renderer.cpp*
/************************************************************************/
#pragma once
#include "Engine/Rendering/Shaders/Shader.hpp"

namespace ShaderSource
{
	//-----Default sorting orders for all shaders--------
	extern const unsigned int DEFAULT_OPAQUE_LAYER;
	extern const unsigned int DEFAULT_ALPHA_LAYER;
	extern const SortingQueue DEFAULT_OPAQUE_QUEUE;
	extern const SortingQueue DEFAULT_ALPHA_QUEUE;
	//---------------------------------------------------


	// Default Opaque Shader
	extern const char* DEFAULT_OPAQUE_NAME;
	extern const char* DEFAULT_OPAQUE_VS;
	extern const char* DEFAULT_OPAQUE_FS;
	extern const RenderState DEFAULT_OPAQUE_STATE;

	// Default Alpha Shader
	extern const char* DEFAULT_ALPHA_NAME;
	extern const char* DEFAULT_ALPHA_VS;
	extern const char* DEFAULT_ALPHA_FS;
	extern const RenderState DEFAULT_ALPHA_STATE;

	// Default Opaque Shader Instanced
	extern const char* DEFAULT_OPAQUE_INSTANCED_NAME;
	extern const char* DEFAULT_OPAQUE_INSTANCED_VS;
	extern const char* DEFAULT_OPAQUE_INSTANCED_FS;
	extern const RenderState DEFAULT_OPAQUE_INSTANCED_STATE;

	// Default Alpha Shader Instanced
	extern const char* DEFAULT_ALPHA_INSTANCED_NAME;
	extern const char* DEFAULT_ALPHA_INSTANCED_VS;
	extern const char* DEFAULT_ALPHA_INSTANCED_FS;
	extern const RenderState DEFAULT_ALPHA_INSTANCED_STATE;

	// Skybox Shader
	extern const char* SKYBOX_SHADER_NAME;
	extern const char* SKYBOX_SHADER_VS;
	extern const char* SKYBOX_SHADER_FS;
	extern const RenderState SKYBOX_SHADER_STATE;

	// UI Shader
	extern const char* UI_SHADER_NAME;
	extern const char* UI_SHADER_VS;
	extern const char* UI_SHADER_FS;
	extern const RenderState UI_SHADER_STATE;


	// Invalid Shader (SHOULD ALWAYS COMPILE)
	extern const char* INVALID_SHADER_NAME;
	extern const char* INVALID_VS;	
	extern const char* INVALID_FS;
	extern const RenderState INVALID_RENDER_STATE;


	// Debug Render System Shader, for color tinting over time
	extern const char* DEBUG_RENDER_NAME;
	extern const char* DEBUG_RENDER_VS;											 
	extern const char* DEBUG_RENDER_FS;
	extern const RenderState DEBUG_RENDER_STATE;

	// Xray shader, for xray draws
	extern const char* XRAY_SHADER_NAME;
	extern const char* XRAY_SHADER_VS;
	extern const char* XRAY_SHADER_FS;
	extern const RenderState XRAY_SHADER_STATE;

	// Phong Opaque Shader
	extern const char* PHONG_OPAQUE_NAME;
	extern const char* PHONG_OPAQUE_VS;											 
	extern const char* PHONG_OPAQUE_FS;
	extern const RenderState PHONG_OPAQUE_STATE;


	// Phong Alpha Shader
	extern const char* PHONG_ALPHA_NAME;
	extern const char* PHONG_ALPHA_VS;											 
	extern const char* PHONG_ALPHA_FS;
	extern const RenderState PHONG_ALPHA_STATE;

	// Phong Opaque Instanced Shader
	extern const char* PHONG_OPAQUE_INSTANCED_NAME;
	extern const char* PHONG_OPAQUE_INSTANCED_VS;											 
	extern const char* PHONG_OPAQUE_INSTANCED_FS;
	extern const RenderState PHONG_OPAQUE_INSTANCED_STATE;

	// Phong Alpha Instanced Shader
	extern const char* PHONG_ALPHA_INSTANCED_NAME;
	extern const char* PHONG_ALPHA_INSTANCED_VS;											 
	extern const char* PHONG_ALPHA_INSTANCED_FS;
	extern const RenderState PHONG_ALPHA_INSTANCED_STATE;


	// Vertex Normal
	extern const char* VERTEX_NORMAL_NAME;
	extern const char* VERTEX_NORMAL_VS;											 
	extern const char* VERTEX_NORMAL_FS;
	extern const RenderState VERTEX_NORMAL_STATE;


	// Vertex Tangent
	extern const char* VERTEX_TANGENT_NAME;
	extern const char* VERTEX_TANGENT_VS;											 
	extern const char* VERTEX_TANGENT_FS;
	extern const RenderState VERTEX_TANGENT_STATE;


	// Vertex Bitangent
	extern const char* VERTEX_BITANGENT_NAME;
	extern const char* VERTEX_BITANGENT_VS;											 
	extern const char* VERTEX_BITANGENT_FS;
	extern const RenderState VERTEX_BITANGENT_STATE;


	// Surface Normal
	extern const char* SURFACE_NORMAL_NAME;
	extern const char* SURFACE_NORMAL_VS;											 
	extern const char* SURFACE_NORMAL_FS;
	extern const RenderState SURFACE_NORMAL_STATE;


	// World Normal
	extern const char* WORLD_NORMAL_NAME;
	extern const char* WORLD_NORMAL_VS;											 
	extern const char* WORLD_NORMAL_FS;
	extern const RenderState WORLD_NORMAL_STATE;


	// Ambient + Diffuse Only
	extern const char* DIFFUSE_NAME;
	extern const char* DIFFUSE_VS;											 
	extern const char* DIFFUSE_FS;
	extern const RenderState DIFFUSE_STATE;


	// Specular Only
	extern const char* SPECULAR_NAME;
	extern const char* SPECULAR_VS;											 
	extern const char* SPECULAR_FS;
	extern const RenderState SPECULAR_STATE;


	// Ambient + Diffuse + Specular Only (no color)
	extern const char* LIGHTING_NAME;
	extern const char* LIGHTING_VS;											 
	extern const char* LIGHTING_FS;
	extern const RenderState LIGHTING_STATE;


	// UV
	extern const char* UV_NAME;
	extern const char* UV_VS;											 
	extern const char* UV_FS;
	extern const RenderState UV_STATE;

};
