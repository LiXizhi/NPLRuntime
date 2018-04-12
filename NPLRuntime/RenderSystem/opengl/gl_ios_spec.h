#pragma once
#ifndef __GL_IOS_SPEC_H__
#define __GL_IOS_SPEC_H__
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#define glBindVertexArray glBindVertexArrayOES
#define glClearDepth glClearDepthf
#define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
//#define glGenVertexArrays glGenVertexArraysAPPLE
//#define glDeleteVertexArrays glDeleteVertexArraysAPPLE
#endif //__GL_IOS_SPEC_H__
