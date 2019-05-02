/************************************************************************/
/* File: Lua.cpp
/* Author: Andrew Chase
/* Date: May 1st 2019
/* Description: Implementation of Lua Virtual Machine Class
/************************************************************************/
#include "Engine/Scripting/Lua.hpp"
#include "Engine/Core/LogSystem.hpp"


LuaVirtualMachine* LuaVirtualMachine::s_instance = nullptr;

//-----------------------------------------------------------------------------------------------
// Constructor
//
LuaVirtualMachine::LuaVirtualMachine()
{
	luaL_openlibs(m_state);
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
LuaVirtualMachine::~LuaVirtualMachine()
{
	lua_close(m_state);
}


//-----------------------------------------------------------------------------------------------
// Opens Lua and loads the I/O Library
//
void LuaVirtualMachine::Initialize()
{
	s_instance = new LuaVirtualMachine();
}


//-----------------------------------------------------------------------------------------------
// Closes the Lua Virtual Machine instance
//
void LuaVirtualMachine::Shutdown()
{
	if (s_instance != nullptr)
	{
		delete s_instance;
		s_instance = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Script Constructor
//
LuaScript::LuaScript(const char* filepath)
{
	m_luaVirtualMachine = luaL_newstate();
	if (luaL_loadfile(m_luaVirtualMachine, filepath) || lua_pcall(m_luaVirtualMachine, 0, 0, 0)) // Calling pcall here removes the compilation result of the file
	{
		PrintError(Stringf("Couldn't load script file \"%s\"", filepath));
		m_luaVirtualMachine = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Script Destructor
//
LuaScript::~LuaScript()
{
	if (m_luaVirtualMachine != nullptr)
	{
		lua_close(m_luaVirtualMachine);
		m_luaVirtualMachine = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Logs the error to the log file for debugging
//
void LuaScript::PrintError(const std::string& error)
{
	LogTaggedPrintf("LUA", "%s", error.c_str());
}


//-----------------------------------------------------------------------------------------------
// Sets the Lua variable stack to be at the variable given by variableName
// If variableName is a subfield of a global member, it is delimited by '.' characters
// (i.e. player.position.x)
//
bool LuaScript::GetToStack(const std::string& variableName)
{
	if (IsStringNullOrEmpty(variableName))
	{
		PrintError("Empty variable name passed to LuaScript::GetToStack()");
		return false;
	}

	std::vector<std::string> variableTokens = Tokenize(variableName, '.');
	m_stackLevel = 0;

	for (int levelIndex = 0; levelIndex < (int)variableTokens.size(); ++levelIndex)
	{
		std::string& currentToken = variableTokens[levelIndex];

		if (m_stackLevel == 0)
		{
			lua_getglobal(m_luaVirtualMachine, currentToken.c_str());
		}
		else
		{
			lua_getfield(m_luaVirtualMachine, -1, currentToken.c_str());
		}

		// Check for errors
		if (lua_isnil(m_luaVirtualMachine, -1))
		{
			PrintError(Stringf("Couldn't get to variable name \"%s\", it doesn't exist!", variableName.c_str()));
			return false;
		}

		m_stackLevel++;
	}

	return true;
}
