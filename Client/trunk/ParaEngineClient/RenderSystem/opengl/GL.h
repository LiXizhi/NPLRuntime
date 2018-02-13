#pragma once

#ifdef ANDROID

#ifndef GL_GLEXT_PROTOTYPES  
#define GL_GLEXT_PROTOTYPES 1  
#endif  
#include <GLES2/gl2platform.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

extern PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOESEXT;
extern PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOESEXT;
extern PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOESEXT;

#define GL_BGRA GL_BGRA_EXT
#define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
#define glClearDepth glClearDepthf

#define glBindVertexArray glBindVertexArrayOESEXT
#define glGenVertexArrays glGenVertexArraysOESEXT
#define glDeleteVertexArrays glDeleteVertexArraysOESEXT;

#else
#include "glad/glad.h"
#endif