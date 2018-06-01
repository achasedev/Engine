/************************************************************************/
/* File: EngineCommon.hpp
/* Author: Andrew Chase
/* Date: October 28th, 2017
/* Bugs: None
/* Description: Header to be included in some Game and Engine files for
				Game metadata
/************************************************************************/
#pragma once
//#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

class Blackboard;
class Window;

extern Blackboard* g_gameConfigBlackboard;

// Macro to make TODO's and UNIMPLEMENTED reminders appear in build output
#define _QUOTE(x) # x
#define QUOTE(x) _QUOTE(x)
#define __FILE__LINE__ __FILE__ "(" QUOTE(__LINE__) ") : "
#define PRAGMA(p)  __pragma( p )
#define NOTE( x )  PRAGMA( message(x) )
#define FILE_LINE  NOTE( __FILE__LINE__ )

#define TODO( x )  NOTE( __FILE__LINE__"\n"           \
        " --------------------------------------------------------------------------------------\n" \
        "|  TODO :   " ##x "\n" \
        " --------------------------------------------------------------------------------------\n" )

#define UNIMPLEMENTED()  QUOTE(__FILE__) " (" QUOTE(__LINE__) ")" ; ERROR_AND_DIE("Function unimplemented!") 
#define UNUSED(x) (void)(x);
//-----------------------------------------------------------------------------------------------
