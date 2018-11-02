#pragma once

#ifndef __GL_ANROID_SPEC_H__
#define __GL_ANROID_SPEC_H__

#ifndef GL_GLEXT_PROTOTYPES  
#define GL_GLEXT_PROTOTYPES 1  
#endif  

#include <GLES2/gl2platform.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define GL_BGRA GL_BGRA_EXT
#define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
#define glClearDepth glClearDepthf

extern PFNGLGENVERTEXARRAYSOESPROC para_glGenVertexArraysOES;
extern PFNGLBINDVERTEXARRAYOESPROC para_glBindVertexArrayOES;
extern PFNGLDELETEVERTEXARRAYSOESPROC para_glDeleteVertexArraysOES;

#define glBindVertexArray para_glBindVertexArrayOES
#define glGenVertexArrays para_glGenVertexArraysOES
#define glDeleteVertexArrays para_glDeleteVertexArraysOES

#endif // __GL_ANROID_SPEC_H__