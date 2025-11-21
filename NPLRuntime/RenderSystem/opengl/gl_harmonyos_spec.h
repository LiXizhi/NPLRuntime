#pragma once

#ifndef __GL_HARMONYOS_SPEC_H__
#define __GL_HARMONYOS_SPEC_H__

#ifndef GL_GLEXT_PROTOTYPES  
#define GL_GLEXT_PROTOTYPES 1  
#endif  

#include <GLES3/gl3.h>

// OpenGL ES 3.0 already defines GL_DEPTH24_STENCIL8 natively
#ifndef GL_DEPTH24_STENCIL8
#define GL_DEPTH24_STENCIL8 0x88F0
#endif

#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif

#define glClearDepth glClearDepthf

// HarmonyOS OpenGL ES 3.0 has native VAO support, no need for OES extensions

#endif // __GL_HARMONYOS_SPEC_H__
