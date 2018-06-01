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

class ShaderDescription;
class PropertyBlockDescription;

class ShaderProgram
{
public:
	//-----Public Methods-----

	ShaderProgram(const std::string& name);
	~ShaderProgram(); // Destructor - free the program memory on the gpu

	const ShaderProgram* Clone() const;

	// Loads the shaders given by rootName, compiles them, and links them to this program
	bool LoadProgramFromFiles(const char *rootName);
	bool LoadProgramFromFiles(const char* vsFilepath, const char* fsFilepath);

	// Loads the shaders from the string literal source code provided, and links them to this program
	bool LoadProgramFromSources(const char *vertexShaderSource, const char* fragmentShaderSource);

	// Accessors
	const std::string&	GetName() const;
	unsigned int		GetHandle() const;
	const std::string&	GetVSFilePathOrSource() const;
	const std::string&	GetFSFilePathOrSource() const;

	const ShaderDescription* GetUniformDescription() const;

	bool WasBuiltFromSource() const;


private:
	//-----Private Methods-----

	// Shader reflection
	void SetupPropertyBlockInfos();
	void FillBlockProperties(PropertyBlockDescription* blockInfo, int blockIndex);


private:
	//-----Private Data-----
	
	std::string m_name;

	// The handle identifying this program on the GPU
	unsigned int m_programHandle = NULL;

	// Source filename(s) of this program, empty string denotes this was a built-in shader
	std::string m_vsFilePathOrSource;	
	std::string m_fsFilePathOrSource;

	bool m_areFilepaths = false;

	ShaderDescription* m_uniformDescription = nullptr;
};
