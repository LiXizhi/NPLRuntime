#include "OpenGL.h"
#include <EGL/egl.h>

// HarmonyOS OpenGL ES 3.0 has native VAO support
int loadGL()
{
	// No extensions need to be loaded for GLES 3.0
	return 1;
}
