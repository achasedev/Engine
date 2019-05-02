/************************************************************************/
/* File: Lua.hpp
/* Author: Andrew Chase
/* Date: May 1st 2019
/* Description: Header for including components necessary for Lua scripting
/************************************************************************/
#ifndef __LUA_INC_H__
#define __LUA_INC_H__

extern "C"
{
#include "ThirdParty/lua/include/lua.h"
#include "ThirdParty/lua/include/lauxlib.h"
#include "ThirdParty/lua/include/lualib.h"
}

#pragma comment(lib, "ThirdParty/lua/lua53.lib")

#include <vector>
#include "Engine/Core/Utility/StringUtils.hpp"

class LuaScript
{
public:

	LuaScript(const char* filepath);
	~LuaScript();

	void PrintLuaMessage(const std::string& message) const;
	bool GetToStack(const std::string& variableName, int& out_numLevelsPushed);

	// Data Manipulation
	template <typename T>
	T Get(const std::string& variableName)
	{
		if (m_luaVirtualMachine == nullptr)
		{
			PrintLuaMessage("Attempted to Get() on a script that isn't loaded");
			return GetDefault<T>();
		}

		T result;
		int numLevelsPushed = 0;
		bool foundVariable = GetToStack(variableName, numLevelsPushed);

		if (foundVariable)
		{
			result = GetByType<T>(variableName);
		}
		else
		{
			result = GetDefault<T>();
		}

		// Reset the stack back to where it was before we started
		lua_pop(m_luaVirtualMachine, numLevelsPushed);

		return result;
	}

	template <typename T>
	inline T GetByType(const std::string& variableName)
	{
		return 0;
	}


	template <>
	inline bool GetByType(const std::string& variableName)
	{
		if (!lua_isboolean(m_luaVirtualMachine, -1))
		{
			PrintLuaMessage(Stringf("Attempted to get variable \"%s\" as a float but it's not a number", variableName.c_str()));
		}

		return (bool)lua_toboolean(m_luaVirtualMachine, -1);
	}

	template <>
	inline float GetByType(const std::string& variableName)
	{
		if (!lua_isnumber(m_luaVirtualMachine, -1))
		{
			PrintLuaMessage(Stringf("Attempted to get variable \"%s\" as a float but it's not a number", variableName.c_str()));
		}

		return (float)lua_tonumber(m_luaVirtualMachine, -1);
	}

	template <>
	inline int GetByType(const std::string& variableName)
	{
		if (!lua_isnumber(m_luaVirtualMachine, -1))
		{
			PrintLuaMessage(Stringf("Attempted to get variable \"%s\" as an int but it's not a number", variableName.c_str()));
		}

		return (int)lua_tonumber(m_luaVirtualMachine, -1);
	}

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

	template <typename T>
	T GetDefault()
	{
		return 0;
	}

	void PrintStack() const;
	

private:

	lua_State*	m_luaVirtualMachine = nullptr;

};


#endif // __LUA_INC_H__