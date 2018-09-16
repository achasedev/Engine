/************************************************************************/
/* File: ComputeShader.cpp
/* Author: Andrew Chase
/* Date: September 15th, 2018
/* Description: Implementation of the ComputeShader class
/************************************************************************/
#include "Engine/Rendering/Shaders/ComputeShader.hpp"
#include "Engine/Rendering/OpenGL/glFunctions.hpp"
#include "Engine/Core/File.hpp"
#include "Engine/Core/LogSystem.hpp"

// C functions for error checking
static void LogShaderError(GLuint shader_id, const char* filename);
static void FormatAndPrintShaderError(const std::string& errorLog, const std::string& localFilePath);
static void LogProgramError(GLuint program_id);


//-----------------------------------------------------------------------------------------------
// Default constructor
//
ComputeShader::ComputeShader()
{
}


//-----------------------------------------------------------------------------------------------
// Destructor - delete the program from GPU
//
ComputeShader::~ComputeShader()
{
	if (m_programHandle != NULL)
	{
		glDeleteProgram(m_programHandle);
		m_programHandle = NULL;
	}
}


//-----------------------------------------------------------------------------------------------
// Loads and compiles the program from the given filename
//
bool ComputeShader::Initialize(const char* filename)
{
	GLuint shaderID = glCreateShader(GL_COMPUTE_SHADER);

	// Get the shader source
	size_t size;
	char* src = (char*)FileReadToNewBuffer(filename, size);

	glShaderSource(shaderID, 1, &src, NULL);
	glCompileShader(shaderID);

	// Check compile status
	GLint status;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) 
	{
		LogShaderError(shaderID, filename);
		glDeleteShader(shaderID);
		return false;
	}

	m_programHandle = glCreateProgram();

	glAttachShader(m_programHandle, shaderID);
	glLinkProgram(m_programHandle);

	// Check Link Status
	GLint link_status;
	glGetProgramiv(m_programHandle, GL_LINK_STATUS, &link_status);

	if (link_status == GL_FALSE) {
		LogProgramError(m_programHandle);
		glDeleteProgram(m_programHandle);
		m_programHandle = NULL;
		return false;
	}

	// no longer need the shaders objects, you can detach them if you want
	// (not necessary)
	glDetachShader(m_programHandle, shaderID);

	return true;
}


//-----------------------------------------------------------------------------------------------
// Runs the compute shader with the given group layout
//
void ComputeShader::Execute(int numGroupsX, int numGroupsY, int numGroupsZ)
{
	PROFILE_LOG_SCOPE_FUNCTION();

	if (m_programHandle == NULL)
	{
		LogTaggedPrintf("COMPUTE_SHADER", "Error: Execute() called on a compute shader with NULL handle");
		return;
	}

	glUseProgram(m_programHandle);
	glDispatchCompute((GLuint)numGroupsX, (GLuint)numGroupsY, (GLuint)numGroupsZ);
	GL_CHECK_ERROR();

	// Block all future gl calls until this step finishes
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}


//-----------------------------------------------------------------------------------------------
// Parses the error log and makes a Visual Studio "Double-click to open" shortcut and prints it
// to the output pane
// Only does extra formatting if the error log is in Nvidia formatting, otherwise prints the
// original error log
//
void FormatAndPrintShaderError(const std::string& errorLog, const std::string& localFilePath)
{
	// Parsing the error data from Nvidia format
	size_t firstParenIndex = errorLog.find_first_of('(');
	size_t lastParenIndex = errorLog.find_first_of(')');
	size_t colonIndex = errorLog.find_first_of(':');

	// Nvidia error log has (LINE_NUM) at the start of the error log, and a ':' starting the error message
	bool isNvidiaFormatting = (firstParenIndex != std::string::npos && lastParenIndex != std::string::npos && colonIndex != std::string::npos);

	// Get the full file path, is printed in either case
	std::string fullFilePath = GetFullFilePath(localFilePath);

	if (isNvidiaFormatting)
	{
		// Print with additional formatting
		std::string lineNum = std::string(errorLog, firstParenIndex + 1, lastParenIndex - firstParenIndex - 1);
		std::string errorMessage = std::string(errorLog, colonIndex);

		DebuggerPrintf("SHADER ERROR on line %s of %s\n", lineNum.c_str(), localFilePath.c_str());
		DebuggerPrintf("%s(%s)%s\n", fullFilePath.c_str(), lineNum.c_str(), errorMessage.c_str()); // Use this specific format so Visual Studio users can double-click to jump to file-and-line of error
	}
	else
	{
		// Otherwise just print the original log if not in Nvidia format
		DebuggerPrintf("SHADER ERROR in file %s\n", localFilePath.c_str());
		DebuggerPrintf(errorLog.c_str());
	}
}

//-----------------------------------------------------------------------------------------------
// Logs the shader error to std::out(?)
// If compilation fails, the driver will usually give us a log about why.  The formatting of this 
// log is up to the vendor, but most of us are on Nvidia cards, so it should be fine.  Just be aware
// that if you use a different vendor (Intel, AMD, etc) your errors may look different.
//
static void LogShaderError(GLuint shader_id, const char* filename)
{
	// figure out how large the buffer needs to be
	GLint length;
	glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);

	// make a buffer, and copy the log to it. 
	char *buffer = new char[length + 1];
	glGetShaderInfoLog(shader_id, length, &length, buffer);

	// Print it out (may want to do some additional formatting)
	buffer[length] = NULL;

	// Log the error
	DebuggerPrintf("\n==============================================================================\n");
	FormatAndPrintShaderError(buffer, filename);
	DebuggerPrintf("==============================================================================\n\n");

	// free up the memory we used. 
	delete buffer;
}

//-----------------------------------------------------------------------------------------------
// Logs program errors
//
static void LogProgramError(GLuint program_id)
{
	// get the buffer length
	GLint length;
	glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &length);

	// copy the log into a new buffer
	char *buffer = new char[length + 1];
	glGetProgramInfoLog(program_id, length, &length, buffer);

	// print it to the output pane
	buffer[length] = NULL;

	// Log the error (could reformat this later)
	DebuggerPrintf("PROGRAM ERROR: %s", buffer);

	// cleanup
	delete buffer;
}
