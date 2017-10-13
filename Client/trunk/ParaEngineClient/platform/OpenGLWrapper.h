

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
USING_NS_CC;
#elif defined(PARA_PLATFORM_WIN32)
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "PEtypes.h"
#include "win32/GLType.h"
#include "win32/GLProgram.h"
#endif

#endif
