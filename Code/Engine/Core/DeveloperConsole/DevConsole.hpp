/************************************************************************/
/* File: DevConsole.hpp
/* Author: Andrew Chase
/* Date: February 3rd, 2018
/* Description: Class to represent a rendered Developer console screen
/************************************************************************/
#pragma once
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/DataStructures/ThreadSafeQueue.hpp"
#include <vector>

#define MAX_HISTORY_WRITE_COUNT 32

TODO("Max console line length, or line wrap");
class Renderer;
class BitmapFont;
class SpriteAnimSet;

struct ConsoleOutputText
{
	std::string m_text;
	Rgba m_color;
	int m_threadID; // Thread that called "ConsolePrintf"
};

// For the RemoteCommandService, and any other system that wants to listen
// to ConsolePrintf messages
typedef void(*DevConsole_cb)(ConsoleOutputText text, void* args);

struct DevConsoleHook_t
{
	DevConsoleHook_t(DevConsole_cb callbackP, void* argsP)
		: callback(callbackP), args(argsP) {}

	DevConsole_cb callback;
	void* args;
};

class DevConsole 
{
public:
	//-----Public Methods-----

	// Handles all input (what)
	void Update();

	// Renders the display
	void Render() const; 

	void Open(); 
	void Close(); 
	bool IsOpen(); 
	void AddToMessageQueue(ConsoleOutputText text);

	static std::vector<ConsoleOutputText> GetConsoleLog();
	static void ClearConsoleLog();

	static void AddCommandLineToHistory(const std::string& commandLine);

	void ProcessCharacterCode(unsigned char keyCode);
	void ProcessKeydownCode(unsigned char keyCode);

	void WriteHistoryToFile();

	static void			Initialize();
	static void			Shutdown();
	static DevConsole*	GetInstance(); 
	static bool			IsDevConsoleOpen();
	static void			ToggleConsole();
	static void			HideLogWindow();
	static void			ShowLogWindow();

	static void			AddConsoleHook(DevConsole_cb callback, void* args = nullptr);
	static void			RemoveConsoleHook(DevConsole_cb callback);

	void				FlushOutputQueue();


private:
	//-----Private Methods-----

	// Only construct through Initialize()
	DevConsole(); 
	~DevConsole(); 

	// Special case keys
	void HandleEnter();
	void HandleBackSpace();
	void HandleDelete();
	void HandleEscape();
	void HandleUpArrow();
	void HandleDownArrow();
	void AddCharacterToInputBuffer(unsigned char character);

	// Input
	void UpdateMouseCursorSettings();

	// Rendering
	void		RenderInputField(Renderer* renderer, BitmapFont* font) const;
	void		RenderLogWindow(Renderer* renderer, BitmapFont* font) const;
	void		RenderFPS() const;
	void		RenderFLChan() const;

	void		SetUpFLChan();


public:
	//-----Public Data-----

	static const Rgba DEFAULT_COMMAND_LOG_COLOR;	// Color of command lines when printed to the log (easier to distinguish from prints)
	static const Rgba DEFAULT_PRINT_LOG_COLOR;		// Default ConsolePrintf color

private: 
	//-----Private Data-----

	static DevConsole* s_instance;							// The single Console instance (singleton)
	bool m_isOpen;											// True if the console is opened
	bool m_showLog;											// Should the log window and text be drawn?

	std::string	m_inputBuffer;								// What is shown as the user is typing
	ThreadSafeQueue<ConsoleOutputText>	m_messageQueue;			// For thread safety
	std::vector<ConsoleOutputText>		m_consoleOutputLog;		// Log that is printed to screen as a list

	std::vector<std::string> m_commandHistory;				// List of previously entered strings (not the log)
	int	m_historyIndex;										// Current location in history to render		
	
	// Mouse settings
	bool		m_wasMouseShown;
	bool		m_wasMouseLocked;
	CursorMode	m_prevMouseMode;

	float	m_cursorBlinkTimer;			// Timer to track if we should draw the end cursor
	int		m_cursorPosition;			// Index position of the cursor in the input buffer

	AABB2		m_consoleLogBounds;		// Draw bounds for the log
	AABB2		m_inputFieldBounds;		// Bounds for the input field

	SpriteAnimSet*	m_FLChanAnimations;
	float			m_FLChanSecondsPerDance;

	std::vector<DevConsoleHook_t> m_consoleHooks;	// Callbacks to be called when a message is printed to the log

	// Constants
	static const Rgba INPUT_BOX_COLOR;
	static const Rgba INPUT_TEXT_COLOR;
	static const Rgba LOG_BOX_COLOR;

	static const float	TEXT_HEIGHT;				// Height of the text
	static const float	TEXT_PADDING;				// Amount of space between lines of text
	static const int	MAX_INPUT_BUFFER_SIZE;		// Max number of characters per command

	static const std::string LOCAL_LOG_FILENAME;	// Name of the default log file
	static const std::string LOCAL_LOGS_DIRECTORY;	// Path to the log directory relative to the executable's working directory
};


// Global stuff

// Should add a line of coloured text to the output 
void ConsolePrintf(const Rgba &color, char const *format, ... ); 

// Same as previous, be defaults to a color visible easily on your console
void ConsolePrintf(char const *format, ... ); 

// Writes the text to screen in yellow
void ConsoleWarningf(char const *format, ...);

// Writes the text to screen in blue
void ConsoleErrorf(char const *format, ...);