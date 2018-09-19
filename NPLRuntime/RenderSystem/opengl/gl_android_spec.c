#include "OpenGL.h"
#include <EGL/egl.h>

PFNGLGENVERTEXARRAYSOESPROC para_glGenVertexArraysOES;
PFNGLBINDVERTEXARRAYOESPROC para_glBindVertexArrayOES;
PFNGLDELETEVERTEXARRAYSOESPROC para_glDeleteVertexArraysOES;
PFNGLGETTEXIMAGEPROC para_glGetTexImage;
PFNGLGETSTRINGIPROC para_glGetStringi;

PFNGLUNIFORMMATRIX2X3FVPROC para_glUniformMatrix2x3fv;
PFNGLUNIFORMMATRIX3X2FVPROC para_glUniformMatrix3x2fv;
PFNGLUNIFORMMATRIX2X4FVPROC para_glUniformMatrix2x4fv;
PFNGLUNIFORMMATRIX4X2FVPROC para_glUniformMatrix4x2fv;
PFNGLUNIFORMMATRIX3X4FVPROC para_glUniformMatrix3x4fv;
PFNGLUNIFORMMATRIX4X3FVPROC para_glUniformMatrix4x3fv;

PFNGLGETTEXLEVELPARAMETERIVPROC para_glGetTexLevelParameteriv;
PFNGLDRAWBUFFERSPROC para_glDrawBuffers;

int loadGL()
{
	para_glGenVertexArraysOES = (PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress("glGenVertexArraysOES");
	if (!para_glGenVertexArraysOES)return 0;
	para_glBindVertexArrayOES = (PFNGLBINDVERTEXARRAYOESPROC)eglGetProcAddress("glBindVertexArrayOES");
	if (!para_glGenVertexArraysOES)return 0;
	para_glDeleteVertexArraysOES = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glDeleteVertexArraysOES");
	if (!para_glGenVertexArraysOES)return 0;

	para_glGetTexImage = (PFNGLGETTEXIMAGEPROC)eglGetProcAddress("glGetTexImage");
	para_glGetStringi = (PFNGLGETSTRINGIPROC)eglGetProcAddress("glGetStringi");

	para_glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC)eglGetProcAddress("glUniformMatrix2x3fv");
	para_glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC)eglGetProcAddress("glUniformMatrix3x2fv");
	para_glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC)eglGetProcAddress("glUniformMatrix2x4fv");
	para_glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC)eglGetProcAddress("glUniformMatrix4x2fv");
	para_glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC)eglGetProcAddress("glUniformMatrix3x4fv");
	para_glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC)eglGetProcAddress("glUniformMatrix4x3fv");

	para_glGetTexLevelParameteriv = (PFNGLGETTEXLEVELPARAMETERIVPROC)eglGetProcAddress("glGetTexLevelParameteriv");

	para_glDrawBuffers = (PFNGLDRAWBUFFERSPROC)eglGetProcAddress("glDrawBuffers");


	return 1;
}