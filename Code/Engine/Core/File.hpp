/************************************************************************/
/* File: File.hpp
/* Author: Andrew Chase
/* Date: January 29th, 2017
/* Bugs: None
/* Description: File for File utility functions
/************************************************************************/
#include <string>

void* FileReadToNewBuffer( char const *filename );
bool FileWriteFromBuffer(char const *filename, char const* buffer, int bufferSize);

std::string GetWorkingDirectory();
std::string GetFullFilePath(const std::string& localFilePath);