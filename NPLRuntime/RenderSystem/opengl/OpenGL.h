#pragma once
#ifdef __cplusplus 
extern "C" { 
#endif

#ifdef ANDROID
#include "gl_android_spec.h"
#elif EMSCRIPTEN
#define GL_GLEXT_PROTOTYPES
#include "SDL2/SDL_opengl.h"
#include "SDL2/SDL_opengl_glext.h"
#elif _WIN32 || LINUX
#include "glad/glad.h"
#elif __APPLE__
#import "TargetConditionals.h"
#if TARGET_OS_OSX
#include "gl_osx_spec.h"
#elif TARGET_OS_IPHONE
#include "gl_ios_spec.h"
#endif
#elif USE_OPENGL_RENDERER
#include "glad/glad.h"
#endif
extern int loadGL();
	
#ifdef __cplusplus
}
#endif



