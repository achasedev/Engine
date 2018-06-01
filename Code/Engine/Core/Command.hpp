/************************************************************************/
/* File: Command.hpp
/* Author: Andrew Chase
/* Date: February 3rd, 2018
/* Description: Class to represent a console command line input
/************************************************************************/
#pragma once

#include <string>
#include <map>
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/DevConsole.hpp"

class Vector2;
class Vector3;
class Rgba;

// Command callbacks take a Command.
class Command;
class CommandRegistration;
typedef void (*command_cb)( Command &cmd ); 

// A command is a single submitted commmand
// Command cmd = Command( "echo_with_color (255,255,0) \"Hello \\"World\\"\" ); 
class Command
{
public:
	Command(const std::string& commandLine); 
	~Command(); 

	std::string GetName();			// Returns the name of the command (first token of the line)

	// For initialization/shutdown
	static void Initialize();
	static void Shutdown();
	static void Register(const std::string& name, const std::string& description, command_cb);	// Registers a callback to the registry under the given name (no duplicates allowed)
	static bool Run(const std::string& commandLine);												// Runs the callback associated with the name specified in commandLine

	// Static wrappers for getting member variables
	const static std::map<std::string, CommandRegistration*>& GetCommands();


	// For getting params from the parsed command line
	template <typename T> 
	bool GetParam(const std::string& flag, T& out_value, const T* defaultValue = nullptr)
	{
		bool paramExists = m_arguments.find(flag) != m_arguments.end();

		if (paramExists)
		{
			bool succeeded = SetFromText(m_arguments[flag], out_value);
			if (!succeeded)
			{
				ConsoleErrorf("Could parse parameter value \"%s\" for flag \"-%s\"", m_arguments[flag].c_str(), flag.c_str());

				if (defaultValue != nullptr)
				{
					ConsoleWarningf("Defaulting to value %s", ToString(defaultValue).c_str());
					out_value = *defaultValue;
				}
			}
		}
		else
		{
			if (defaultValue != nullptr)
			{
				ConsoleWarningf("Flag \"-%s\" was not specified, defaulting to value %s", flag.c_str(), ToString(defaultValue).c_str());
				out_value = *defaultValue;
			}
		}

		return paramExists;
	}

private:
	//-----Private Methods-----

	void	ParseNameAndArguments(const std::string& commandLine);				// Separates the name from the arguments, for Command constructor
	int		ParseSingleArgument(const std::string& commandLine, int startIndex);

	void	AddArgumentToMap(const std::string& flag, const std::string& value);


private:
	//-----Private Data-----

	std::string m_name;									// First token of the command line
	std::map<std::string, std::string> m_arguments;		// All of the flag/value pairs from the command line

	static std::map<std::string, CommandRegistration*> s_commandRegistry;		// Collection of registered commands by name
};




// *********************************************************************//
// ** EXTRAS - left here as a reminder of tasks to complete			 ** //
// *********************************************************************//

// [E02.01]
// Runs a "script", or multiple lines of commands (each line
// is treated as a command). 
// Leading whitespace is ignored, and 
// lines starting with '#' should be ignored. 
//void CommandRunScript( char const *script ); 
//void CommandRunScriptFile( char const *filename ); 

// Returns a list of commands that start with the root word
// should ignore case. 
//std::vector<string> CommandAutoComplete( char const *root ); 