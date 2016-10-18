


#if WIN32

#include <windows.h>


#include "../../../glew-2.0.0/include/GL/glew.h"

#include <GL/gl.h>
#include <GL/glu.h>


#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"opengl32.lib")



#define GLchar char
#define ssize_t int



#define CCLOG(...) do {} while (0)
#define CCASSERT(a,b)

//#define glActiveTexture(a)

/*
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_TEXTURE0 0


#define GL_BGRA  0x80E1

#define GL_BGRA_EXT                       0x80E1
#define GL_BGRA8_EXT                      0x93A1
#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1         0x8034
#define GL_UNSIGNED_SHORT_5_6_5           0x8363
*/

#else

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>



#define CCLOG(...)  do {} while (0)
#define CCASSERT(a,b)  do {} while (0)

#endif

#include <string>
#include <vector>




#define CC_GL_DEPTH24_STENCIL8      -1


#ifdef WIN32
#else

#define glDeleteVertexArrays            glDeleteVertexArraysAPPLE
#define glGenVertexArrays               glGenVertexArraysAPPLE
#define glBindVertexArray               glBindVertexArrayAPPLE
#define glClearDepthf                   glClearDepth
#define glDepthRangef                   glDepthRange
#define glReleaseShaderCompiler(xxx)

#endif
