/************************************************************************/
/* File: glFunctions.cpp
/* Author: Andrew Chase
/* Date: January 25th, 2018
/* Bugs: None
/* Description: Initializes the gl functions handles to null
/************************************************************************/
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Rendering/OpenGL/glFunctions.hpp"
// Members needed to create a modern context
HMODULE gGLLibrary  = NULL; 
HWND gGLwnd         = NULL;    // window our context is attached to; 
HDC gHDC            = NULL;    // our device context
HGLRC gGLContext    = NULL;    // our rendering context; 

//----------Windows context creation functions----------
PFNWGLGETEXTENSIONSSTRINGARBPROC	wglGetExtensionsStringARB = nullptr;
PFNWGLCHOOSEPIXELFORMATARBPROC		wglChoosePixelFormatARB = nullptr;
PFNWGLCREATECONTEXTATTRIBSARBPROC	wglCreateContextAttribsARB = nullptr;

//----------GL drawing functions----------
PFNGLCLEARPROC			glClear = nullptr;
PFNGLCLEARCOLORPROC		glClearColor = nullptr;

PFNGLENABLEPROC					glEnable = nullptr;
PFNGLDISABLEPROC				glDisable = nullptr;
PFNGLBLENDFUNCPROC				glBlendFunc = nullptr;
PFNGLBLENDFUNCSEPARATEPROC		glBlendFuncSeparate = nullptr;
PFNGLBLENDEQUATIONPROC			glBlendEquation = nullptr;
PFNGLBLENDEQUATIONSEPARATEPROC	glBlendEquationSeparate = nullptr;
PFNGLLINEWIDTHPROC				glLineWidth = nullptr;
PFNGLPOLYGONMODEPROC			glPolygonMode = nullptr;
PFNGLFRONTFACEPROC				glFrontFace = nullptr;
PFNGLCULLFACEPROC				glCullFace = nullptr;

PFNGLGETATTRIBLOCATIONPROC			glGetAttribLocation = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC	glEnableVertexAttribArray = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC		glVertexAttribPointer = nullptr;
PFNGLVERTEXATTRIBDIVISORPROC		glVertexAttribDivisor = nullptr;
PFNGLGETUNIFORMLOCATIONPROC			glGetUniformLocation = nullptr;
PFNGLUNIFORMMATRIX4FVPROC			glUniformMatrix4fv = nullptr;
PFNGLUNIFORM1IPROC					glUniform1i = nullptr;
PFNGLUNIFORM1UIPROC					glUniform1ui = nullptr;
PFNGLUNIFORM1FPROC					glUniform1f = nullptr;
PFNGLUNIFORM2FPROC					glUniform2f = nullptr;
PFNGLUNIFORM3FPROC					glUniform3f = nullptr;
PFNGLUNIFORM4FPROC					glUniform4f = nullptr;
PFNGLUSEPROGRAMPROC					glUseProgram = nullptr;
PFNGLDRAWARRAYSPROC					glDrawArrays = nullptr;
PFNGLDRAWELEMENTSPROC				glDrawElements = nullptr;
PFNGLDRAWARRAYSINSTANCEDPROC		glDrawArraysInstanced = nullptr;
PFNGLDRAWELEMENTSINSTANCEDPROC		glDrawElementsInstanced = nullptr;

PFNGLGETACTIVEUNIFORMNAMEPROC		glGetActiveUniformName = nullptr;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC	glGetActiveUniformBlockiv = nullptr;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC	glGetActiveUniformBlockName = nullptr;
PFNGLGETACTIVEUNIFORMSIVPROC		glGetActiveUniformsiv = nullptr;

//----------GL Shader functions----------
PFNGLCREATESHADERPROC		glCreateShader = nullptr;
PFNGLDELETESHADERPROC		glDeleteShader = nullptr;
PFNGLSHADERSOURCEPROC		glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC		glCompileShader = nullptr;
PFNGLGETSHADERIVPROC		glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC	glGetShaderInfoLog = nullptr;
PFNGLCREATEPROGRAMPROC		glCreateProgram = nullptr;
PFNGLATTACHSHADERPROC		glAttachShader = nullptr;
PFNGLLINKPROGRAMPROC		glLinkProgram = nullptr;
PFNGLGETPROGRAMIVPROC		glGetProgramiv = nullptr;
PFNGLDELETEPROGRAMPROC		glDeleteProgram = nullptr;
PFNGLDETACHSHADERPROC		glDetachShader = nullptr;
PFNGLGETPROGRAMINFOLOGPROC	glGetProgramInfoLog = nullptr;

//----------Vertex Array Objects----------
PFNGLGENVERTEXARRAYSPROC	glGenVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC	glBindVertexArray = nullptr;
PFNGLDELETEVERTEXARRAYSPROC	glDeleteVertexArrays = nullptr;
PFNGLISVERTEXARRAYPROC		glIsVertexArray = nullptr;

//----------Render Buffer----------
PFNGLGENBUFFERSPROC			glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC			glBindBuffer = nullptr;
PFNGLBINDBUFFERBASEPROC		glBindBufferBase = nullptr;
PFNGLBUFFERDATAPROC			glBufferData = nullptr;
PFNGLDELETEBUFFERSPROC      glDeleteBuffers = nullptr;

//----------Frame Buffer----------
PFNGLGENFRAMEBUFFERSPROC			glGenFramebuffers = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC			glDeleteFramebuffers = nullptr;
PFNGLBINDFRAMEBUFFERPROC			glBindFramebuffer = nullptr;
PFNGLFRAMEBUFFERTEXTUREPROC			glFramebufferTexture = nullptr;
PFNGLDRAWBUFFERSPROC				glDrawBuffers = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC		glCheckFramebufferStatus = nullptr;
PFNGLBLITFRAMEBUFFERPROC			glBlitFramebuffer = nullptr;
PFNGLDEPTHFUNCPROC					glDepthFunc = nullptr;
PFNGLDEPTHMASKPROC					glDepthMask = nullptr;
PFNGLCLEARDEPTHFPROC				glClearDepthf = nullptr;
PFNGLVIEWPORTPROC					glViewport = nullptr;


//----------Textures----------
PFNGLPIXELSTOREIPROC		glPixelStorei = nullptr;
PFNGLTEXPARAMETERIPROC		glTexParameteri = nullptr;

PFNGLGENTEXTURESPROC		glGenTextures = nullptr;
PFNGLBINDTEXTUREPROC		glBindTexture = nullptr;
PFNGLTEXIMAGE2DPROC			glTexImage2D = nullptr;
PFNGLACTIVETEXTUREPROC		glActiveTexture = nullptr;
PFNGLCOPYIMAGESUBDATAPROC	glCopyImageSubData = nullptr;

PFNGLTEXSTORAGE2DPROC		glTexStorage2D = nullptr;
PFNGLTEXSUBIMAGE2DPROC		glTexSubImage2D = nullptr;
PFNGLDELETETEXTURESPROC		glDeleteTextures = nullptr;	
PFNGLGENERATEMIPMAPPROC		glGenerateMipmap = nullptr;


//-----Samplers-----
PFNGLGENSAMPLERSPROC		glGenSamplers = nullptr;
PFNGLSAMPLERPARAMETERIPROC	glSamplerParameteri = nullptr;
PFNGLDELETESAMPLERSPROC		glDeleteSamplers = nullptr;
PFNGLBINDSAMPLERPROC		glBindSampler = nullptr;
PFNGLSAMPLERPARAMETERFVPROC	glSamplerParameterfv = nullptr;


//-----Misc-----
PFNGLGETERRORPROC			glGetError = nullptr;
PFNGLREADPIXELSPROC			glReadPixels = nullptr;

//-----------------------------------------------------------------------------------------------
//----------------------------------------Local Functions----------------------------------------

//-----------------------------------------------------------------------------------------------
// Creates an old context to be used to bind gl functions
//
HGLRC CreateOldRenderContext(HDC hdc)
{
	// Setup the output to be able to render how we want
	// (in our case, an RGBA (4 bytes per channel) output that supports OpenGL
	// and is double buffered
	PIXELFORMATDESCRIPTOR pfd;
	memset( &pfd, 0, sizeof(pfd) ); 
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 0; // 24; Depth/Stencil handled by FBO
	pfd.cStencilBits = 0; // 8; DepthStencil handled by FBO
	pfd.iLayerType = PFD_MAIN_PLANE; // ignored now according to MSDN

									 // Find a pixel format that matches our search criteria above. 
	int pixel_format = ChoosePixelFormat( hdc, &pfd );
	if ( pixel_format == NULL ) {
		return NULL; 
	}

	// Set our HDC to have this output. 
	if (!SetPixelFormat( hdc, pixel_format, &pfd )) {
		return NULL; 
	}

	// Create the context for the HDC
	HGLRC context = wglCreateContext( hdc );
	if (context == NULL) {
		return NULL; 
	}

	// return the context; 
	return context; 
}


//------------------------------------------------------------------------
// Creates a real context as a specific version (major.minor) (the modern context)
//
HGLRC CreateRealRenderContext( HDC hdc, int major, int minor ) 
{
	// So similar to creating the temp one - we want to define 
	// the style of surface we want to draw to.  But now, to support
	// extensions, it takes key_value pairs
	int const format_attribs[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,    // The rc will be used to draw to a window
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,    // ...can be drawn to by GL
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,     // ...is double buffered
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB, // ...uses a RGBA texture
		WGL_COLOR_BITS_ARB, 24,             // 24 bits for color (8 bits per channel)
		// WGL_DEPTH_BITS_ARB, 24,          // if you wanted depth a default depth buffer...
		// WGL_STENCIL_BITS_ARB, 8,         // ...you could set these to get a 24/8 Depth/Stencil.
		NULL, NULL,                         // Tell it we're done.
	};

	// Given the above criteria, we're going to search for formats
	// our device supports that give us it.  I'm allowing 128 max returns (which is overkill)
	size_t const MAX_PIXEL_FORMATS = 128;
	int formats[MAX_PIXEL_FORMATS];
	int pixel_format = 0;
	UINT format_count = 0;

	BOOL succeeded = wglChoosePixelFormatARB( hdc, 
		format_attribs, 
		nullptr, 
		MAX_PIXEL_FORMATS, 
		formats, 
		(UINT*)&format_count );

	if (!succeeded) {
		return NULL; 
	}

	// Loop through returned formats, till we find one that works
	for (unsigned int i = 0; i < format_count; ++i) {
		pixel_format = formats[i];
		succeeded = SetPixelFormat( hdc, pixel_format, NULL ); // same as the temp context; 
		if (succeeded) {
			break;
		} else {
			DWORD error = GetLastError();
			DebuggerPrintf( "Failed to set the format: %u", error ); 
		}
	}

	if (!succeeded) {
		return NULL; 
	}

	// Okay, HDC is setup to the right format, now create our GL context

	// First, options for creating a debug context (potentially slower, but 
	// driver may report more useful errors). 
	int context_flags = 0; 
#if defined(_DEBUG)
	context_flags |= WGL_CONTEXT_DEBUG_BIT_ARB; 
#endif

	// describe the context
	int const attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, major,                             // Major GL Version
		WGL_CONTEXT_MINOR_VERSION_ARB, minor,                             // Minor GL Version
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,   // Restrict to core (no compatibility)
		WGL_CONTEXT_FLAGS_ARB, context_flags,                             // Misc flags (used for debug above)
		0, 0
	};

	// Try to create context
	HGLRC context = wglCreateContextAttribsARB( hdc, NULL, attribs );
	if (context == NULL) {
		return NULL; 
	}

	return context;
}


//-----------------------------------------------------------------------------------------------
// Binds the WGL functions needed to create a modern context, pulling them from the old context
//
void BindNewWGLFunctions()
{
	GL_BIND_FUNCTION( wglGetExtensionsStringARB ); 
	GL_BIND_FUNCTION( wglChoosePixelFormatARB ); 
	GL_BIND_FUNCTION( wglCreateContextAttribsARB );
}


//-----------------------------------------------------------------------------------------------
// Binds the GL functions we will be using to the current context, called after the modern context
// is created
//
void BindGLFunctions()
{
	// Drawing functions
	GL_BIND_FUNCTION(glClear);
	GL_BIND_FUNCTION(glClearColor);

	GL_BIND_FUNCTION(glEnable);
	GL_BIND_FUNCTION(glDisable);
	GL_BIND_FUNCTION(glBlendFunc);
	GL_BIND_FUNCTION(glBlendFuncSeparate);
	GL_BIND_FUNCTION(glBlendEquation);
	GL_BIND_FUNCTION(glBlendEquationSeparate);
	GL_BIND_FUNCTION(glLineWidth);
	GL_BIND_FUNCTION(glPolygonMode);
	GL_BIND_FUNCTION(glFrontFace);
	GL_BIND_FUNCTION(glCullFace);

	GL_BIND_FUNCTION(glGetAttribLocation);
	GL_BIND_FUNCTION(glEnableVertexAttribArray);
	GL_BIND_FUNCTION(glVertexAttribDivisor);
	GL_BIND_FUNCTION(glVertexAttribPointer);
	GL_BIND_FUNCTION(glGetUniformLocation);
	GL_BIND_FUNCTION(glUniformMatrix4fv);
	GL_BIND_FUNCTION(glUniform1i);
	GL_BIND_FUNCTION(glUniform1ui);
	GL_BIND_FUNCTION(glUniform1f);
	GL_BIND_FUNCTION(glUniform2f);
	GL_BIND_FUNCTION(glUniform3f);
	GL_BIND_FUNCTION(glUniform4f);

	GL_BIND_FUNCTION(glGetActiveUniformName);
	GL_BIND_FUNCTION(glGetActiveUniformBlockiv);
	GL_BIND_FUNCTION(glGetActiveUniformBlockName);
	GL_BIND_FUNCTION(glGetActiveUniformsiv);

	GL_BIND_FUNCTION(glUseProgram);
	GL_BIND_FUNCTION(glDrawArrays);
	GL_BIND_FUNCTION(glDrawElements);
	GL_BIND_FUNCTION(glDrawArraysInstanced);
	GL_BIND_FUNCTION(glDrawElementsInstanced);

	// Shader functions
	GL_BIND_FUNCTION(glCreateShader);
	GL_BIND_FUNCTION(glDeleteShader);
	GL_BIND_FUNCTION(glShaderSource);
	GL_BIND_FUNCTION(glCompileShader);
	GL_BIND_FUNCTION(glGetShaderiv);
	GL_BIND_FUNCTION(glGetShaderInfoLog);
	GL_BIND_FUNCTION(glCreateProgram);
	GL_BIND_FUNCTION(glAttachShader);
	GL_BIND_FUNCTION(glLinkProgram);
	GL_BIND_FUNCTION(glGetProgramiv);
	GL_BIND_FUNCTION(glDeleteProgram);
	GL_BIND_FUNCTION(glDetachShader);
	GL_BIND_FUNCTION(glGetProgramInfoLog);
	GL_BIND_FUNCTION(glDeleteProgram);

	// VAO
	GL_BIND_FUNCTION(glGenVertexArrays);
	GL_BIND_FUNCTION(glBindVertexArray);
	GL_BIND_FUNCTION(glDeleteVertexArrays);
	GL_BIND_FUNCTION(glIsVertexArray);

	// Render Buffer
	GL_BIND_FUNCTION(glGenBuffers);
	GL_BIND_FUNCTION(glBindBuffer);
	GL_BIND_FUNCTION(glBindBufferBase);
	GL_BIND_FUNCTION(glBufferData);
	GL_BIND_FUNCTION(glDeleteBuffers);

	// Frame Buffer
	GL_BIND_FUNCTION(glGenFramebuffers);
	GL_BIND_FUNCTION(glDeleteFramebuffers);
	GL_BIND_FUNCTION(glBindFramebuffer);
	GL_BIND_FUNCTION(glFramebufferTexture);
	GL_BIND_FUNCTION(glDrawBuffers);
	GL_BIND_FUNCTION(glCheckFramebufferStatus);
	GL_BIND_FUNCTION(glBlitFramebuffer);
	GL_BIND_FUNCTION(glDepthFunc);
	GL_BIND_FUNCTION(glDepthMask);
	GL_BIND_FUNCTION(glClearDepthf);
	GL_BIND_FUNCTION(glViewport);


	// Textures
	GL_BIND_FUNCTION(glPixelStorei);
	GL_BIND_FUNCTION(glTexParameteri);
	GL_BIND_FUNCTION(glGenTextures);
	GL_BIND_FUNCTION(glBindTexture);
	GL_BIND_FUNCTION(glTexImage2D); 
	GL_BIND_FUNCTION(glActiveTexture);
	GL_BIND_FUNCTION(glCopyImageSubData);
	GL_BIND_FUNCTION(glTexStorage2D);
	GL_BIND_FUNCTION(glTexSubImage2D);
	GL_BIND_FUNCTION(glDeleteTextures);	
	GL_BIND_FUNCTION(glGenerateMipmap);

	// Sampler generation
	GL_BIND_FUNCTION(glGenSamplers);
	GL_BIND_FUNCTION(glSamplerParameteri);
	GL_BIND_FUNCTION(glDeleteSamplers);
	GL_BIND_FUNCTION(glBindSampler);
	GL_BIND_FUNCTION(glSamplerParameterfv);

	// Misc
	GL_BIND_FUNCTION(glGetError);
	GL_BIND_FUNCTION(glReadPixels);
}


//-----------------------------------------------------------------------------------------------
// Gets the active window and creates a modern context to render with
//
bool GLStartup()
{
	// Ensure that RenderStartup is only called once, so the modern context is created once
	GUARANTEE_OR_DIE(gGLContext == NULL, "Error: GLStartup called after the context was already created.");

	// Get the active window to render to
	HWND hwnd = GetActiveWindow();

	// load and get a handle to the opengl dll (dynamic link library)
	gGLLibrary = LoadLibraryA("opengl32.dll"); 

	// Get the Device Context (DC) - how windows handles the interface to rendering devices
	// This "acquires" the resource - to cleanup, you must have a ReleaseDC(hwnd, hdc) call. 
	HDC hdc = GetDC(hwnd);       

	// use the DC to create a rendering context (handle for all OpenGL state - like a pointer)
	// This should be very similar to SD1
	HGLRC temp_context = CreateOldRenderContext(hdc); 

	wglMakeCurrent(hdc, temp_context); 
	BindNewWGLFunctions();  // find the functions we'll need to create the real context; 

	// create the real context, using opengl version 4.2
	HGLRC real_context = CreateRealRenderContext(hdc, 4, 2); 

	// Set and cleanup
	wglMakeCurrent(hdc, real_context); 
	wglDeleteContext(temp_context); 

	// Bind all our OpenGL functions we'll be using.
	BindGLFunctions(); 

	// set the globals
	gGLwnd = hwnd;
	gHDC = hdc; 
	gGLContext = real_context; 

	return true; 
}


//-----------------------------------------------------------------------------------------------
// Cleans up the GL context and libraries, only called when the program is exiting
// (Not necessary, but good to be in the habit of cleaning up)
//
void GLShutdown()
{
	wglMakeCurrent( gHDC, NULL ); 

	wglDeleteContext( gGLContext ); 
	ReleaseDC( gGLwnd, gHDC ); 

	gGLContext = NULL; 
	gHDC = NULL;
	gGLwnd = NULL; 

	FreeLibrary( gGLLibrary ); 
}


//-----------------------------------------------------------------------------------------------
// Checks if there was an OpenGL error (from a previous gl call) and prints to console if so
//
bool GLCheckError( char const *file, int line )
{
#if defined(_DEBUG)
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		DebuggerPrintf("GL ERROR [0x%04x] at [%s(%i)]\n", error, file, line);
		ConsoleErrorf("GL ERROR [0x%04x] at [%s(%i)]\n", error, file, line);
		return true; 
	}
#endif

	UNUSED(file);
	UNUSED(line);

	return false; 
}


//-----------------------------------------------------------------------------------------------
// Checks if there was an OpenGL error (from a previous gl call) and returns true if there was
//
bool GLFailed()
{
	return GLCheckError( __FILE__, __LINE__ ); 
}


//-----------------------------------------------------------------------------------------------
// Checks if there was an OpenGL error (from a previous gl call) and returns false if there wasn't
//
bool GLSucceeded()
{
	return !GLFailed();
}
