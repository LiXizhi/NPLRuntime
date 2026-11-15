#pragma once

#ifndef __GL_ANROID_SPEC_H__
#define __GL_ANROID_SPEC_H__

#ifndef GL_GLEXT_PROTOTYPES  
#define GL_GLEXT_PROTOTYPES 1  
#endif  

#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>

#define GL_BGRA GL_BGRA_EXT
#ifndef GL_DEPTH24_STENCIL8
#define GL_DEPTH24_STENCIL8 0x88F0
#endif
#define glClearDepth glClearDepthf

// OpenGL ES 3.0 provides native glGenVertexArrays / glBindVertexArray / glDeleteVertexArrays

#endif // __GL_ANROID_SPEC_H__