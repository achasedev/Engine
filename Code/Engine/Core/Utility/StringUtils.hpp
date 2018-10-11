#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>

class Vector2;
class Vector3;
class Vector4;
class IntVector3;
class Rgba;

//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... );
const std::string Stringf( const int maxLength, const char* format, ... );
const std::vector<std::string> Tokenize(const std::string& stringToTokenize, const char delimiter);

int GetStringLength(const char* string);
int GetCharacterCount(const std::string& text, const char character);

bool IsStringNullOrEmpty(const char* string);
bool IsStringNullOrEmpty(const std::string& string);

float StringToFloat(const std::string& text);
int StringToInt(const std::string& text);

bool SetFromText(const std::string& text, float& out_value);
bool SetFromText(const std::string& text, int& out_value);
bool SetFromText(const std::string& text, unsigned int& out_value);
bool SetFromText(const std::string& text, bool& out_value);
bool SetFromText(const std::string& text, unsigned short& out_value);

bool SetFromText(const std::string& text, Rgba& out_value);
bool SetFromText(const std::string& text, Vector2& out_value);
bool SetFromText(const std::string& text, Vector3& out_value);
bool SetFromText(const std::string& text, std::string& out_value);
bool SetFromText(const std::string& text, IntVector3& out_value);

std::string ToString(float value);
std::string ToString(int value);
std::string ToString(unsigned int value);
std::string ToString(bool value);

std::string ToString(const Rgba& value);
std::string ToString(const Vector2& value);
std::string ToString(const Vector3& value);
std::string ToString(const Vector4& value);

std::string ToString(const float* value);
std::string ToString(const int* value);
std::string ToString(const unsigned int* value);
std::string ToString(const bool* value);

std::string ToString(const Rgba* value);
std::string ToString(const Vector2* value);
std::string ToString(const Vector3* value);
std::string ToString(const Vector4* value);

std::string ToString(const std::string& value);	// For use with templating
std::string ToString(const std::string* value);

std::string GetAsBitString(uint16_t value);