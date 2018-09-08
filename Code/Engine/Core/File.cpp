/************************************************************************/
/* File: File.hpp
/* Author: Andrew Chase
/* Date: July 10th, 2018
/* Description: Implementation of the File Class + helper functions
/************************************************************************/
#include "Engine/Core/File.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <stdio.h>
#include <cstdlib>

// For Windows directory functions
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>			// #include this (massive, platform-specific) header in very few places

TODO("Safety checks, if file is open already or not, if file is loaded into memory or not");
//-----------------------------------------------------------------------------------------------
// Opens the file given by filepath and returns the file pointer associated to it
//
FILE* OpenFile(const char* filepath, const char* flags)
{
	FILE *fp = nullptr; 
	fopen_s(&fp, filepath, flags); 
	if (fp == nullptr) {
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


//-----------------------------------------------------------------------------------------------
// Writes to the file given by filename, returning false if the file doesn't exist
// Currently overwrites all file contents, does NOT append
//
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


//-----------------------------------------------------------------------------------------------
// Returns the working directory path
//
std::string GetWorkingDirectory()
{
	char buffer[MAX_PATH + 1];

	GetCurrentDirectoryA(MAX_PATH, buffer);
	buffer[MAX_PATH] = '\0';

	return std::string(buffer);
}


//-----------------------------------------------------------------------------------------------
// Returns the full file path of the file given by localFilePath (using the current working directory)
//
std::string GetFullFilePath(const std::string& localFilePath)
{
	return GetWorkingDirectory() + "\\" + localFilePath;
}


//-----------------------------------------------------------------------------------------------
// Opens the file given by filepath, using the appropriate flags
//
bool File::Open(const char* filepath, const char* flags)
{
	m_filePointer = (void*) OpenFile(filepath, flags);

	return (m_filePointer != nullptr);
}


//-----------------------------------------------------------------------------------------------
// Closes the file currently opened by this file
//
bool File::Close()
{
	bool success = CloseFile((FILE*) m_filePointer);
	return success;
}


//-----------------------------------------------------------------------------------------------
// Writes the buffer data to the file currently opened by this file
//
void File::Write(const char* buffer, size_t length)
{
	fwrite(buffer, sizeof(char), length, (FILE*) m_filePointer);
}


//-----------------------------------------------------------------------------------------------
// Flushes any outstanding writes to disk
//
void File::Flush()
{
	fflush((FILE*) m_filePointer);
}


//-----------------------------------------------------------------------------------------------
// Reads the file contents into memory
//
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


//-----------------------------------------------------------------------------------------------
// Returns the next line of the file, using the internal offset in this File object
//
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


//-----------------------------------------------------------------------------------------------
// Returns true if the file offset is at or passed the end of the file contents
//
bool File::IsAtEndOfFile() const
{
	return m_isAtEndOfFile;
}
