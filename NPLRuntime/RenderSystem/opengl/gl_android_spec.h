#pragma once

#ifndef __GL_ANROID_SPEC_H__
#define __GL_ANROID_SPEC_H__

#ifndef GL_GLEXT_PROTOTYPES  
#define GL_GLEXT_PROTOTYPES 1  
#endif  

#include <GLES2/gl2platform.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

//#include <GLES3/gl3platform.h>
//#include <GLES3/gl3.h>
//#include <GLES3/gl3ext.h>
//#include <GLES3/gl31.h>

#define GL_BGRA GL_BGRA_EXT
#define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES

#define GL_CLAMP_TO_BORDER 0x812D
#define GL_TEXTURE_WIDTH 0x1000
#define GL_TEXTURE_HEIGHT 0x1001
#define GL_TEXTURE_BORDER_COLOR 0x1004
#define GL_RED 0x1903
#define GL_RG 0x8227
#define GL_DEPTH_STENCIL 0x84F9
#define GL_UNSIGNED_INT_24_8 0x84FA
#define GL_R16F 0x822D
#define GL_R32F 0x822E
#define GL_RGBA16F 0x881A
#define GL_TEXTURE_WRAP_R 0x8072
#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
#define GL_MAX_DRAW_BUFFERS 0x8824
#define GL_NUM_EXTENSIONS 0x821D


typedef void (GL_APIENTRYP PFNGLGETTEXIMAGEPROC)(GLenum target, GLint level, GLenum format, GLenum type, void *pixels);
typedef const GLubyte * (GL_APIENTRYP PFNGLGETSTRINGIPROC)(GLenum name, GLuint index);


typedef void (GL_APIENTRYP PFNGLUNIFORMMATRIX2X3FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern PFNGLUNIFORMMATRIX2X3FVPROC para_glUniformMatrix2x3fv;
#define glUniformMatrix2x3fv para_glUniformMatrix2x3fv

typedef void (GL_APIENTRYP PFNGLUNIFORMMATRIX3X2FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern PFNGLUNIFORMMATRIX3X2FVPROC para_glUniformMatrix3x2fv;
#define glUniformMatrix3x2fv para_glUniformMatrix3x2fv

typedef void (GL_APIENTRYP PFNGLUNIFORMMATRIX2X4FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern PFNGLUNIFORMMATRIX2X4FVPROC para_glUniformMatrix2x4fv;
#define glUniformMatrix2x4fv para_glUniformMatrix2x4fv

typedef void (GL_APIENTRYP PFNGLUNIFORMMATRIX4X2FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern PFNGLUNIFORMMATRIX4X2FVPROC para_glUniformMatrix4x2fv;
#define glUniformMatrix4x2fv para_glUniformMatrix4x2fv

typedef void (GL_APIENTRYP PFNGLUNIFORMMATRIX3X4FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern PFNGLUNIFORMMATRIX3X4FVPROC para_glUniformMatrix3x4fv;
#define glUniformMatrix3x4fv para_glUniformMatrix3x4fv

typedef void (GL_APIENTRYP PFNGLUNIFORMMATRIX4X3FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern PFNGLUNIFORMMATRIX4X3FVPROC para_glUniformMatrix4x3fv;
#define glUniformMatrix4x3fv para_glUniformMatrix4x3fv

typedef void (GL_APIENTRYP PFNGLGETTEXLEVELPARAMETERIVPROC)(GLenum target, GLint level, GLenum pname, GLint *params);
extern PFNGLGETTEXLEVELPARAMETERIVPROC para_glGetTexLevelParameteriv;
#define glGetTexLevelParameteriv para_glGetTexLevelParameteriv


typedef void (GL_APIENTRYP PFNGLDRAWBUFFERSPROC)(GLsizei n, const GLenum *bufs);
extern PFNGLDRAWBUFFERSPROC para_glDrawBuffers;
#define glDrawBuffers para_glDrawBuffers


extern PFNGLGENVERTEXARRAYSOESPROC para_glGenVertexArraysOES;
extern PFNGLBINDVERTEXARRAYOESPROC para_glBindVertexArrayOES;
extern PFNGLDELETEVERTEXARRAYSOESPROC para_glDeleteVertexArraysOES;
extern PFNGLGETTEXIMAGEPROC para_glGetTexImage;
extern PFNGLGETSTRINGIPROC para_glGetStringi;

#define glBindVertexArray para_glBindVertexArrayOES
#define glGenVertexArrays para_glGenVertexArraysOES
#define glDeleteVertexArrays para_glDeleteVertexArraysOES
#define glGetTexImage para_glGetTexImage;
#define glClearDepth glClearDepthf
#define glGetStringi para_glGetStringi
#endif // __GL_ANROID_SPEC_H__