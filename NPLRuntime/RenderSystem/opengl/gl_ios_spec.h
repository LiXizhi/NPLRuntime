#pragma once
#ifndef __GL_IOS_SPEC_H__
#define __GL_IOS_SPEC_H__
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#define glClearDepth glClearDepthf
#ifndef GL_DEPTH24_STENCIL8
#define GL_DEPTH24_STENCIL8 0x88F0
#endif
#endif //__GL_IOS_SPEC_H__
