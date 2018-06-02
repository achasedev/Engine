/************************************************************************/
/* File: File.cpp
/* Author: Andrew Chase
/* Date: January 29th, 2017
/* Bugs: None
/* Description: File for File utility functions
/************************************************************************/
#include "Engine/Core/File.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <stdio.h>
#include <cstdlib>

#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places

//-----------------------------------------------------------------------------------------------
// Reads the file given by filename into a buffer and returns a reference to it 
//
void* FileReadToNewBuffer( char const *filename )
{
	FILE *fp = nullptr; 
	fopen_s( &fp, filename, "r" ); 
	if (fp == nullptr) {
		ERROR_RECOVERABLE(Stringf("Warning: FileReadToNewBuffer() couldn't open file \"%s\".", filename));
		return nullptr; 
	}

	size_t size = 0U; 

	fseek(fp, 0L, SEEK_END);
	size = ftell(fp); 

	fseek(fp, 0L, SEEK_SET); 

	unsigned char *buffer = (unsigned char*) malloc(size + 1U); // space for NULL 

	size_t read = fread( buffer, 1, size, fp );
	fclose(fp);

	buffer[read] = NULL; 
	return buffer;  
}

bool FileWriteFromBuffer(char const *filename, char const* buffer, int bufferSize)
{
	FILE *fp = nullptr;
	fopen_s(&fp, filename, "w+");
	if (fp == nullptr)
	{
		return false;
	}

	fwrite(buffer, sizeof(char), bufferSize, fp);

	int err = fclose(fp);
	if (err == 0)
	{
		ConsolePrintf(Rgba::LIGHT_GREEN, "File written to %s.", filename);
		return true;
	}


	ConsoleErrorf("Could not write the file to the given path: %s!", filename);
	return false;
}


std::string GetWorkingDirectory()
{
	char buffer[MAX_PATH + 1];

	GetCurrentDirectoryA(MAX_PATH, buffer);
	buffer[MAX_PATH] = '\0';

	return std::string(buffer);
}


std::string GetFullFilePath(const std::string& localFilePath)
{
	return GetWorkingDirectory() + "\\" + localFilePath;
}
