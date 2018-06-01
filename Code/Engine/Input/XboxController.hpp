/************************************************************************/
/* Project: Game Engine
/* File: XboxController.hpp
/* Author: Andrew Chase
/* Date: September 18th, 2017
/* Bugs: None
/* Description: Class to represent an Xbox Controller, for input
/************************************************************************/
#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Input/KeyButtonState.hpp"


// Represents an analog stick on the controller
struct XboxStickState
{
	// The (x,y) axes of the stick
	Vector2 m_normalizedRawPosition = Vector2::ZERO;
	Vector2 m_normalizedCorrectedPosition = Vector2::ZERO;

	// The magnitude distance from the center position
	// (between 0.f and 1.f)
	float m_normalizedRawMagnitude = 0.f;
	float m_normalizedCorrectedMagnitude = 0.f;

	// The angular measure of the stick position, in degrees
	float m_orientationAngle = 0.f;

	float m_innerDeadzoneFraction = 0.3f;
	float m_outerDeadzoneFraction = 0.9f;
};


// Enumeration of the 14 buttons on the controller
enum XboxButtonID 
{
	XBOX_BUTTON_ERROR = -1,
	XBOX_BUTTON_A, 
	XBOX_BUTTON_B, 
	XBOX_BUTTON_X,
	XBOX_BUTTON_Y,
	XBOX_BUTTON_DPAD_UP,
	XBOX_BUTTON_DPAD_DOWN,
	XBOX_BUTTON_DPAD_LEFT,
	XBOX_BUTTON_DPAD_RIGHT,
	XBOX_BUTTON_THUMBCLICK_LEFT,
	XBOX_BUTTON_THUMBCLICK_RIGHT,
	XBOX_BUTTON_LB,
	XBOX_BUTTON_RB,
	XBOX_BUTTON_START,
	XBOX_BUTTON_BACK,
	NUM_XBOX_BUTTONS
};


// Enumeration for the two sticks
enum XboxStickID
{
	XBOX_STICK_ERROR = -1,
	XBOX_STICK_LEFT,
	XBOX_STICK_RIGHT,
	NUM_XBOX_STICKS
};


// Enumeration for the two triggers
enum XboxTriggerID
{
	XBOX_TRIGGER_ERROR = -1,
	XBOX_TRIGGER_LEFT,
	XBOX_TRIGGER_RIGHT,
	NUM_XBOX_TRIGGERS
};


class XboxController
{

public: // Methods

	// Default constructor, should NOT be used without a parameter!
	// Controller number should match its index in the array
	XboxController(int controllerNumber = 0);

	void Update();														// Fetches the controller input from XInput

	bool IsButtonPressed(XboxButtonID buttonID) const;					// Returns whether the button is currently pressed
	bool WasButtonJustPressed(XboxButtonID buttonID) const;				// Returns whether the button was just pressed this frame
	bool WasButtonJustReleased(XboxButtonID buttonID) const;			// Returns whether the button was just released this frame
	bool IsConnected() const;											// Returns whether the contoller is connected to the pc

	Vector2 GetRawStickPosition(XboxStickID stickID) const;				// Returns the raw stick (x, y) position
	Vector2 GetCorrectedStickPosition(XboxStickID stickID) const;		// Returns the corrected (x, y) position
	float GetStickOrientationDegrees(XboxStickID stickID) const;			// Returns the angle orientation of the stick
	float GetRawStickMagnitude(XboxStickID stickID) const;				// Returns the raw magnitude of the stick
	float GetCorrectedStickMagnitude(XboxStickID stickID) const;		// Returns the magnitude of the stick after compensating for deadzone

	float GetTriggerValue(XboxTriggerID triggerID) const;				// Gets the current value of the trigger, from 0.f to 1.f

public: // Data

	

private: // Methods

	// Updates the button state given the XInput information
	void UpdateButtonState(XboxButtonID buttonToUpdate, unsigned short buttonFlags, unsigned short buttonMask);

	// Updates the stick state and calculates the corrected stick information
	void UpdateStickState(XboxStickID stickToUpdate, short rawX, short rawY);

	// Updates the trigger state information
	void UpdateTriggerState(XboxTriggerID triggerToUpdate, unsigned char triggerValue);

	// Resets the 'just' flags of the KeyButtonStates
	void ResetButtonStates();

	void ResetStickStates();

	void ResetTriggerStates();


private: // Data

	int m_controllerNumber;							// The controllerID, between 0 and 3 inclusive
	bool m_isConnected;							// True when connected to the computer and XInput recognizes it

	KeyButtonState m_buttons[NUM_XBOX_BUTTONS];		// Buttons on the controller
	XboxStickState m_sticks[NUM_XBOX_STICKS];		// Sticks on the controller
	float m_triggers[NUM_XBOX_TRIGGERS];			// Triggers on the controller


	// Max and min values attained by the sticks, their value is a signed short
	static constexpr float MAX_AXIS = 32767.f;		
	static constexpr float MIN_AXIS = -32768.f;
};