/************************************************************************/
/* File: Window.hpp
/* Author: Andrew Chase
/* Date: January 30th, 2017
/* Description: Class to represent a Windows window
/************************************************************************/
#include "Engine/Math/AABB2.hpp"
#include <vector>

// Listeners to input that can be bound and passed input to, before the Engine gets it
typedef bool (*windows_message_handler_cb)( unsigned int msg, size_t wparam, size_t lparam ); 

class Window
{
public:
	//-----Public Methods-----

	void	RegisterHandler( windows_message_handler_cb cb );		// Adds the given listener to the list of listeners	
	void	UnregisterHandler( windows_message_handler_cb cb );		// Removes the given listener from the list of listeners
	void*	GetHandle() const { return m_hwnd; }					// Returns the hwnd reference
	void	SetTitle(const std::string& newTitle);					// Sets the title of the window

	std::vector<windows_message_handler_cb> GetHandlers() const;	// Returns the list of listeners for input

	unsigned int	GetWidthInPixels() const;
	unsigned int	GetHeightInPixels() const;
	float			GetWindowAspect() const;
	AABB2			GetWindowBounds() const;

	//-----Static Methods-----

	// Used to create the singleton instance (constructors private)
	static Window*	Initialize(float clientAspect=1.77777f, const std::string& windowTitle="NO TITLE SET");
	static Window*  Initialize(unsigned int widthInPixels, unsigned int heightInPixels, const std::string& windowTitle="NO TITLE SET");
	static void		Shutdown();

	// Getter for the singleton instance
	static Window* GetInstance();

	
private:
	//-----Private Methods-----

	// Creates a window, default aspect is 16:9, use InitializeWindow() to construct, not the constructor
	Window(float clientAspect=1.77777f, const std::string& windowTitle="NO TITLE SET"); 
	Window(unsigned int widthInPixels, unsigned int heightInPixels, const std::string& windowTitle="NO TITLE SET");
	~Window();
	Window(const Window& copy) = delete;


private:

	//-----Private Data-----
	void* m_hwnd; 

	// Windows message listeners 
	std::vector<windows_message_handler_cb> m_handlers; 

	// Title to be displayed on this window
	std::string m_windowTitle;

	// Width/height
	unsigned int m_widthInPixels;
	unsigned int m_heightInPixels;

	static Window* s_instance;	// The singleton Window instance
};
