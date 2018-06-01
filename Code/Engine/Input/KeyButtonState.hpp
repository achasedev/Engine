/************************************************************************/
/* Project: Game Engine
/* File: KeyButtonState.hpp
/* Author: Andrew Chase
/* Date: September 19th, 2017
/* Bugs: None
/* Description: Struct used to represent any key/button state
/************************************************************************/
#pragma once

struct KeyButtonState
{

public:

	bool m_isPressed = false;
	bool m_wasJustPressed = false;
	bool m_wasJustReleased = false;
};
