/************************************************************************/
/* File: Command.cpp
/* Author: Andrew Chase
/* Date: February 3rd, 2018
/* Description: Implementation of the Command class
/************************************************************************/
#include "Engine/Core/DeveloperConsole/Command.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----Commands-----
 void Command_Help(Command &cmd);
 
//--------------------Registration Class----------------------
// Class to represent a stored command in the registry, holds meta data
class CommandRegistration
{
public:
	//-----Public Methods-----
	CommandRegistration(const std::string& name, const std::string& helpDescription, command_cb callback);

public:
	//-----Public Data-----

	std::string m_name;
	std::string m_description;
	command_cb m_callBack;

};

CommandRegistration::CommandRegistration(const std::string& name, const std::string& helpDescription, command_cb callback)
	: m_name(name)
	, m_description(helpDescription)
	, m_callBack(callback)
{
}


// Collection of registered commands
std::map<std::string, CommandRegistration*> Command::s_commandRegistry;


//-----------------------------------------------------------------------------------------------
// Constructs a command given the command line (first token is name, remaining tokens are
// arguments
//
Command::Command(const std::string& commandLine)
{
	ParseNameAndArguments(commandLine);
}


//-----------------------------------------------------------------------------------------------
// Destructor - unused
//
Command::~Command()
{
}


//-----------------------------------------------------------------------------------------------
// Returns the name of the command associated with this command
//
std::string Command::GetName()
{
	return m_name;
}


//-----------------------------------------------------------------------------------------------
// Gets the parameter value for the given flag and returns it in out_string
// Returns true if it exists, false otherwise
//
// bool Command::GetParam(const std::string& flag, std::string& out_string)
// {
// 	bool flagExists = m_arguments.find(flag) != m_arguments.end();
// 
// 	if (flagExists)
// 	{
// 		out_string = m_arguments[flag];
// 	}
// 
// 	return flagExists;
// }


//-----------------------------------------------------------------------------------------------
// Constructs and registers all Command class commands
//
void Command::Initialize()
{
	Command::Register("help", "Prints out all available commands to console", Command_Help);
}


//-----------------------------------------------------------------------------------------------
// Currently does nothing, here just in case we need something cleaned up later
//
void Command::Shutdown()
{
}


//-----------------------------------------------------------------------------------------------
// Adds the name and callback pair to the registry, checking for duplicates
//
void Command::Register(const std::string& name, const std::string& description, command_cb cb)
{
	bool commandAlreadyExists = (s_commandRegistry.find(name) != s_commandRegistry.end());
	GUARANTEE_OR_DIE(!commandAlreadyExists, Stringf("Error: Duplicate command \"%s\" in command registry.", name.c_str()));

	s_commandRegistry[name] = new CommandRegistration(name, description, cb);
}


//-----------------------------------------------------------------------------------------------
// Constructs a Command object with the given command line, and calls the command callback
// associated with its name
// Returns true if a command was found and called, false otherwise
//
bool Command::Run(const std::string& commandLine)
{
	// Push the command on to the console log
	ConsolePrintf(DevConsole::DEFAULT_COMMAND_LOG_COLOR, commandLine.c_str());

	// Add the commandLine to the console history
	DevConsole::AddCommandLineToHistory(commandLine);

	// Make a command object to be used by the callback
	Command* cmd = new Command(commandLine);

	// Ensure it exists
	bool commandExists = (s_commandRegistry.find(cmd->GetName()) != s_commandRegistry.end());
	if (!commandExists)
	{
		// Print an error
		ConsoleErrorf("INVALID COMMAND: \"%s\"", cmd->GetName().c_str());
		return false;
	}

	// Run the command
	s_commandRegistry[cmd->GetName()]->m_callBack(*cmd);
	
	// Flush the DevConsole command queue, for RCS
	DevConsole::GetInstance()->FlushOutputQueue();

	// Clean up and return
	delete cmd;
	return true;
}


//-----------------------------------------------------------------------------------------------
// Returns the command registry
//
const std::map<std::string, CommandRegistration*>& Command::GetCommands()
{
	return s_commandRegistry;
}


//-----------------------------------------------------------------------------------------------
// Separates the name and arguments from the command line and store them in this command
//
void Command::ParseNameAndArguments(const std::string& commandLine)
{
	// First get the name parsed
	size_t nameStart = commandLine.find_first_not_of(' ');

	// If the line is empty or all white space don't do anything
	if (commandLine.size() == 0 || nameStart == std::string::npos)
	{
		return;
	}

	// Get the end of the name by finding the next whitespace
	size_t nameEnd = commandLine.find_first_of(' ', nameStart);

	// No white space exists means the line is a single token, so no arguments
	if (nameEnd == std::string::npos)
	{
		m_name = commandLine;
	}
	else
	{
		// Set the name
		m_name = std::string(commandLine, nameStart, nameEnd - nameStart);

		// Parse the rest of the arguments
		size_t dashIndex = commandLine.find_first_of('-', nameEnd);

		while (dashIndex != std::string::npos)
		{
			size_t endIndex = ParseSingleArgument(commandLine, dashIndex);

			// Increment and continue
			dashIndex = commandLine.find_first_of('-', endIndex);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Parses the command line for the next flag/param from the start index
//
size_t Command::ParseSingleArgument(const std::string& commandLine, size_t dashIndex)
{
	size_t flagNameEnd = commandLine.find_first_of(' ', dashIndex);

	// The rest of the line is just a single flag with no param value associated, so throw it out
	if (flagNameEnd == std::string::npos)
	{
		return dashIndex + 1;
	}

	size_t flagLength = flagNameEnd - dashIndex - 1;

	// Just a dash is specified, no name for the flag after it, so ignore it
	if (flagLength == 0)
	{
		return dashIndex + 1;
	}

	std::string flagName = std::string(commandLine, dashIndex + 1, flagLength);
	std::string paramValue;

	// Get the parameter value
	size_t paramValueStart = commandLine.find_first_not_of(' ', flagNameEnd + 1);

	// Check if the param value is in quotes
	if (commandLine[paramValueStart] == '\"')
	{
		size_t paramValueEnd = commandLine.find('\"', paramValueStart + 1);

		// No end quote, so just return after it
		if (paramValueEnd == std::string::npos)
		{
			return paramValueStart + 1;
		}

		// Make sure there is something between the quotes
		if (paramValueStart + 1 == paramValueEnd)
		{
			return paramValueEnd + 1;
		}

		paramValue = std::string(commandLine, paramValueStart + 1, paramValueEnd - paramValueStart - 1);
		AddArgumentToMap(flagName, paramValue);
		return paramValueEnd + 1;
	}

	// No param value specified for the flag, and no other data is associated afterward so exit
	if (paramValueStart == std::string::npos)
	{
		return dashIndex + 1;
	}

	size_t paramValueEnd = commandLine.find_first_of(' ', paramValueStart);

	// No more spaces in the param value string, so pull the rest of the command line as the value
	if (paramValueEnd == std::string::npos)
	{
		paramValue = std::string(commandLine, paramValueStart, paramValueEnd);
	}
	else
	{
		// Pull the param value
		paramValue = std::string(commandLine, paramValueStart, paramValueEnd - paramValueStart);
	}

	// Add to the map of params
	AddArgumentToMap(flagName, paramValue);

	return paramValueEnd;
}


//-----------------------------------------------------------------------------------------------
// Adds the argument flag/value pair to the argument map if it doesn't exist
//
void Command::AddArgumentToMap(const std::string& flag, const std::string& value)
{
	bool alreadyExists = m_arguments.find(flag) != m_arguments.end();

	if (!alreadyExists)
	{
		m_arguments[flag] = value;
	}
}


//-----COMMANDS-----

//-----------------------------------------------------------------------------------------------
// Prints all commands in the registry with their description
// Usage: help
//
void Command_Help(Command &cmd)
{
	UNUSED(cmd);
	const std::map<std::string, CommandRegistration*>& commandRegistry = Command::GetCommands();
	std::map<std::string, CommandRegistration*>::const_iterator itr = commandRegistry.begin();

	ConsolePrintf(Rgba::GREEN, "-----Begin Help-----");
	int commandCount = 0;
	for (itr; itr != commandRegistry.end(); itr++)
	{
		ConsolePrintf("%s: %s", itr->second->m_name.c_str(), itr->second->m_description.c_str());
		commandCount++;
	}
	ConsolePrintf(Rgba::GREEN, "-----End Help, %i results-----", commandCount);
}
