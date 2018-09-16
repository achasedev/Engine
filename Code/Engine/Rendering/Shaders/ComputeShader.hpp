/************************************************************************/
/* File: ComputeShader.hpp
/* Author: Andrew Chase
/* Date: September 15th, 2018
/* Description: Class to represent an OpenGL compute shader
/************************************************************************/
#pragma once

class ComputeShader
{
public:
	//-----Public Methods----

	ComputeShader();
	~ComputeShader();

	// Loads and compiles
	bool Initialize(const char* filename);

	// Runs the program
	void Execute(int numGroupsX, int numGroupsY, int numGroupsZ);

private:
	//-----Private Data-----

	unsigned int m_programHandle = 0;

};
