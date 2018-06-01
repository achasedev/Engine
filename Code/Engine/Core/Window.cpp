/************************************************************************/
/* File: Window.cpp
/* Author: Andrew Chase
/* Date: January 30th, 2017
/* Description: Class to represent a Windows window
/************************************************************************/
#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include "Engine/Math/Vector2.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/EngineCommon.hpp"


// For singleton style classes, I like to had the instance variable within the CPP; 
Window* Window::s_instance = nullptr;

// Function declaration for the constructor's use
LRESULT CALLBACK WindowsMessageHandlingProcedure( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );

// C functions for setting up a window

//-----------------------------------------------------------------------------------------------
// Creates and returns a window class description, used for Window construction
//
WNDCLASSEX CreateWindowClassDescription()
{
	// Define a window style/class
	WNDCLASSEX windowClassDescription;
	memset( &windowClassDescription, 0, sizeof( windowClassDescription ) );
	windowClassDescription.cbSize = sizeof( windowClassDescription );
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast< WNDPROC >( WindowsMessageHandlingProcedure ); // Register our Windows message-handling function
	windowClassDescription.hInstance = GetModuleHandle( NULL );
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT( "Simple Window Class" );
	RegisterClassEx( &windowClassDescription );

	return windowClassDescription;
}


//-----------------------------------------------------------------------------------------------
// Returns a RECT of the desktop's width and height (monitor resolution)
//
RECT GetDesktopRect()
{
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect( desktopWindowHandle, &desktopRect );

	return desktopRect;
}


//-----------------------------------------------------------------------------------------------
// Creates a RECT describing the created window's bounds on the desktop (centered, of the correct size)
//
RECT GetClientRect(float desktopWidth, float desktopHeight, unsigned int windowWidth, unsigned int windowHeight)
{
	float clientMarginX = 0.5f * (desktopWidth - (float) windowWidth);
	float clientMarginY = 0.5f * (desktopHeight - (float) windowHeight);
	RECT clientRect;
	clientRect.left = (int) clientMarginX;
	clientRect.right = clientRect.left + (int) windowWidth;
	clientRect.top = (int) clientMarginY;
	clientRect.bottom = clientRect.top + (int) windowHeight;

	return clientRect;
}


//-----------------------------------------------------------------------------------------------
// Creates the Window's window instance and returns the handle for it
//
HWND FinalizeWindow(RECT windowRect, const std::string& windowTitle)
{
	WNDCLASSEX windowClassDescription = CreateWindowClassDescription();

	const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	AdjustWindowRectEx( &windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

	WCHAR windowTitleBuffer[ 1024 ];

	MultiByteToWideChar(GetACP(), 0, windowTitle.c_str(), -1, windowTitleBuffer, sizeof(windowTitle) / sizeof( windowTitle[0]));
	HWND activeWindow = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitleBuffer,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL );

	ShowWindow(activeWindow, SW_SHOW);
	SetForegroundWindow(activeWindow);
	SetFocus(activeWindow);

	HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
	SetCursor(cursor);

	return activeWindow;
}


//-----------------------------------------------------------------------------------------------
// Constructor for a window - sets the window to take up 90% of the screen on the min(width, height) axis
// 
Window::Window(float clientAspect/*=1.77777f*/, const std::string& windowTitle/*="NO TITLE SET"*/)
{
	s_instance = this;

	// Get desktop rect, dimensions, aspect
	RECT desktopRect = GetDesktopRect();

	// Calculations to determine exact window size
	float desktopWidth = (float)(desktopRect.right - desktopRect.left);
	float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
	float desktopAspect = desktopWidth / desktopHeight;

	// Calculate maximum client size (as some % of desktop size)
	constexpr float maxClientFractionOfDesktop = 0.90f;
	m_widthInPixels		= static_cast<unsigned int>(desktopWidth * maxClientFractionOfDesktop);
	m_heightInPixels	= static_cast<unsigned int>(desktopHeight * maxClientFractionOfDesktop);
	if(clientAspect > desktopAspect)
	{
		// Client window has a wider aspect than desktop; shrink client height to match its width
		m_heightInPixels = static_cast<unsigned int>(m_widthInPixels / clientAspect);
	}
	else
	{
		// Client window has a taller aspect than desktop; shrink client width to match its height
		m_widthInPixels = static_cast<unsigned int>(m_heightInPixels * clientAspect);
	}

	// Calculate client rect bounds by centering the client area
	RECT clientRect = GetClientRect(desktopWidth, desktopHeight, m_widthInPixels, m_heightInPixels);

	// Calculate the outer dimensions of the physical window, including frame et. al.
	m_hwnd = FinalizeWindow(clientRect, windowTitle);
	m_windowTitle = windowTitle;
}


//-----------------------------------------------------------------------------------------------
// Constructs a window of the given width and height, only shrinking to fit to window (preserving aspect)
//
Window::Window(unsigned int widthInPixels, unsigned int heightInPixels, const std::string& windowTitle/*="NO TITLE SET"*/)
{
	s_instance = this;
	
	// Get desktop rect, dimensions, aspect
	RECT desktopRect = GetDesktopRect();

	m_heightInPixels = heightInPixels;
	m_widthInPixels = widthInPixels;

	float desktopWidth = (float)(desktopRect.right - desktopRect.left);
	float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
	float clientAspect = (float) (m_widthInPixels) / (float) (m_heightInPixels);

	// Ensure the window fits within the desktop height
	if (m_heightInPixels > desktopHeight)
	{
		m_heightInPixels = (unsigned int)desktopHeight;
		m_widthInPixels = (unsigned int)(m_heightInPixels * clientAspect);
	}
	
	// Ensure the window fits within the desktop width
	if (m_widthInPixels > desktopWidth)
	{
		m_widthInPixels = (unsigned int)desktopWidth;
		m_heightInPixels = (unsigned int)(m_widthInPixels / clientAspect);
	}

	// Get the client bounds
	RECT clientRect = GetClientRect(desktopWidth, desktopHeight, m_widthInPixels, m_heightInPixels);

	// Create the window
	m_hwnd = FinalizeWindow(clientRect, windowTitle);
	m_windowTitle = windowTitle;
}


//-----------------------------------------------------------------------------------------------
// Message Pump for the system, is called when the OS receives an input interrupt and processes
// the input accordingly
//
LRESULT CALLBACK WindowsMessageHandlingProcedure( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	//  Give the custom handlers a chance to run first; 
	Window *window = Window::GetInstance(); 
	std::vector<windows_message_handler_cb> handlers = window->GetHandlers();

	bool returnDefaultProc = true;
	if (nullptr != window) 
	{
		for (int i = 0; i < static_cast<int>(handlers.size()); ++i) 
		{
			// If any return true then do default windows behavior
			returnDefaultProc = returnDefaultProc && handlers[i]( msg, wparam, lparam ); 
		}
	}

	if (returnDefaultProc)
	{
		return DefWindowProc( hwnd, msg, wparam, lparam );	// do default windows behaviour
	}
	else
	{
		return 0;
	}
}


//-----------------------------------------------------------------------------------------------
// Destructor, just sets the singleton instance to nullptr
//
Window::~Window()
{
	// Unregister all handlers on this window
	m_handlers.clear();
}


//-----------------------------------------------------------------------------------------------
// Constructs a new window for our system, throws an error if a singleton instance already exists
//
Window* Window::Initialize(float clientAspect/*=1.77777*/, const std::string& windowTitle/*="NO TITLE SET"*/)
{
	// Check if an instance already exists
	ASSERT_OR_DIE(s_instance == nullptr, "Error: Window::Initialize() called when an instance already exists.");

	// Window is created, assigns itself to s_instance internally (otherwise callback throws exception)
	return new Window(clientAspect, windowTitle);

}


//-----------------------------------------------------------------------------------------------
// Constructs a new window for the system, of the given width and height (fit to screen)
//
Window* Window::Initialize(unsigned int widthInPixels, unsigned int heightInPixels, const std::string& windowTitle/*="NO TITLE SET"*/)
{
	// Check if an instance already exists
	ASSERT_OR_DIE(s_instance == nullptr, "Error: Window constructor called when an instance already exists.");

	// Window is created, assigns itself to s_instance internally (otherwise callback throws exception)
	return new Window(widthInPixels, heightInPixels, windowTitle);

}


//-----------------------------------------------------------------------------------------------
// Called when the system is shutting down, just deletes our window instance
//
void Window::Shutdown()
{
	delete s_instance;
	s_instance = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Adds a listener to the vector of handlers to be called when an input interrupt is received
//
void Window::RegisterHandler(windows_message_handler_cb cb)
{
	m_handlers.push_back(cb);
}


//-----------------------------------------------------------------------------------------------
// Removes the given listener to the vector of handlers
//
void Window::UnregisterHandler(windows_message_handler_cb cb)
{
	for (int i = 0; i < static_cast<int>(m_handlers.size()); i++)
	{
		// Pointer comparison
		if (m_handlers[i] == cb)
		{
			m_handlers.erase(m_handlers.begin() + i);
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the vector of listeners of this window
//
std::vector<windows_message_handler_cb> Window::GetHandlers() const
{
	return m_handlers;
}


//-----------------------------------------------------------------------------------------------
// Returns the width of the window in pixels
//
unsigned int Window::GetWidthInPixels() const
{
	return m_widthInPixels;
}


//-----------------------------------------------------------------------------------------------
// Returns the height of the window in pixels
//
unsigned int Window::GetHeightInPixels() const
{
	return m_heightInPixels;
}


//-----------------------------------------------------------------------------------------------
// Returns the height of the window in pixels
//
float Window::GetWindowAspect() const
{
	return static_cast<float>(m_widthInPixels)/static_cast<float>(m_heightInPixels);
}


//-----------------------------------------------------------------------------------------------
// Returns the bounds of the window as an AABB2
//
AABB2 Window::GetWindowBounds() const
{
	return AABB2(Vector2::ZERO, Vector2((float) m_widthInPixels, (float) m_heightInPixels));
}


//-----------------------------------------------------------------------------------------------
// Sets the title of this window to the one given
//
void Window::SetTitle(const std::string& newTitle)
{
	m_windowTitle = newTitle;
	SetWindowTextA((HWND)m_hwnd, newTitle.c_str());
}


//-----------------------------------------------------------------------------------------------
// Returns the singleton window instance
//
Window* Window::GetInstance()
{
	return s_instance;
}
