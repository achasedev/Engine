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

#endif // __LUA_INC_H__