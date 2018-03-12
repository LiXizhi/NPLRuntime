#pragma once
#ifdef __cplusplus 
extern "C" { 
#endif

#ifdef ANDROID
#include "gl_android_spec.h"
#elif _WIN32
#include "glad/glad.h"
#elif __APPLE__
#include "gl_osx_spec.h"
#endif

extern int loadGL();
	
#ifdef __cplusplus
}
#endif



