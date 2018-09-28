#include "OpenGL.h"
#import <stdlib.h>
#import <string.h>
#include <dlfcn.h>

void* NSGLGetProcAddress(const char *functionName) {
    static void *handle = NULL;
    if (!handle) {
        handle = dlopen(NULL, RTLD_LAZY);
    }
    return handle ? (void*)dlsym(handle, functionName) : NULL;
}
int loadGL()
{
	return gladLoadGLLoader(NSGLGetProcAddress);
}
