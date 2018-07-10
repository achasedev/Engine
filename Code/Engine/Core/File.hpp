/************************************************************************/
/* File: File.hpp
/* Author: Andrew Chase
/* Date: July 10th, 2018
/* Description: File for File I/O utility functions
/************************************************************************/
#include <string>
#include "Engine/Core/EngineCommon.hpp"

TODO("Remove these excess functions, and make everything use the File class");
TODO("Make enumeration for file open flags");
TODO("Update files to not have to load all to memory");
TODO("File flush");

// File I/O
FILE*				OpenFile(const char* filepath, const char* flags);
bool				CloseFile(FILE* fileHandle);
void*				FileReadToNewBuffer( char const *filename, size_t& out_size);
bool				FileWriteFromBuffer(char const *filename, char const* buffer, int bufferSize);
					
// Windows directory
std::string			GetWorkingDirectory();
std::string			GetFullFilePath(const std::string& localFilePath);


// Class to represent a single file object
class File
{
public:
	//-----Public Methods-----

	// Initialization
	File() {}
	~File() {}

	// Opening/Closing
	bool Open(const char* filepath, const char* flags);
	bool Close();

	// Read/Writing
	void Write(const char* buffer, size_t length);
	bool LoadFileToMemory();

	// For manipulating files loaded into memory
	unsigned int	GetNextLine(std::string& out_string);
	bool			IsAtEndOfFile() const;


private:
	//-----Private Data-----

	// File pointer (FILE*)
	void* m_filePointer = nullptr;

	// File data
	size_t		m_size = 0;
	const char* m_data = nullptr;

	// For parsing file contents loaded into memory
	unsigned int	m_offset = 0;
	bool			m_isAtEndOfFile = false;
	unsigned int	m_lineNumber = 0;

};
