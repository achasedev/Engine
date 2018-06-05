/************************************************************************/
/* File: glfunctions.hpp
/* Author: Andrew Chase
/* Date: January 25th, 2018
/* Bugs: None
/* Description: File to hold gl function handles for the renderer to use
/************************************************************************/
#pragma once
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"

#include "ThirdParty/gl/glcorearb.h"
#include "ThirdParty/gl/wglext.h"
#include "ThirdParty/gl/glext.h"
#pragma comment(lib, "opengl32")	// Link in the OpenGL32.lib static library

// Members needed to create a modern context - declared here so header doesn't include windows.h
extern HMODULE gGLLibrary; 
extern HWND gGLwnd;			// window our context is attached to; 
extern HDC gHDC;			// our device context
extern HGLRC gGLContext;    // our rendering context; 

//-----For setting up and shutting down the modern render context, each should only be called once globally-----
bool	GLStartup();	
void    GLShutdown();

// Windows context creation functions
extern PFNWGLGETEXTENSIONSSTRINGARBPROC		wglGetExtensionsStringARB;
extern PFNWGLCHOOSEPIXELFORMATARBPROC		wglChoosePixelFormatARB;
extern PFNWGLCREATECONTEXTATTRIBSARBPROC	wglCreateContextAttribsARB;

// GL drawing functions
extern PFNGLCLEARPROC					glClear;
extern PFNGLCLEARCOLORPROC				glClearColor;

extern PFNGLENABLEPROC					glEnable;
extern PFNGLDISABLEPROC					glDisable;
extern PFNGLBLENDFUNCPROC				glBlendFunc;
extern PFNGLBLENDFUNCSEPARATEPROC		glBlendFuncSeparate;
extern PFNGLBLENDEQUATIONPROC			glBlendEquation;
extern PFNGLBLENDEQUATIONSEPARATEPROC	glBlendEquationSeparate;
extern PFNGLLINEWIDTHPROC				glLineWidth;
extern PFNGLPOLYGONMODEPROC				glPolygonMode;
extern PFNGLFRONTFACEPROC				glFrontFace;
extern PFNGLCULLFACEPROC				glCullFace;

// Uniforms and attributes
extern PFNGLGETATTRIBLOCATIONPROC		glGetAttribLocation;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC		glVertexAttribPointer;
extern PFNGLVERTEXATTRIBDIVISORPROC		glVertexAttribDivisor;
extern PFNGLGETUNIFORMLOCATIONPROC		glGetUniformLocation;
extern PFNGLUNIFORMMATRIX4FVPROC		glUniformMatrix4fv;
extern PFNGLUNIFORM1IPROC				glUniform1i;
extern PFNGLUNIFORM1UIPROC				glUniform1ui;
extern PFNGLUNIFORM1FPROC				glUniform1f;
extern PFNGLUNIFORM2FPROC				glUniform2f;
extern PFNGLUNIFORM3FPROC				glUniform3f;
extern PFNGLUNIFORM4FPROC				glUniform4f;
extern PFNGLUSEPROGRAMPROC				glUseProgram;
extern PFNGLDRAWARRAYSPROC				glDrawArrays;
extern PFNGLDRAWELEMENTSPROC			glDrawElements;
extern PFNGLDRAWARRAYSINSTANCEDPROC		glDrawArraysInstanced;
extern PFNGLDRAWELEMENTSINSTANCEDPROC	glDrawElementsInstanced;

extern PFNGLGETACTIVEUNIFORMNAMEPROC		glGetActiveUniformName;
extern PFNGLGETACTIVEUNIFORMBLOCKIVPROC		glGetActiveUniformBlockiv;
extern PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC	glGetActiveUniformBlockName;
extern PFNGLGETACTIVEUNIFORMSIVPROC			glGetActiveUniformsiv;

// GL shader functions
extern PFNGLCREATESHADERPROC		glCreateShader;
extern PFNGLDELETESHADERPROC		glDeleteShader;
extern PFNGLSHADERSOURCEPROC		glShaderSource;
extern PFNGLCOMPILESHADERPROC		glCompileShader;
extern PFNGLGETSHADERIVPROC			glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC	glGetShaderInfoLog;
extern PFNGLCREATEPROGRAMPROC		glCreateProgram;
extern PFNGLATTACHSHADERPROC		glAttachShader;
extern PFNGLLINKPROGRAMPROC			glLinkProgram;
extern PFNGLGETPROGRAMIVPROC		glGetProgramiv;
extern PFNGLDELETEPROGRAMPROC		glDeleteProgram;
extern PFNGLDETACHSHADERPROC		glDetachShader;
extern PFNGLGETPROGRAMINFOLOGPROC	glGetProgramInfoLog;

// For Vertex Array Objects
extern PFNGLGENVERTEXARRAYSPROC		glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC		glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC	glDeleteVertexArrays;
extern PFNGLISVERTEXARRAYPROC		glIsVertexArray;

// RenderBuffer
extern PFNGLGENBUFFERSPROC			glGenBuffers;
extern PFNGLBINDBUFFERPROC			glBindBuffer;
extern PFNGLBINDBUFFERBASEPROC		glBindBufferBase;
extern PFNGLBUFFERDATAPROC			glBufferData;
extern PFNGLDELETEBUFFERSPROC       glDeleteBuffers;

// FrameBuffer
extern PFNGLGENFRAMEBUFFERSPROC			glGenFramebuffers;
extern PFNGLDELETEFRAMEBUFFERSPROC		glDeleteFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC			glBindFramebuffer;
extern PFNGLFRAMEBUFFERTEXTUREPROC		glFramebufferTexture;
extern PFNGLDRAWBUFFERSPROC				glDrawBuffers;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC	glCheckFramebufferStatus;
extern PFNGLBLITFRAMEBUFFERPROC			glBlitFramebuffer;
extern PFNGLDEPTHFUNCPROC				glDepthFunc;
extern PFNGLDEPTHMASKPROC				glDepthMask;
extern PFNGLCLEARDEPTHFPROC				glClearDepthf;

// Loading Textures
extern PFNGLPIXELSTOREIPROC			glPixelStorei;
extern PFNGLTEXPARAMETERIPROC		glTexParameteri;
extern PFNGLGENTEXTURESPROC			glGenTextures;
extern PFNGLBINDTEXTUREPROC			glBindTexture;
extern PFNGLTEXIMAGE2DPROC			glTexImage2D;
extern PFNGLACTIVETEXTUREPROC		glActiveTexture;
extern PFNGLCOPYIMAGESUBDATAPROC	glCopyImageSubData;
extern PFNGLTEXSTORAGE2DPROC		glTexStorage2D;
extern PFNGLTEXSUBIMAGE2DPROC		glTexSubImage2D;
extern PFNGLDELETETEXTURESPROC		glDeleteTextures;	


// Generating samplers
extern PFNGLGENSAMPLERSPROC			glGenSamplers;
extern PFNGLSAMPLERPARAMETERIPROC	glSamplerParameteri;
extern PFNGLDELETESAMPLERSPROC		glDeleteSamplers;
extern PFNGLBINDSAMPLERPROC			glBindSampler;

// Misc
extern PFNGLGETERRORPROC			glGetError;
extern PFNGLREADPIXELSPROC			glReadPixels;


// For binding GL functions through macro
// Use this to deduce type of the pointer so we can cast; 
template <typename T>
void wglGetTypedProcAddress( T *out, char const *name ) 
{
	// Grab the function from the currently bound render context
	// most opengl 2.0+ features will be found here
	*out = (T) wglGetProcAddress(name); 

	if ((*out) == nullptr) {
		// if it is not part of wgl (the device), then attempt to get it from the GLL library
		// (most OpenGL functions come from here)
		*out = (T) GetProcAddress(gGLLibrary, name); 
	}

	ASSERT_RECOVERABLE(*out != nullptr, Stringf("Error: gl function \"%s\" could not bind correctly", name));
}

// Binding macro for GL functions
#define GL_BIND_FUNCTION(f)      wglGetTypedProcAddress( &f, #f )

// For GL error checking
bool GLCheckError(char const *file, int line);
bool GLFailed();
bool GLSucceeded();
#define GL_CHECK_ERROR()  GLCheckError( __FILE__, __LINE__ )
