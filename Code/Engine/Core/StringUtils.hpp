#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>

//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... );
const std::string Stringf( const int maxLength, const char* format, ... );
const std::vector<std::string> Tokenize(const std::string& stringToTokenize, const char delimiter);

int GetStringLength(const char* string);

bool IsStringNullOrEmpty(const char* string);
bool IsStringNullOrEmpty(const std::string& string);


