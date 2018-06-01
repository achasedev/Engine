#include "Engine/Core/StringUtils.hpp"
#include <stdarg.h>


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
	int subStringStartIndex = stringToTokenize.find_first_not_of(delimiter);
	if (subStringStartIndex == (int) std::string::npos) { return tokens; }	// Return if the entire string is just delimiters
	int subStringEndPostion = stringToTokenize.find(delimiter, subStringStartIndex + 1);

	// Iterate across the entire string
	while (subStringEndPostion != (int) std::string::npos)
	{
		// Create the substring
		int substringLength = (subStringEndPostion - subStringStartIndex);
		tokens.push_back(std::string(stringToTokenize, subStringStartIndex, substringLength));

		// Update the indices
		subStringStartIndex = stringToTokenize.find_first_not_of(delimiter, subStringEndPostion + 1);
		if (subStringStartIndex == (int) std::string::npos) { return tokens; } // Return if the rest of the string is just delimiters
		subStringEndPostion = stringToTokenize.find(delimiter, subStringStartIndex + 1);
	}

	// Add the rest of the string
	tokens.push_back(std::string(stringToTokenize, subStringStartIndex));

	return tokens;
}




