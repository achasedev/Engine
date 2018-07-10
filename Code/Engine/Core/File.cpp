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
// Opens the file given by filepath and returns the file pointer associated to it
//
FILE* OpenFile(const char* filepath, const char* flags)
{
	FILE *fp = nullptr; 
	fopen_s(&fp, filepath, flags); 
	if (fp == nullptr) {
		ERROR_RECOVERABLE(Stringf("Warning: FileReadToNewBuffer() couldn't open file \"%s\".", filepath));
		return nullptr; 
	}

	fseek(fp, 0L, SEEK_SET); 
	return fp;
}


//-----------------------------------------------------------------------------------------------
// Closes the file given by the fileHandle
//
bool CloseFile(FILE* fileHandle)
{
	int err = fclose(fileHandle);
	if (err != 0)
	{
		ERROR_RECOVERABLE(Stringf("Warning: CloseFile could not close the file."));
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
// Reads the file given by filename into a buffer and returns a reference to it 
//
void* FileReadToNewBuffer( char const *filename, size_t& out_size)
{
	FILE* fp = OpenFile(filename, "r");
	if (fp == nullptr) 
	{
		return nullptr;
	}

	out_size = 0U; 

	fseek(fp, 0L, SEEK_END);
	out_size = ftell(fp); 

	fseek(fp, 0L, SEEK_SET); 

	unsigned char *buffer = (unsigned char*) malloc(out_size + 1U); // space for NULL 

	size_t read = fread( buffer, 1, out_size, fp );
	
	CloseFile(fp);

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

	return CloseFile(fp);
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

bool File::Open(const char* filepath, const char* flags)
{
	m_filePointer = (void*) OpenFile(filepath, flags);

	return (m_filePointer != nullptr);
}

bool File::Close()
{
	bool success = CloseFile((FILE*) m_filePointer);
	return success;
}

void File::Write(const char* buffer, size_t length)
{
	fwrite(buffer, sizeof(char), length, (FILE*) m_filePointer);
}

bool File::LoadFileToMemory()
{
	m_size = 0U; 

	FILE* fp = (FILE*) m_filePointer;

	// Get the file size
	fseek(fp, 0L, SEEK_END);
	m_size = ftell(fp); 

	// Set back to beginning
	fseek(fp, 0L, SEEK_SET); 

	// Make the buffer
	unsigned char* data = (unsigned char*) malloc(m_size + 1U); // space for NULL 

	// Read the data in
	size_t read = fread(data, 1, m_size, fp);

	// Null terminate and return
	data[read] = NULL;   
	m_data = (const char*) data;

	return true;
}

unsigned int File::GetNextLine(std::string& out_string)
{
	if (m_data == nullptr || m_offset >= m_size)
	{
		out_string = "";
		m_lineNumber;
	}

	unsigned int endIndex = m_offset;
	while (m_data[endIndex] != '\n' && endIndex < m_size)
	{
		endIndex++;
	}

	size_t length = (endIndex - m_offset);
	out_string = std::string(&m_data[m_offset], length);

	m_offset = endIndex + 1;
	m_lineNumber++;
	m_isAtEndOfFile = (m_offset >= m_size);

	return m_lineNumber;
}

bool File::IsAtEndOfFile() const
{
	return m_isAtEndOfFile;
}
