#pragma once

#ifndef __GL_ANROID_SPEC_H__
#define __GL_ANROID_SPEC_H__

#ifndef GL_GLEXT_PROTOTYPES  
#define GL_GLEXT_PROTOTYPES 1  
#endif  

#include "glad/glad.h"



#define GL_BGRA GL_BGRA_EXT
#define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
#define GL_DEPTH_STENCIL GL_DEPTH_STENCIL_OES
#define GL_UNSIGNED_INT_24_8 GL_UNSIGNED_INT_24_8_OES


extern PFNGLGENVERTEXARRAYSOESPROC para_glGenVertexArraysOES;
extern PFNGLBINDVERTEXARRAYOESPROC para_glBindVertexArrayOES;
extern PFNGLDELETEVERTEXARRAYSOESPROC para_glDeleteVertexArraysOES;

#define glBindVertexArray glBindVertexArrayOES
#define glGenVertexArrays glGenVertexArraysOES
#define glDeleteVertexArrays glDeleteVertexArraysOES
#define glClearDepth glClearDepthf

#endif // __GL_ANROID_SPEC_H__