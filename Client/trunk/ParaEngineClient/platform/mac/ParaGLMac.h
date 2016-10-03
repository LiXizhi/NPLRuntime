#ifndef PARA_GL_MAC_H
#define PARA_GL_MAC_H


#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import <OpenGL/glext.h>

#include <string>

#include "platform/PlatformMacro.h"


#include "CCType.h"


#define CC_GL_DEPTH24_STENCIL8      -1


#define glDeleteVertexArrays            glDeleteVertexArraysAPPLE
#define glGenVertexArrays               glGenVertexArraysAPPLE
#define glBindVertexArray               glBindVertexArrayAPPLE
#define glClearDepthf                   glClearDepth
#define glDepthRangef                   glDepthRange
#define glReleaseShaderCompiler(xxx)





#endif
