


#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>



#define CCASSERT(a,b)  do {} while (0)

#include <string>
#include <vector>


#define CC_GL_DEPTH24_STENCIL8      -1


#define glDeleteVertexArrays            glDeleteVertexArraysAPPLE
#define glGenVertexArrays               glGenVertexArraysAPPLE
#define glBindVertexArray               glBindVertexArrayAPPLE
#define glClearDepthf                   glClearDepth
#define glDepthRangef                   glDepthRange
#define glReleaseShaderCompiler(xxx)
