#include "OpenGL.h"
#include <EGL/egl.h>

int loadGL()
{
	return gladLoadGLES2Loader(eglGetProcAddress);
}