/************************************************************************/
/* File: ShaderProgram.hpp
/* Author: Andrew Chase
/* Date: January 25th, 2018
/* Bugs: None
/* Description: Class to represent a shader program (vertex/fragment shader stages)
/*				*For now, should only be included by Renderer.cpp*
/************************************************************************/
#pragma once
#include <map>
#include <string>

class ShaderProgram
{
public:
	//-----Public Methods-----

	// Do-nothing constructor
	ShaderProgram();

	// Destructor - free the program memory on the gpu
	~ShaderProgram();

	// Loads the shaders given by rootName, compiles them, and links them to this program
	bool LoadProgramFromFiles(const char *rootName);

	// Loads the shaders from the string literal source code provided, and links them to this program
	bool LoadProgramFromSources(const char *vertexShaderSource, const char* fragmentShaderSource);

	// Accessors
	GLuint GetHandle() const;
	const std::string& GetSourceFileName() const;


public:
	//-----Public Data-----

	// Built-in shader names
	const static std::string INVALID_SHADER_NAME;
	const static std::string DEFAULT_SHADER_NAME;


private:
	//-----Private Data-----

	// The handle identifying this program on the GPU
	GLuint m_programHandle = NULL;

	// Source filename(s) of this program, empty string denotes this was a built-in shader
	std::string m_sourceFilename;	

};
