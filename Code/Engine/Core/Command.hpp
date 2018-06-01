/************************************************************************/
/* File: Command.hpp
/* Author: Andrew Chase
/* Date: February 3rd, 2018
/* Description: Class to represent a console command line input
/************************************************************************/
#pragma once

#include <string>
#include <map>

class Vector2;
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
	std::string GetNextString();	// Returns the next string argument not parsed out of the line (DESTRUCTIVE)


	// Helpers for getting arguments
	bool GetNextInt( int *out_val ); 
	bool GetNextColor( Rgba *out_val );
	bool GetNextVector2( Vector2 *out_val );  

	// For initialization/shutdown
	static void Initialize();
	static void Shutdown();
	static void Register(const std::string& name, const std::string& description, command_cb);	// Registers a callback to the registry under the given name (no duplicates allowed)
	static bool Run(const std::string& commandLine);												// Runs the callback associated with the name specified in commandLine

	// Static wrappers for getting member variables
	const static std::map<std::string, CommandRegistration*>& GetCommands();


private:
	//-----Private Methods-----

	void ParseNameAndArguments(const std::string& commandLine);				// Separates the name from the arguments, for Command constructor


private:
	//-----Private Data-----

	std::string m_name;				// First token of the command line
	std::string m_arguments;		// All other tokens of the line, is mutated as GetNextString() is called

	static std::map<std::string, CommandRegistration*> s_commandRegistry;		// Collection of registered commands by name
};




// *********************************************************************//
// ** EXTRAS - left here as a reminder of tasks to complete			 ** //
// *********************************************************************//

// [E02.00]
// Returns a list containing the last entered
// commands ordered by how recently they were used.
// If a command is entered twice, it should not appear
// twice, but instead just just move to the front. 
// History length is up to you, but 32 or 64 is good.
//std::vector<string> GetCommandHistory(); 

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