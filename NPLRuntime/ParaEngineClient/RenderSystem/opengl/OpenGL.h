#pragma once
#ifdef __cplusplus 
extern "C" { 
#endif

#ifdef ANDROID
#include "gl_android_spec.h"
#else
#include "glad/glad.h"
#endif

extern int loadGL();
	
#ifdef __cplusplus
}
#endif



