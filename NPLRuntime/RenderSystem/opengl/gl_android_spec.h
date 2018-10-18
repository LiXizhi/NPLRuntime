#pragma once

#ifndef __GL_ANROID_SPEC_H__
#define __GL_ANROID_SPEC_H__

#ifndef GL_GLEXT_PROTOTYPES  
#define GL_GLEXT_PROTOTYPES 1  
#endif  

#include "glad/glad.h"


#undef GL_BGRA
#define GL_BGRA GL_BGRA_EXT
#undef GL_DEPTH24_STENCIL8
#define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
#undef GL_DEPTH_STENCIL
#define GL_DEPTH_STENCIL GL_DEPTH_STENCIL_OES
#undef GL_UNSIGNED_INT_24_8
#define GL_UNSIGNED_INT_24_8 GL_UNSIGNED_INT_24_8_OES

#undef glBindVertexArray
#define glBindVertexArray glBindVertexArrayOES
#undef glGenVertexArrays
#define glGenVertexArrays glGenVertexArraysOES
#undef glDeleteVertexArrays
#define glDeleteVertexArrays glDeleteVertexArraysOES
#undef glClearDepth
#define glClearDepth glClearDepthf

#endif // __GL_ANROID_SPEC_H__