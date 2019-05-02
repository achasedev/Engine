/************************************************************************/
/* File: Lua.hpp
/* Author: Andrew Chase
/* Date: May 1st 2019
/* Description: Header for including components necessary for Lua scripting
/************************************************************************/
#ifndef __LUA_INC_H__
#define __LUA_INC_H__

// Everything is defined as C functions
extern "C"
{
#include "ThirdParty/lua/include/lua.h"
#include "ThirdParty/lua/include/lauxlib.h"
#include "ThirdParty/lua/include/lualib.h"
}

// Link to the static library
#pragma comment(lib, "ThirdParty/lua/lua53.lib")

#include <vector>
#include "Engine/Core/Utility/StringUtils.hpp"


class LuaScript
{
public:
	//-----Public Methods-----

	LuaScript(const char* filepath);
	~LuaScript();

	void PrintLuaMessage(const std::string& message) const;
	void ClearLuaStack();

public:
	//-----Data Manipulation-----

	void PrintStack() const;
	bool GetToStack(const std::string& variableName);


	//-----------------------------------------------------------------------------------------------
	template <typename T>
	T Get(const std::string& variableName)
	{
		if (m_luaVirtualMachine == nullptr)
		{
			PrintLuaMessage("Attempted to Get() on a script that isn't loaded");
			return GetDefault<T>();
		}

		T result;
		bool foundVariable = GetToStack(variableName);

		if (foundVariable)
		{
			result = GetByType<T>(variableName);
		}
		else
		{
			result = GetDefault<T>();
		}

		// Reset the stack back to where it was before we started
		ClearLuaStack();

		return result;
	}


private:
	//-----Data Accessor Helpers (Template Specializations)-----

	//-----------------------------------------------------------------------------------------------
	template <typename T>
	inline T GetByType(const std::string& variableName)
	{
		return 0;
	}


	//-----------------------------------------------------------------------------------------------
	template <>
	inline bool GetByType(const std::string& variableName)
	{
		if (!lua_isboolean(m_luaVirtualMachine, -1))
		{
			PrintLuaMessage(Stringf("Attempted to get variable \"%s\" as a float but it's not a number", variableName.c_str()));
		}

		return (bool)lua_toboolean(m_luaVirtualMachine, -1);
	}

	
	//-----------------------------------------------------------------------------------------------
	template <>
	inline float GetByType(const std::string& variableName)
	{
		if (!lua_isnumber(m_luaVirtualMachine, -1))
		{
			PrintLuaMessage(Stringf("Attempted to get variable \"%s\" as a float but it's not a number", variableName.c_str()));
		}

		return (float)lua_tonumber(m_luaVirtualMachine, -1);
	}


	//-----------------------------------------------------------------------------------------------
	template <>
	inline int GetByType(const std::string& variableName)
	{
		if (!lua_isnumber(m_luaVirtualMachine, -1))
		{
			PrintLuaMessage(Stringf("Attempted to get variable \"%s\" as an int but it's not a number", variableName.c_str()));
		}

		return (int)lua_tonumber(m_luaVirtualMachine, -1);
	}


	//-----------------------------------------------------------------------------------------------
	template <>
	inline std::string GetByType(const std::string& variableName)
	{
		std::string value = "null";
		if (lua_isstring(m_luaVirtualMachine, -1))
		{
			value = std::string(lua_tostring(m_luaVirtualMachine, -1));
		}
		else
		{
			PrintLuaMessage(Stringf("Attempted to get variable \"%s\" as a string but it's not a string", variableName.c_str()));
		}

		return value;
	}


	//-----------------------------------------------------------------------------------------------
	template <typename T>
	T GetDefault()
	{
		return 0;
	}


public:
	//-----Data Array Accessors (Template Specialization)-----

	//-----------------------------------------------------------------------------------------------
	template <typename T>
	std::vector<T> GetArray(const std::string& arrayName)
	{
		return std::vector<T>();
	}


	//-----------------------------------------------------------------------------------------------
	template <>
	std::vector<int> GetArray(const std::string& arrayName)
	{
		std::vector<int> intVector;
		lua_getglobal(m_luaVirtualMachine, arrayName.c_str());

		if (lua_isnil(m_luaVirtualMachine, -1))
		{
			ClearLuaStack();
			return intVector;
		}

		lua_pushnil(m_luaVirtualMachine);

		while (lua_next(m_luaVirtualMachine, -2)) // Pops a key, pushes a key-value pair
		{
			if (!lua_isnumber(m_luaVirtualMachine, -1))
			{
				PrintLuaMessage(Stringf("Array \"%s\" has a non-number value at index %i", arrayName.c_str(), intVector.size()));
			}

			intVector.push_back((int)lua_tonumber(m_luaVirtualMachine, -1));
			lua_pop(m_luaVirtualMachine, 1); // Remove the value we just stored off
		}

		ClearLuaStack();
		
		return intVector;
	}


	//-----------------------------------------------------------------------------------------------
	template <>
	std::vector<float> GetArray(const std::string& arrayName)
	{
		std::vector<float> floatVector;
		lua_getglobal(m_luaVirtualMachine, arrayName.c_str());

		if (lua_isnil(m_luaVirtualMachine, -1))
		{
			ClearLuaStack();
			return floatVector;
		}

		lua_pushnil(m_luaVirtualMachine);

		while (lua_next(m_luaVirtualMachine, -2)) // Pops a key, pushes a key-value pair
		{
			if (!lua_isnumber(m_luaVirtualMachine, -1))
			{
				PrintLuaMessage(Stringf("Array \"%s\" has a non-number value at index %i", arrayName.c_str(), floatVector.size()));
			}

			floatVector.push_back((float)lua_tonumber(m_luaVirtualMachine, -1));
			lua_pop(m_luaVirtualMachine, 1); // Remove the value we just stored off
		}

		ClearLuaStack();

		return floatVector;
	}

	//-----------------------------------------------------------------------------------------------
	template <>
	std::vector<std::string> GetArray(const std::string& arrayName)
	{
		std::vector<std::string> stringVector;
		lua_getglobal(m_luaVirtualMachine, arrayName.c_str());

		if (lua_isnil(m_luaVirtualMachine, -1))
		{
			ClearLuaStack();
			return stringVector;
		}

		lua_pushnil(m_luaVirtualMachine);

		while (lua_next(m_luaVirtualMachine, -2)) // Pops a key, pushes a key-value pair
		{
			if (!lua_isstring(m_luaVirtualMachine, -1))
			{
				PrintLuaMessage(Stringf("Array \"%s\" has a non-string value at index %i", arrayName.c_str(), stringVector.size()));
			}

			stringVector.push_back(lua_tostring(m_luaVirtualMachine, -1));
			lua_pop(m_luaVirtualMachine, 1); // Remove the value we just stored off
		}

		ClearLuaStack();

		return stringVector;
	}


	//-----------------------------------------------------------------------------------------------
	template <>
	std::vector<bool> GetArray(const std::string& arrayName)
	{
		std::vector<bool> boolVector;
		lua_getglobal(m_luaVirtualMachine, arrayName.c_str());

		if (lua_isnil(m_luaVirtualMachine, -1))
		{
			ClearLuaStack();
			return boolVector;
		}

		lua_pushnil(m_luaVirtualMachine);

		while (lua_next(m_luaVirtualMachine, -2)) // Pops a key, pushes a key-value pair
		{
			if (!lua_isboolean(m_luaVirtualMachine, -1))
			{
				PrintLuaMessage(Stringf("Array \"%s\" has a non-boolean value at index %i", arrayName.c_str(), boolVector.size()));
			}

			boolVector.push_back(lua_toboolean(m_luaVirtualMachine, -1));
			lua_pop(m_luaVirtualMachine, 1); // Remove the value we just stored off
		}

		ClearLuaStack();

		return boolVector;
	}

	
private:
	//-----Private Data-----

	std::string	m_scriptFilePath;
	lua_State*	m_luaVirtualMachine = nullptr;

};


#endif // __LUA_INC_H__