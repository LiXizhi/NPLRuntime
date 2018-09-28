#include "OpenGL.h"


    PFNGLGETTEXIMAGEPROC para_glGetTexImage;
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
    para_glGetTexImage = 0;
    para_glUniformMatrix2x3fv=0;
    para_glUniformMatrix3x2fv=0;
    para_glUniformMatrix2x4fv=0;
    para_glUniformMatrix4x2fv=0;
    para_glUniformMatrix3x4fv=0;
    para_glUniformMatrix4x3fv=0;

    para_glGetTexLevelParameteriv=0;
    para_glDrawBuffers=0;  

	return 1;
}
