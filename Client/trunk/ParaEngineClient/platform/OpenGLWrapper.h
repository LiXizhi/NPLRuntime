

#ifndef OPENGL_Wrapper_H

#define OPENGL_Wrapper_H

#include "ParaPlatformConfig.h"





#if defined(PLATFORM_MAC)

#include "platform/mac/ParaGLMac.h"
#include "platform/PlatformMacro.h"


#include "platform/mac/CCImage.h"
#include "platform/mac/CCLabel.h"
#include "platform/mac/CCGLProgram.h"
#include "platform/mac/CCTexture2D.h"


#else


#include "cocos2d.h"

#endif




#endif
