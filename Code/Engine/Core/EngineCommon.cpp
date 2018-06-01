/************************************************************************/
/* File: EngineCommon.cpp
/* Author: Andrew Chase
/* Date: October 28th, 2017
/* Bugs: None
/* Description: Defines the global variables shared across engine files
/************************************************************************/
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Blackboard.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

Blackboard* g_gameConfigBlackboard = nullptr;