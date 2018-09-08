/************************************************************************/
/* File: Mouse.cpp
/* Author: Andrew Chase
/* Date: March 27th, 2017
/* Description: Implementation of the Mouse class
/************************************************************************/
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include "Engine/Core/Window.hpp"
#include "Engine/Input/Mouse.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
TODO("Remove renderer here");

//-----------------------------------------------------------------------------------------------
// Resets mouse state and sets initial cursor position if necessary
//
void Mouse::BeginFrame()
{
	// Reset the mouse wheel
	m_currFrameWheel = 0.f;

	// Reset the 'just' data members before starting
	m_buttons[MOUSEBUTTON_LEFT].m_wasJustPressed = false;
	m_buttons[MOUSEBUTTON_LEFT].m_wasJustReleased = false;

	m_buttons[MOUSEBUTTON_RIGHT].m_wasJustPressed = false;
	m_buttons[MOUSEBUTTON_RIGHT].m_wasJustReleased = false;

	m_buttons[MOUSEBUTTON_MIDDLE].m_wasJustPressed = false;
	m_buttons[MOUSEBUTTON_MIDDLE].m_wasJustReleased = false;

	// Update the cursor position

	// Absolute Mode - I get mouse position - and I can potentially lock to the screen
	UpdateCursorPositions();

	// Relative mode -> I care about deltas - I reset to the center (meaning, mutually exclusive modes)
	if (m_cursorMode == CURSORMODE_RELATIVE) {
		m_lastFramePosition = GetCenterOfClientWindow(); 
		SetCursorPos(m_lastFramePosition.x, m_lastFramePosition.y);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns true if the given button was pressed at the start of this frame only
//
bool Mouse::WasButtonJustPressed(MouseButton button)
{
	return m_buttons[button].m_wasJustPressed;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the given button was released at the start of this frame only
//
bool Mouse::WasButtonJustReleased(MouseButton button)
{
	return m_buttons[button].m_wasJustReleased;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the given button is currently pressed
//
bool Mouse::IsButtonPressed(MouseButton button)
{
	return m_buttons[button].m_isPressed;
}


//-----------------------------------------------------------------------------------------------
// Callback for when a mouse button is pressed or released - updates all mouse button states
//
void Mouse::OnMouseButton(size_t wParam)
{
	// Update all buttons
	UpdateButtonState(MOUSEBUTTON_LEFT,		(unsigned short) wParam, MK_LBUTTON);
	UpdateButtonState(MOUSEBUTTON_RIGHT,	(unsigned short) wParam, MK_RBUTTON);
	UpdateButtonState(MOUSEBUTTON_MIDDLE,	(unsigned short) wParam, MK_MBUTTON);
}
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
// Callback for when the mouse wheel is turned - accumulates total delta_wheel over multiple messages
//
void Mouse::OnMouseWheel(size_t wParam)
{
	short wheel = GET_WHEEL_DELTA_WPARAM(wParam);
	float wheelDelta = (float) wheel * (1.f / (float) WHEEL_DELTA);

	m_currFrameWheel += wheelDelta;

	DebuggerPrintf("Mouse Wheel: %f\n", m_currFrameWheel);
}


//-----------------------------------------------------------------------------------------------
// Sets the cursor position to the one given
//
void Mouse::SetCursorPosition(const IntVector2& newPosition)
{
	m_currFramePosition = newPosition;
	SetCursorPos(newPosition.x, newPosition.y);
}


//-----------------------------------------------------------------------------------------------
// If shouldLock, clips the mouse to be within the client bounds. Otherwise, it frees the cursor
//
void Mouse::LockCursorToClient(bool shouldLock)
{
	if (!shouldLock)
	{
		ClipCursor(nullptr);	// Unlock the mouse
	}
	else
	{
		// Get the client bounds
		HWND hwnd = (HWND) Window::GetInstance()->GetHandle();

		RECT clientRect;
		GetClientRect(hwnd, &clientRect);

		POINT offset; 
		offset.x = 0; 
		offset.y = 0; 
		ClientToScreen( hwnd, &offset ); 

		clientRect.left		+= offset.x; 
		clientRect.right	+= offset.x; 
		clientRect.top		+= offset.y; 
		clientRect.bottom	+= offset.y; 

		// lock the mouse to the bounds
		ClipCursor(&clientRect);  
	}

	m_isCursorLocked = shouldLock;
}


//-----------------------------------------------------------------------------------------------
// Shows or hides the cursor based on shouldShow
//
void Mouse::ShowMouseCursor(bool shouldShow)
{
	int showHideCount = ShowCursor(shouldShow);

	if (shouldShow)
	{
		while (showHideCount <= 0)
		{
			showHideCount = ShowCursor(shouldShow);
		}
	}
	else
	{
		while (showHideCount >= 0)
		{
			showHideCount = ShowCursor(shouldShow);
		}
	}

	m_isCursorShown = shouldShow;
}


//-----------------------------------------------------------------------------------------------
// Sets the cursor behavior mode to the one given
//
void Mouse::SetCursorMode(CursorMode newMode)
{
	m_cursorMode = newMode;

	if (newMode == CURSORMODE_RELATIVE)
	{
		// Reset state to avoid first-frame bugs
		m_lastFramePosition = GetCenterOfClientWindow();
		m_currFramePosition = m_lastFramePosition;
		SetCursorPos(m_lastFramePosition.x, m_lastFramePosition.y);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the mouse position in client window coordinates
//
IntVector2 Mouse::GetCursorClientPosition()
{
	// Get it in desktop coordinates
	POINT clientPosition;
	clientPosition.x = m_currFramePosition.x;
	clientPosition.y = m_currFramePosition.y;

	HWND hwnd = (HWND) Window::GetInstance()->GetHandle();
	ScreenToClient(hwnd, &clientPosition);

	return IntVector2(clientPosition.x, clientPosition.y);

}


//-----------------------------------------------------------------------------------------------
// Returns the mouse position in desktop screen coordinates
//
IntVector2 Mouse::GetCursorDesktopPosition()
{
	return m_currFramePosition;
}


//-----------------------------------------------------------------------------------------------
// Returns the mouse's UI position in the system's default UI orthographic space
//
Vector2 Mouse::GetCursorUIPosition()
{
	AABB2 uiBounds = Renderer::GetUIBounds();
	AABB2 pixelBounds = Window::GetInstance()->GetWindowBounds();
	IntVector2 pixelPosition = GetCursorClientPosition();

	float xPos = RangeMapFloat((float) pixelPosition.x, pixelBounds.mins.x, pixelBounds.maxs.x, uiBounds.mins.x, uiBounds.maxs.x);
	float yPos = RangeMapFloat((float) pixelPosition.y, pixelBounds.mins.y, pixelBounds.maxs.y, uiBounds.maxs.y, uiBounds.mins.y);

	return Vector2(xPos, yPos);
}


//-----------------------------------------------------------------------------------------------
// Returns the mouse delta position as a change in screen pixels
//
IntVector2 Mouse::GetMouseDelta() const
{
	return (m_currFramePosition - m_lastFramePosition);
}


//-----------------------------------------------------------------------------------------------
// Returns the current frame's mouse wheel delta
//
float Mouse::GetMouseWheelDelta() const
{
	return m_currFrameWheel;
}


//-----------------------------------------------------------------------------------------------
// Returns whether the cursor is currently visible
//
bool Mouse::IsCursorShown() const
{
	return m_isCursorShown;
}


//-----------------------------------------------------------------------------------------------
// Returns whether the cursor is currently locked to the client window
//
bool Mouse::IsCursorLocked() const
{
	return m_isCursorLocked;
}


//-----------------------------------------------------------------------------------------------
// Returns the current behavior mode the cursor is in
//
CursorMode Mouse::GetCursorMode() const
{
	return m_cursorMode;
}


//-----------------------------------------------------------------------------------------------
// Sets the previous position to the old current, and updates the current to the mouse's actual
// position at the time this function is called
//
void Mouse::UpdateCursorPositions()
{
	// last frame position
	m_lastFramePosition = m_currFramePosition; 

	// Get the desktop position
	POINT desktopPosition;
	GetCursorPos(&desktopPosition);

	m_currFramePosition = IntVector2(desktopPosition.x, desktopPosition.y);
}


//-----------------------------------------------------------------------------------------------
// Returns the center position of the client window, in desktop coordinates
//
IntVector2 Mouse::GetCenterOfClientWindow() const
{
	HWND hwnd = (HWND)Window::GetInstance()->GetHandle();

	POINT offset;
	offset.x = 0;
	offset.y = 0;
	ClientToScreen(hwnd, &offset);

	IntVector2 windowDimensions = Window::GetInstance()->GetDimensions();

	offset.x += windowDimensions.x / 2; 
	offset.y += windowDimensions.y / 2;

	return IntVector2(offset.x, offset.y);
}


//-----------------------------------------------------------------------------------------------
// Updates the given button state based on the state flags
//
void Mouse::UpdateButtonState(MouseButton buttonToUpdate, unsigned short buttonFlags, unsigned short buttonMask)
{
	// Update the left button
	bool isCurrentlyPressed = ((buttonFlags & buttonMask) == buttonMask);
	bool justChanged = (isCurrentlyPressed != m_buttons[buttonToUpdate].m_isPressed);

	// Update the button's current pressed state
	m_buttons[buttonToUpdate].m_isPressed = isCurrentlyPressed;

	if (justChanged)
	{
		// Just changed and is pressed, so it was just pressed
		if (isCurrentlyPressed)
		{
			m_buttons[buttonToUpdate].m_wasJustPressed = true;
		}
		// Just changed and is not pressed, so it was just released
		else
		{
			m_buttons[buttonToUpdate].m_wasJustReleased = true;
		}
	}
}
