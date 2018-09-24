#pragma once
#ifdef __cplusplus 
extern "C" { 
#endif

#ifdef ANDROID
#include "gl_android_spec.h"
#elif  defined(_WIN32) && defined(GLES)
#include "gl_android_spec.h"
#elif defined(_WIN32)
#include "glad/glad.h"
#elif __APPLE__
#import "TargetConditionals.h"
#if TARGET_OS_OSX
#include "gl_osx_spec.h"
#elif TARGET_OS_IPHONE
#include "gl_ios_spec.h"
#endif
#endif
extern int loadGL();
	
#ifdef __cplusplus
}
#endif



