

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
//USING_NS_CC;	//this will cause a awful lot of problems on mobile(cocos2d) platform
#elif defined(PARA_PLATFORM_WIN32)
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "PEtypes.h"
#include "win32/GLType.h"
#include "win32/GLProgram.h"
#include "math/ParaMathUtility.h"
#include "win32/GLLabel.h"
#include "win32/GLFontAtlas.h"
#include "win32/GLTexture2D.h"
#include "win32/GLImage.h"
#endif

#if defined(PARAENGINE_MOBILE)
#define UNI_AUTO_RELEASE(ptr) ptr->autorelease();
#else
#define UNI_AUTO_RELEASE(ptr) ptr->AddToAutoReleasePool();
#endif

#if defined(PARAENGINE_MOBILE)
#define UNI_SAFE_RELEASE(ptr) CC_SAFE_RELEASE(ptr);
#else
#define UNI_SAFE_RELEASE(ptr) SAFE_RELEASE(ptr);
#endif

#if defined(PARAENGINE_MOBILE)
#define UNI_SAFE_RELEASE_NULL(ptr) CC_SAFE_RELEASE_NULL(ptr);
#else
#define UNI_SAFE_RELEASE(ptr) SAFE_RELEASE(ptr);
#endif

#if defined(PARAENGINE_MOBILE)
#define UNI_SAFE_RETAIN(ptr) CC_SAFE_RETAIN(ptr);
#else
#define UNI_SAFE_RETAIN(ptr) if(ptr) ptr->addref();
#endif

#endif
