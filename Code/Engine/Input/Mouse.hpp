/************************************************************************/
/* File: Mouse.hpp
/* Author: Andrew Chase
/* Date: March 27th, 2017
/* Description: Class to represent a mouse input device
/************************************************************************/
#pragma once
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Input/KeyButtonState.hpp"

// Enumeration for mouse buttons
enum MouseButton
{
	MOUSEBUTTON_LEFT,
	MOUSEBUTTON_RIGHT,
	MOUSEBUTTON_MIDDLE,
	NUM_MOUSEBUTTONS
};


// Enumeration for how the mouse cursor behaves
enum CursorMode
{
	CURSORMODE_ABSOLUTE,
	CURSORMODE_RELATIVE,
	NUM_CURSORMODE
};


class Mouse
{
public:
	//-----Public Methods-----

	void BeginFrame();	// For resetting state, and setting initial cursor position if necessary

	bool WasButtonJustPressed(MouseButton button);
	bool WasButtonJustReleased(MouseButton button);
	bool IsButtonPressed(MouseButton button);

	// Callbacks
	void OnMouseButton(size_t wParam);
	void OnMouseWheel(size_t wParam);

	// Mutators
	void SetCursorPosition(const IntVector2& newPosition);
	void LockCursorToClient(bool shouldLock);
	void ShowMouseCursor(bool shouldShow);
	void SetCursorMode(CursorMode newMode);

	// Accessors
	IntVector2	GetCursorClientPosition();
	IntVector2	GetCursorDesktopPosition();
	IntVector2	GetMouseDelta() const;
	float		GetMouseWheelDelta() const;

	bool		IsCursorShown() const;
	bool		IsCursorLocked() const;
	CursorMode	GetCursorMode() const;


private:
	//-----Private Methods-----

	void UpdateCursorPositions();
	void UpdateButtonState(MouseButton buttonToUpdate, unsigned short buttonFlags, unsigned short buttonMask);

	IntVector2 GetCenterOfClientWindow() const;


private:
	//-----Private Data-----

	// Cursor
	IntVector2 m_lastFramePosition = IntVector2::ZERO;
	IntVector2 m_currFramePosition = IntVector2::ZERO;
	CursorMode m_cursorMode = CURSORMODE_ABSOLUTE;

	// Cursor state
	bool m_isCursorShown = true;
	bool m_isCursorLocked = false;

	// Mouse wheel
	float m_currFrameWheel;

	// Buttons
	KeyButtonState m_buttons[NUM_MOUSEBUTTONS];		// Buttons on the mouse

};
