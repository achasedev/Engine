/************************************************************************/
/* Project: Game Engine
/* File: InputSystem.hpp
/* Author: Andrew Chase
/* Date: September 13th, 2017
/* Bugs: None
/* Description: Class used for keyboard and Xbox Controller input
/************************************************************************/
#pragma once
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Input/XboxController.hpp"


class InputSystem
{

public:
	//-----Public Methods-----

	static void Initialize();
	static void Shutdown();

	void BeginFrame();										
	void EndFrame();

	void OnKeyPressed(unsigned char keyCode);		// Used WM_KeyDown events
	void OnKeyReleased(unsigned char keyCode);		// Used with WM_KeyUp events

	// Accessors for Keystates
	bool IsKeyPressed(unsigned char keyCode) const;
	bool WasKeyJustPressed(unsigned char keyCode) const;
	bool WasKeyJustReleased(unsigned char keyCode) const;

	// Accessor for controllers
	XboxController& GetController(int controllerNumber);

	// Accessor for the singleton instance
	static InputSystem* GetInstance();
	static XboxController& GetPlayerOneController();

private:
	//-----Private Methods-----

	InputSystem();		// Default - constructs four XboxController objects
	~InputSystem();		// Use shutdown publicly instead
	InputSystem(const InputSystem& copy) = delete;

	void ResetJustKeyStates();	// Resets the 'Just' key states every frame
	void UpdateControllers();	// Fetches controller input from XInput

public:
	//-----Public Data-----

	static const int NUM_KEYS = 256; 
	static const int NUM_CONTROLLERS = 4;
	static const unsigned char	KEYBOARD_ESCAPE;
	static const unsigned char	KEYBOARD_SPACEBAR;
	static const unsigned char	KEYBOARD_UP_ARROW;
	static const unsigned char	KEYBOARD_DOWN_ARROW;
	static const unsigned char	KEYBOARD_LEFT_ARROW;
	static const unsigned char	KEYBOARD_RIGHT_ARROW;
	static const unsigned char	KEYBOARD_F1;
	static const unsigned char	KEYBOARD_F2;
	static const unsigned char  KEYBOARD_F3;
	static const unsigned char  KEYBOARD_F4;
	static const unsigned char  KEYBOARD_F5;
	static const unsigned char  KEYBOARD_F6;
	static const unsigned char  KEYBOARD_F7;
	static const unsigned char  KEYBOARD_F8;
	static const unsigned char  KEYBOARD_F9;
	static const unsigned char  KEYBOARD_F10;
	static const unsigned char  KEYBOARD_TILDE;

private:
	//-----Private Data-----

	KeyButtonState m_keyStates[NUM_KEYS];
	XboxController m_xboxControllers[NUM_CONTROLLERS];

	static InputSystem* s_instance;	// The singleton InputSystem instance
};
