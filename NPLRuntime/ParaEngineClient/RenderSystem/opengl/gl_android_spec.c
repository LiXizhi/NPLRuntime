#include "OpenGL.h"
#include <EGL/egl.h>

PFNGLGENVERTEXARRAYSOESPROC para_glGenVertexArraysOES;
PFNGLBINDVERTEXARRAYOESPROC para_glBindVertexArrayOES;
PFNGLDELETEVERTEXARRAYSOESPROC para_glDeleteVertexArraysOES;

int loadGL()
{
	para_glGenVertexArraysOES = (PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress("glGenVertexArraysOES");
	if (!para_glGenVertexArraysOES)return 0;
	para_glBindVertexArrayOES = (PFNGLBINDVERTEXARRAYOESPROC)eglGetProcAddress("glBindVertexArrayOES");
	if (!para_glGenVertexArraysOES)return 0;
	para_glDeleteVertexArraysOES = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glDeleteVertexArraysOES");
	if (!para_glGenVertexArraysOES)return 0;
	return 1;
}