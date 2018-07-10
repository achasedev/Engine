/************************************************************************/
/* File: File.hpp
/* Author: Andrew Chase
/* Date: January 29th, 2017
/* Bugs: None
/* Description: File for File utility functions
/************************************************************************/
#include <string>
#include "Engine/Core/EngineCommon.hpp"

TODO("Remove these excess functions, and make everything use the File class");
TODO("Comment");
FILE*			OpenFile(const char* filepath, const char* flags);
bool			CloseFile(FILE* fileHandle);
void*			FileReadToNewBuffer( char const *filename, size_t& out_size);
bool			FileWriteFromBuffer(char const *filename, char const* buffer, int bufferSize);

std::string		GetWorkingDirectory();
std::string		GetFullFilePath(const std::string& localFilePath);


class File
{
public:

	File() {}
	~File() {}

	bool Open(const char* filepath, const char* flags);
	bool Close();

	void			Write(const char* buffer, size_t length);
	bool			LoadFileToMemory();

	unsigned int	GetNextLine(std::string& out_string);
	bool			IsAtEndOfFile() const;


private:

	void* m_filePointer = nullptr;

	size_t m_size = 0;
	unsigned int m_offset = 0;
	unsigned int m_lineNumber = 0;
	const char* m_data = nullptr;

	bool m_isAtEndOfFile = false;
};