#include <cstring>
#include <stdarg.h>
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/IntVector3.hpp"

//-----------------------------------------------------------------------------------------------
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const int maxLength, const char* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}


//-----------------------------------------------------------------------------------------------
const std::vector<std::string> Tokenize(const std::string& stringToTokenize, const char delimiter)
{
	std::vector<std::string> tokens;
	
	// Set up the substring indices
	size_t subStringStartIndex = stringToTokenize.find_first_not_of(delimiter);
	if (subStringStartIndex == std::string::npos) { return tokens; }	// Return if the entire string is just delimiters
	size_t subStringEndPostion = stringToTokenize.find(delimiter, subStringStartIndex + 1);

	// Iterate across the entire string
	while (subStringEndPostion != std::string::npos)
	{
		// Create the substring
		size_t substringLength = (subStringEndPostion - subStringStartIndex);
		tokens.push_back(std::string(stringToTokenize, subStringStartIndex, substringLength));

		// Update the indices
		subStringStartIndex = stringToTokenize.find_first_not_of(delimiter, subStringEndPostion + 1);
		if (subStringStartIndex == std::string::npos) { return tokens; } // Return if the rest of the string is just delimiters
		subStringEndPostion = stringToTokenize.find(delimiter, subStringStartIndex + 1);
	}

	// Add the rest of the string
	tokens.push_back(std::string(stringToTokenize, subStringStartIndex));

	return tokens;
}


//-----------------------------------------------------------------------------------------------
int GetStringLength(const char* string)
{
	return (int) strlen(string);
}


//-----------------------------------------------------------------------------------------------
int GetCharacterCount(const std::string& text, const char character)
{
	int count = 0;

	for (int charIndex = 0; charIndex < (int) text.size(); ++charIndex)
	{
		if (text[charIndex] == character)
		{
			count++;
		}
	}

	return count;
}


//-----------------------------------------------------------------------------------------------
bool IsStringNullOrEmpty(const char* string)
{
	bool stringNull = (string == nullptr);
	bool stringEmpty = true;
	if (!stringNull)
	{
		stringEmpty = (strlen(string) == 0);
	}

	return (stringNull || stringEmpty);
}


//-----------------------------------------------------------------------------------------------
bool IsStringNullOrEmpty(const std::string& string)
{
	return IsStringNullOrEmpty(string.c_str());
}


//-----------------------------------------------------------------------------------------------
float StringToFloat(const std::string& text)
{
	return (float) atof(text.c_str());
}

//-----------------------------------------------------------------------------------------------

int StringToInt(const std::string& text)
{
	return atoi(text.c_str());
}


//-----------------------------------------------------------------------------------------------
bool StringToBool(const std::string& text, bool& out_bool)
{
	bool succeeded = false;

	if (text == "True" || text == "true")	
	{ 
		out_bool = true;
		succeeded = true;
	}
	else if (text == "false" || text == "False")
	{
		out_bool = false;
		succeeded = true;
	}

	return succeeded;
}


//-----------------------------------------------------------------------------------------------
// Sets the Vector3 to the values represented in the text passed
//
bool SetFromText(const std::string& text, Vector3& out_val)
{
	size_t firstComma = static_cast<int>(text.find(","));

	// No comma present in text
	if (firstComma == std::string::npos)
	{
		return false;
	}

	size_t secondComma = text.find(",", firstComma + 1);

	// No second comma present in text
	if (secondComma == std::string::npos)
	{
		return false;
	}

	// Set the values
	out_val.x = static_cast<float>(atof(std::string(text, 0, firstComma).c_str()));
	out_val.y = static_cast<float>(atof(std::string(text, firstComma + 1, secondComma - firstComma - 1).c_str()));
	out_val.z = static_cast<float>(atof(std::string(text, secondComma + 1).c_str()));

	return true;
}


//-----------------------------------------------------------------------------------------------
// Sets the float to the value represented in the text passed
//
bool SetFromText(const std::string& text, float& out_value)
{
	out_value = (float) atof(text.c_str());
	return true;
}


//-----------------------------------------------------------------------------------------------
// Sets the int to the value represented in the text passed
//
bool SetFromText(const std::string& text, int& out_value)
{
	out_value = atoi(text.c_str());
	return true;
}


//-----------------------------------------------------------------------------------------------
// Sets the unsigned int to the value represented in the text passed
//
bool SetFromText(const std::string& text, unsigned int& out_value)
{
	out_value = atoi(text.c_str());
	return true;
}


//-----------------------------------------------------------------------------------------------
// Sets the unsigned int to the value represented in the text passed
//
bool SetFromText(const std::string& text, bool& out_value)
{
	bool successful = false;

	if (text == "true" || text == "True")
	{
		out_value = true;
		successful = true;
	}
	else if (text == "false" || text == "False")
	{
		out_value = false;
		successful = true;
	}

	return successful;
}


//-----------------------------------------------------------------------------------------------
// Sets the Rgba out_value to the value represented in the text passed
//
bool SetFromText(const std::string& text, Rgba& out_value)
{
	return out_value.SetFromText(text.c_str());
}


//-----------------------------------------------------------------------------------------------
// Sets the Vector2 out_value to the value represented in the text passed
//
bool SetFromText(const std::string& text, Vector2& out_value)
{
	size_t firstComma = text.find(",");

	// No comma present in text
	if (firstComma == std::string::npos)
	{
		return false;
	}

	// Set the values
	out_value.x = static_cast<float>(atof(std::string(text, 0, firstComma).c_str()));
	out_value.y = static_cast<float>(atof(std::string(text, firstComma + 1).c_str()));

	return true;
}


//-----------------------------------------------------------------------------------------------
// Sets the string out_value to the value represented in the text passed (for use with templates)
//
bool SetFromText(const std::string& text, std::string& out_value)
{
	out_value = text;
	return true;
}


//-----------------------------------------------------------------------------------------------
// Sets the string out_value to the value represented in the text passed (for use with templates)
//
bool SetFromText(const std::string& text, unsigned short& out_value)
{
	out_value = (unsigned short) atoi(text.c_str());
	return true;
}


//-----------------------------------------------------------------------------------------------
// Sets the string out_value to the value represented in the text passed (for use with templates)
//
bool SetFromText(const std::string& text, IntVector3& out_value)
{
	std::vector<std::string> tokens = Tokenize(text, ' ');

	if (tokens.size() != 3)
	{
		return false;
	}

	out_value.x = StringToInt(tokens[0]);
	out_value.y = StringToInt(tokens[1]);
	out_value.z = StringToInt(tokens[2]);

	return true;
}


//-----------------------------------------------------------------------------------------------
// Returns text representation of the float
//
std::string ToString(float value)
{
	return std::to_string(value);
}


//-----------------------------------------------------------------------------------------------
// Returns text representation of the int
//
std::string ToString(int value)
{
	return std::to_string(value);
}


//-----------------------------------------------------------------------------------------------
// Returns text representation of the unsigned int
//
std::string ToString(unsigned int value)
{
	return std::to_string(value);
}


//-----------------------------------------------------------------------------------------------
// Returns text representation of the Rgba
//
std::string ToString(const Rgba& value)
{
	return Stringf("(%u,%u,%u,%u)", value.r, value.g, value.b, value.a);
}


//-----------------------------------------------------------------------------------------------
// Returns text representation of the Vector2
//
std::string ToString(const Vector2& value)
{
	return Stringf("(%f,%f)", value.x, value.y);
}


//-----------------------------------------------------------------------------------------------
// Returns text representation of the Vector3
//
std::string ToString(const Vector3& value)
{
	return Stringf("(%f,%f,%f)", value.x, value.y, value.z);
}


//-----------------------------------------------------------------------------------------------
// Returns text representation of the Vector4
//
std::string ToString(const Vector4& value)
{
	return Stringf("(%f,%f,%f,%f)", value.x, value.y, value.z, value.w);
}


//-----------------------------------------------------------------------------------------------
// For use with templates, does nothing
//
std::string ToString(const std::string& value)
{
	return value;
}


//-----------------------------------------------------------------------------------------------
// For use with templates, does nothing
//
std::string ToString(bool value)
{
	std::string text;

	if (value)  { text = "true";  }
	else		{ text = "false"; }

	return text;
}


std::string ToString(const float* value)
{
	return ToString(*value);
}

std::string ToString(const int* value)
{
	return ToString(*value);
}

std::string ToString(const unsigned int* value)
{
	return ToString(*value);
}

std::string ToString(const bool* value)
{
	return ToString(*value);
}

std::string ToString(const Rgba* value)
{
	return ToString(*value);
}

std::string ToString(const Vector2* value)
{
	return ToString(*value);
}

std::string ToString(const Vector3* value)
{
	return ToString(*value);
}

std::string ToString(const Vector4* value)
{
	return ToString(*value);
}

std::string ToString(const std::string* value)
{
	return *value;
}
