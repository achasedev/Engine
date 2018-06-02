/************************************************************************/
/* File: EngineCommon.cpp
/* Author: Andrew Chase
/* Date: October 28th, 2017
/* Bugs: None
/* Description: Defines the global variables shared across engine files
/************************************************************************/
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Utility/Blackboard.hpp"
#include "Engine/Rendering/Resources/SpriteSheet.hpp"

Blackboard* g_gameConfigBlackboard = nullptr;