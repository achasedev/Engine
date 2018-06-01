/************************************************************************/
/* Project: Game Engine
/* File: XboxController.cpp
/* Author: Andrew Chase
/* Date: September 18th, 2017
/* Bugs: None
/* Description: Implementation of the XboxController class
/************************************************************************/
#include "Engine/Input/XboxController.hpp"
#include "Engine/Math/MathUtils.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#pragma comment( lib, "xinput9_1_0" ) // Link in the xinput.lib static library // #Eiserloh: Xinput 1_4 doesn't work in Windows 7; use 9_1_0 explicitly for broadest compatibility


//-----------------------------------------------------------------------------------------------
// Only constructor, - the controllerNumber should match the controller's index in the InputSystem Array
// 
XboxController::XboxController(int controllerNumber /* = 0 */)
{
	m_controllerNumber = controllerNumber;
}


//-----------------------------------------------------------------------------------------------
// Updates the controller states with the XInput values this last frame
// 
void XboxController::Update() 
{
	// Fetch the XInput information
	XINPUT_STATE xboxControllerState;
	memset(&xboxControllerState, 0, sizeof(xboxControllerState));

	DWORD errorStatus = XInputGetState(m_controllerNumber, &xboxControllerState);

	if (errorStatus == ERROR_SUCCESS)
	{
		// Is connected, so get its input
		m_isConnected = true;

		// Update all of the buttons
		WORD buttonFlags = xboxControllerState.Gamepad.wButtons;

		UpdateButtonState(XBOX_BUTTON_A,					buttonFlags, XINPUT_GAMEPAD_A);
		UpdateButtonState(XBOX_BUTTON_B,					buttonFlags, XINPUT_GAMEPAD_B);
		UpdateButtonState(XBOX_BUTTON_X,					buttonFlags, XINPUT_GAMEPAD_X);
		UpdateButtonState(XBOX_BUTTON_Y,					buttonFlags, XINPUT_GAMEPAD_Y);
		UpdateButtonState(XBOX_BUTTON_DPAD_UP,				buttonFlags, XINPUT_GAMEPAD_DPAD_UP);
		UpdateButtonState(XBOX_BUTTON_DPAD_DOWN,			buttonFlags, XINPUT_GAMEPAD_DPAD_DOWN);
		UpdateButtonState(XBOX_BUTTON_DPAD_LEFT,			buttonFlags, XINPUT_GAMEPAD_DPAD_LEFT);
		UpdateButtonState(XBOX_BUTTON_DPAD_RIGHT,			buttonFlags, XINPUT_GAMEPAD_DPAD_RIGHT);
		UpdateButtonState(XBOX_BUTTON_THUMBCLICK_LEFT,		buttonFlags, XINPUT_GAMEPAD_LEFT_THUMB);
		UpdateButtonState(XBOX_BUTTON_THUMBCLICK_RIGHT,		buttonFlags, XINPUT_GAMEPAD_RIGHT_THUMB);
		UpdateButtonState(XBOX_BUTTON_LB,					buttonFlags, XINPUT_GAMEPAD_LEFT_SHOULDER);
		UpdateButtonState(XBOX_BUTTON_RB,					buttonFlags, XINPUT_GAMEPAD_RIGHT_SHOULDER);
		UpdateButtonState(XBOX_BUTTON_START,				buttonFlags, XINPUT_GAMEPAD_START);
		UpdateButtonState(XBOX_BUTTON_BACK,					buttonFlags, XINPUT_GAMEPAD_BACK);

		// Update the sticks
		UpdateStickState(XBOX_STICK_LEFT,	xboxControllerState.Gamepad.sThumbLX, xboxControllerState.Gamepad.sThumbLY);
		UpdateStickState(XBOX_STICK_RIGHT,	xboxControllerState.Gamepad.sThumbRX, xboxControllerState.Gamepad.sThumbRY);


		// Update the triggers
		UpdateTriggerState(XBOX_TRIGGER_LEFT,	 xboxControllerState.Gamepad.bLeftTrigger);
		UpdateTriggerState(XBOX_TRIGGER_RIGHT,	 xboxControllerState.Gamepad.bRightTrigger);
	}
	else if (errorStatus == ERROR_DEVICE_NOT_CONNECTED)
	{
		m_isConnected = false;
		ResetButtonStates();
		ResetStickStates();
		ResetTriggerStates();
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the button given by buttonID with the XInput information this frame
// 
void XboxController::UpdateButtonState(XboxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonMask)
{
	KeyButtonState& currButton = m_buttons[buttonID];

	bool isCurrentlyPressed = ((buttonFlags & buttonMask) == buttonMask);
	bool justChanged = (isCurrentlyPressed != currButton.m_isPressed);

	// Update the button's current pressed state
	currButton.m_isPressed = isCurrentlyPressed;

	// Set the 'just' data members to false before starting
	currButton.m_wasJustPressed = false;
	currButton.m_wasJustReleased = false;

	if (justChanged)
	{
		// Just changed and is pressed, so it was just pressed
		if (isCurrentlyPressed)
		{
			currButton.m_wasJustPressed = true;
		}
		// Just changed and is not pressed, so it was just released
		else
		{
			currButton.m_wasJustReleased = true;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the stick given by stickID with the XInput information, and calculated corrected values
// 
void XboxController::UpdateStickState(XboxStickID stickID, short rawX, short rawY)
{
	XboxStickState& currStick = m_sticks[stickID];

	// Represent the raw input as a float between -1.f and 1.f
	float normalizedRawX = RangeMapFloat(static_cast<float>(rawX), MIN_AXIS, MAX_AXIS, -1.f, 1.f);
	float normalizedRawY = RangeMapFloat(static_cast<float>(rawY), MIN_AXIS, MAX_AXIS, -1.f, 1.f);

	currStick.m_normalizedRawPosition = Vector2(normalizedRawX, normalizedRawY);
	currStick.m_orientationAngle = currStick.m_normalizedRawPosition.GetOrientationDegrees();

	// Correct the magnitude through range mapping
	currStick.m_normalizedRawMagnitude = currStick.m_normalizedRawPosition.GetLength();
	float correctedMagnitude = RangeMapFloat(currStick.m_normalizedRawMagnitude, currStick.m_innerDeadzoneFraction, currStick.m_outerDeadzoneFraction, 0.f, 1.f);
	currStick.m_normalizedCorrectedMagnitude = ClampFloatZeroToOne(correctedMagnitude);

	// Calculate the correct X and Y
	float normalizedCorrectedX = currStick.m_normalizedCorrectedMagnitude * CosDegrees(currStick.m_orientationAngle);
	float normalizedCorrectedY = currStick.m_normalizedCorrectedMagnitude * SinDegrees(currStick.m_orientationAngle);

	// Set the corrected (x, y)
	currStick.m_normalizedCorrectedPosition = Vector2(normalizedCorrectedX, normalizedCorrectedY);
}


//-----------------------------------------------------------------------------------------------
// Updates the trigger given by triggerID with a float representation of it's value, between 
// 0.f (not pulled) and 1.f (completely pulled)
// 
void XboxController::UpdateTriggerState(XboxTriggerID triggerID, unsigned char triggerValue)
{
	m_triggers[triggerID] = static_cast<float>(triggerValue) * (1.f / 255.f);
}


//-----------------------------------------------------------------------------------------------
// Sets the KeyButtonState flags to false, used for when a controller is not connected
// 
void XboxController::ResetButtonStates()
{
	for (int i = 0; i < NUM_XBOX_BUTTONS; i++)
	{
		KeyButtonState& currbutton = m_buttons[i];
		currbutton.m_isPressed = false;
		currbutton.m_wasJustPressed = false;
		currbutton.m_wasJustReleased = false;
	}
}


//-----------------------------------------------------------------------------------------------
// Resets the stick states of this controller, used for when a controller disconnects
//
void XboxController::ResetStickStates()
{
	for (int i = 0; i < NUM_XBOX_STICKS; i++)
	{
		XboxStickState& currStick = m_sticks[i];

		currStick.m_normalizedRawPosition = Vector2::ZERO;
		currStick.m_normalizedCorrectedPosition = Vector2::ZERO;
		currStick.m_normalizedRawMagnitude = 0.f;
		currStick.m_normalizedCorrectedMagnitude = 0.f;
		currStick.m_orientationAngle = 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
// Resets the trigger values, used for when a controller disconnects
// 
void XboxController::ResetTriggerStates()
{
	for (int i = 0; i < NUM_XBOX_TRIGGERS; i++)
	{
		m_triggers[i] = 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
// Returns true if the button given by buttonID is currently pressed, and false otherwise
// 
bool XboxController::IsButtonPressed(XboxButtonID buttonID) const
{
	return m_buttons[buttonID].m_isPressed;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the button given by buttonID was just pressed this frame, and false otherwise
// 
bool XboxController::WasButtonJustPressed(XboxButtonID buttonID) const
{
	return m_buttons[buttonID].m_wasJustPressed;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the button given by buttonID was just released this frame, and false otherwise
// 
bool XboxController::WasButtonJustReleased(XboxButtonID buttonID) const
{
	return m_buttons[buttonID].m_wasJustReleased;
}


//-----------------------------------------------------------------------------------------------
// Accessor for the isConnected flag
// 
bool XboxController::IsConnected() const
{
	return m_isConnected;
}


//-----------------------------------------------------------------------------------------------
// Returns the raw (x,y) value of the stick given by stickID, x and y both between 0.f and 1.f
// 
Vector2 XboxController::GetRawStickPosition(XboxStickID stickID) const
{
	return m_sticks[stickID].m_normalizedRawPosition;
}


//-----------------------------------------------------------------------------------------------
// Returns the corrected (x,y) value of the stick given by stickID, x and y both between 0.f and 1.f
// 
Vector2 XboxController::GetCorrectedStickPosition(XboxStickID stickID) const
{
	return m_sticks[stickID].m_normalizedCorrectedPosition;
}


//-----------------------------------------------------------------------------------------------
// Returns the stick's orientation on a circle, in degrees
// 
float XboxController::GetStickOrientationDegrees(XboxStickID stickID) const
{
	return m_sticks[stickID].m_orientationAngle;
}


//-----------------------------------------------------------------------------------------------
// Returns the raw stick magnitude, mapped to 0.f and 1.f
//
float XboxController::GetRawStickMagnitude(XboxStickID stickID) const
{
	return m_sticks[stickID].m_normalizedRawMagnitude;
}

//-----------------------------------------------------------------------------------------------
// Returns the corrected stick magnitude, mapped to 0.f and 1.f
//
float XboxController::GetCorrectedStickMagnitude(XboxStickID stickID) const
{
	return m_sticks[stickID].m_normalizedCorrectedMagnitude;
}


//-----------------------------------------------------------------------------------------------
// Returns the value of the trigger given by triggerID (between 0.f and 1.f)
//
float XboxController::GetTriggerValue(XboxTriggerID triggerID) const
{
	return m_triggers[triggerID];
}
