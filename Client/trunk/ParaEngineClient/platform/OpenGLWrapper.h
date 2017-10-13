

#ifndef OPENGL_Wrapper_H

#define OPENGL_Wrapper_H

#include "ParaPlatformConfig.h"



#if defined(PLATFORM_MAC)

#include "platform/PlatformMacro.h"
#include "platform/mac/CCImage.h"
#include "platform/mac/CCLabel.h"
#include "platform/mac/CCGLProgram.h"
#include "platform/mac/CCTexture2D.h"
#include "Platform/mac/CCFontAtlas.h"

#elif defined(PARAENGINE_MOBILE)
#include "cocos2d.h"

#elif defined(PARA_PLATFORM_WIN32)
	#include <gl.h>
	#include <glu.h>
	#include <glext.h>

	#define CCASSERT(a,b)  do {} while (0)
	#include <string>
	#include <vector>

	#define CC_GL_DEPTH24_STENCIL8      -1


	#define glDeleteVertexArrays            glDeleteVertexArraysAPPLE
	#define glGenVertexArrays               glGenVertexArraysAPPLE
	#define glBindVertexArray               glBindVertexArrayAPPLE
	#define glClearDepthf                   glClearDepth
	#define glDepthRangef                   glDepthRange
	#define glReleaseShaderCompiler(xxx)

#endif




#endif
