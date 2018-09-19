#include "OpenGL.h"
#include <EGL/egl.h>

PFNGLGENVERTEXARRAYSOESPROC para_glGenVertexArraysOES;
PFNGLBINDVERTEXARRAYOESPROC para_glBindVertexArrayOES;
PFNGLDELETEVERTEXARRAYSOESPROC para_glDeleteVertexArraysOES;
PFNGLGETTEXIMAGEPROC para_glGetTexImage;
PFNGLGETSTRINGIPROC para_glGetStringi;

extern PFNGLUNIFORMMATRIX2X3FVPROC para_glUniformMatrix2x3fv;
extern PFNGLUNIFORMMATRIX3X2FVPROC para_glUniformMatrix3x2fv;
extern PFNGLUNIFORMMATRIX2X4FVPROC para_glUniformMatrix2x4fv;
extern PFNGLUNIFORMMATRIX4X2FVPROC para_glUniformMatrix4x2fv;
extern PFNGLUNIFORMMATRIX3X4FVPROC para_glUniformMatrix3x4fv;
extern PFNGLUNIFORMMATRIX4X3FVPROC para_glUniformMatrix4x3fv;

int loadGL()
{
	para_glGenVertexArraysOES = (PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress("glGenVertexArraysOES");
	if (!para_glGenVertexArraysOES)return 0;
	para_glBindVertexArrayOES = (PFNGLBINDVERTEXARRAYOESPROC)eglGetProcAddress("glBindVertexArrayOES");
	if (!para_glGenVertexArraysOES)return 0;
	para_glDeleteVertexArraysOES = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glDeleteVertexArraysOES");
	if (!para_glGenVertexArraysOES)return 0;

	para_glGetTexImage = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glGetTexImage");
	para_glGetStringi = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glGetStringi");

	para_glUniformMatrix2x3fv = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glUniformMatrix2x3fv");
	para_glUniformMatrix3x2fv = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("para_glUniformMatrix3x2fv");
	para_glUniformMatrix2x4fv = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("para_glUniformMatrix2x4fv");
	para_glUniformMatrix2x3fv = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glUniformMatrix2x3fv");
	para_glUniformMatrix2x3fv = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glUniformMatrix2x3fv");
	para_glUniformMatrix2x3fv = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glUniformMatrix2x3fv");

	return 1;
}