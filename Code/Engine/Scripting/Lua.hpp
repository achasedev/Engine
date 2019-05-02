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

	void PrintError(const std::string& error);
	bool GetToStack(const std::string& variableName);

	// Data Manipulation
	template <typename T>
	T Get(const std::string& variableName)
	{
		if (m_luaVirtualMachine == nullptr)
		{
			PrintError("Attempted to Get() on a script that isn't loaded");
			return GetDefault<T>();
		}

		T result;
		if (GetToStack(variableName))
		{
			result = lua_get<T>(variableName);
		}
		else
		{
			result = GetDefault<T>();
		}

		return result;
	}

	template <typename T>
	inline T lua_get(const std::string& variableName)
	{
		return 0;
	}


	template <>
	inline bool lua_get(const std::string& variableName)
	{
		if (!lua_isboolean(m_luaVirtualMachine, -1))
		{
			PrintError(Stringf("Attempted to get variable \"%s\" as a float but it's not a number", variableName.c_str()));
		}

		return (bool)lua_toboolean(m_luaVirtualMachine, -1);
	}

	template <>
	inline float lua_get(const std::string& variableName)
	{
		if (!lua_isnumber(m_luaVirtualMachine, -1))
		{
			PrintError(Stringf("Attempted to get variable \"%s\" as a float but it's not a number", variableName.c_str()));
		}

		return (float)lua_tonumber(m_luaVirtualMachine, -1);
	}

	template <>
	inline int lua_get(const std::string& variableName)
	{
		if (!lua_isnumber(m_luaVirtualMachine, -1))
		{
			PrintError(Stringf("Attempted to get variable \"%s\" as an int but it's not a number", variableName.c_str()));
		}

		return (int)lua_tonumber(m_luaVirtualMachine, -1);
	}

	template <>
	inline std::string lua_get(const std::string& variableName)
	{
		std::string value = "null";
		if (lua_isstring(m_luaVirtualMachine, -1))
		{
			value = std::string(lua_tostring(m_luaVirtualMachine, -1));
		}
		else
		{
			PrintError(Stringf("Attempted to get variable \"%s\" as a string but it's not a string", variableName.c_str()));
		}

		return value;
	}

	template <typename T>
	T GetDefault()
	{
		return 0;
	}

	

private:

	lua_State*	m_luaVirtualMachine = nullptr;
	int			m_stackLevel = 0;

};


class LuaVirtualMachine
{
public:

	static void Initialize();
	static void Shutdown();


private:

	LuaVirtualMachine();
	~LuaVirtualMachine();
	LuaVirtualMachine(const LuaVirtualMachine& copy) = delete;


private:

	lua_State* m_state		= nullptr;

	static std::vector<LuaScript*> m_luaScripts;
	static LuaVirtualMachine* s_instance;

};


#endif // __LUA_INC_H__