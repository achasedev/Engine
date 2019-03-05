/************************************************************************/
/* Project: Game Engine
/* File: InputSystem.cpp
/* Author: Andrew Chase
/* Date: September 13th, 2017
/* Bugs: None
/* Description: Implementation of the InputSystem class
/************************************************************************/
#include "Engine/Core/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif		// Always #define this before #including <windows.h>
#include <windows.h>

// Singleton instance
InputSystem* InputSystem::s_instance = nullptr;

// Mapping InputSystem constants to Windows macros
const unsigned char	InputSystem::KEYBOARD_ESCAPE		= VK_ESCAPE;
const unsigned char InputSystem::KEYBOARD_SPACEBAR		= VK_SPACE;
const unsigned char	InputSystem::KEYBOARD_CONTROL		= VK_CONTROL;
const unsigned char	InputSystem::KEYBOARD_F1			= VK_F1;
const unsigned char	InputSystem::KEYBOARD_F2			= VK_F2;
const unsigned char	InputSystem::KEYBOARD_F3			= VK_F3;
const unsigned char	InputSystem::KEYBOARD_F4			= VK_F4;
const unsigned char	InputSystem::KEYBOARD_F5			= VK_F5;
const unsigned char	InputSystem::KEYBOARD_F6			= VK_F6;
const unsigned char	InputSystem::KEYBOARD_F7			= VK_F7;
const unsigned char	InputSystem::KEYBOARD_F8			= VK_F8;
const unsigned char	InputSystem::KEYBOARD_F9			= VK_F9;
const unsigned char	InputSystem::KEYBOARD_F10			= VK_F10;
const unsigned char	InputSystem::KEYBOARD_LEFT_ARROW	= VK_LEFT;
const unsigned char	InputSystem::KEYBOARD_UP_ARROW		= VK_UP;
const unsigned char	InputSystem::KEYBOARD_DOWN_ARROW	= VK_DOWN;
const unsigned char	InputSystem::KEYBOARD_RIGHT_ARROW	= VK_RIGHT;
const unsigned char InputSystem::KEYBOARD_TILDE			= VK_OEM_3;
const unsigned char	InputSystem::KEYBOARD_SHIFT			= VK_SHIFT;


//-----------------------------------------------------------------------------------------------
// Handles messages for input from windows
//
bool InputMessageHandler(unsigned int msg, size_t wparam, size_t lparam)
{
	UNUSED(lparam);

	// Process the message and pass to the input system if a key was pressed
	unsigned char keyCode = (unsigned char) wparam;
	switch( msg )
	{
	// Raw physical keyboard "key-was-just-pressed" event (case-insensitive, not translated)
	case WM_KEYDOWN:
	{
		// Process which key was pressed
		InputSystem::GetInstance()->OnKeyPressed(keyCode);
		break;
	}

	// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
	case WM_KEYUP:
	{
		// Process which key was released
		InputSystem::GetInstance()->OnKeyReleased(keyCode);
		break;
	}

	// Mouse input - all handled the same way
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		InputSystem::GetMouse().OnMouseButton(wparam);
		break;
	case WM_MOUSEWHEEL:
		InputSystem::GetMouse().OnMouseWheel(wparam);
		break;
	} 

	return true;
}


//-----------------------------------------------------------------------------------------------
// Constructor - Creates 4 XboxControllers with their ID's equal to their index in the array
//
InputSystem::InputSystem()
{
	for (int i = 0; i < NUM_CONTROLLERS; i++)
	{
		m_xboxControllers[i] = XboxController(i);
	}
}


//-----------------------------------------------------------------------------------------------
// Destructor - made private (use InputSystem::Shutdown() instead)
//
InputSystem::~InputSystem()
{
}


//-----------------------------------------------------------------------------------------------
// Processes all Windows messages (WM_xxx) for this app that have queued up since last frame.
// For each message in the queue, our WindowsMessageHandlingProcedure (or "WinProc") function
//	is called, telling us what happened (key up/down, minimized/restored, gained/lost focus, etc.)
//
void RunMessagePump()
{
	MSG queuedMessage;
	for( ;; )
	{
		const BOOL wasMessagePresent = PeekMessage( &queuedMessage, NULL, 0, 0, PM_REMOVE );
		if( !wasMessagePresent )
		{
			break;
		}

		TranslateMessage( &queuedMessage );
		DispatchMessage( &queuedMessage );
	}
}


//-----------------------------------------------------------------------------------------------
// Creates the InputSystem singleton instance
//
void InputSystem::Initialize()
{
	GUARANTEE_OR_DIE(s_instance == nullptr, "Error: InputSystem::Initialize() called with an existing instance.");
	s_instance = new InputSystem();

	// Set up the message handler
	Window::GetInstance()->RegisterHandler(InputMessageHandler);
}


//-----------------------------------------------------------------------------------------------
// Deletes the InputSystem singleton instance
//
void InputSystem::Shutdown()
{
	if (s_instance != nullptr)
	{
		delete s_instance;
		s_instance = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Resets the 'just' fields of all key states and setup up to receive messages from windows
//
void InputSystem::BeginFrame()
{
	m_mouse.BeginFrame();
	ResetJustKeyStates();
	UpdateControllers();
	RunMessagePump();
}


//-----------------------------------------------------------------------------------------------
// Handle End-of-frame tasks for the input system
// 
void InputSystem::EndFrame()
{
}


//-----------------------------------------------------------------------------------------------
// Called whenever a key (keyCode) was pressed, and updates its KeyButtonState
//
void InputSystem::OnKeyPressed(unsigned char keyCode)
{
	if (!m_keyStates[keyCode].m_isPressed)
	{
		m_keyStates[keyCode].m_wasJustPressed = true;
	}

	m_keyStates[keyCode].m_isPressed = true;
}


//-----------------------------------------------------------------------------------------------
// Called whenever a key (keyCode) was released, and updates its KeyButtonState
//
void InputSystem::OnKeyReleased(unsigned char keyCode)
{
	m_keyStates[keyCode].m_isPressed = false;
	
	m_keyStates[keyCode].m_wasJustReleased = true;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the key 'keyCode' is currently pressed this frame, false otherwise
//
bool InputSystem::IsKeyPressed(unsigned char keyCode) const
{
	return m_keyStates[keyCode].m_isPressed;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the key 'keyCode' was just pressed this frame, and false otherwise
//
bool InputSystem::WasKeyJustPressed(unsigned char keyCode) const
{
	return m_keyStates[keyCode].m_wasJustPressed;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the key 'keyCode' was currently released this frame, and false otherwise
bool InputSystem::WasKeyJustReleased(unsigned char keyCode) const
{
	return m_keyStates[keyCode].m_wasJustReleased;
}


//-----------------------------------------------------------------------------------------------
// Returns the controller given by the passed index
//
XboxController& InputSystem::GetController(int controllerNumber)
{
	return m_xboxControllers[controllerNumber];
}


//-----------------------------------------------------------------------------------------------
// Returns the InputSystem singleton instance
//
InputSystem* InputSystem::GetInstance()
{
	return s_instance;
}


//-----------------------------------------------------------------------------------------------
// Returns the mouse object for the input system
//
Mouse& InputSystem::GetMouse()
{
	return s_instance->m_mouse;
}


//-----------------------------------------------------------------------------------------------
// Returns the controller currently used by player one
//
XboxController& InputSystem::GetPlayerOneController()
{
	return s_instance->m_xboxControllers[0];
}


//-----------------------------------------------------------------------------------------------
// Sets the 'just' states of all KeyButtonStates to false, so that they may be updated correctly
// next frame
//
void InputSystem::ResetJustKeyStates()
{
	for (int i = 0; i < NUM_KEYS; i++)
	{
		m_keyStates[i].m_wasJustPressed = false;
		m_keyStates[i].m_wasJustReleased = false;
	}
}


//-----------------------------------------------------------------------------------------------
// Fetches the last-frame input for each controller from XInput
//
void InputSystem::UpdateControllers()
{
	for (int i = 0; i < NUM_CONTROLLERS; i++)
	{
		m_xboxControllers[i].Update();
	}
}


