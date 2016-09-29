

#ifndef OPENGL_WARPPER_H

#define OPENGL_WARPPER_H

#include "ParaPlatformConfig.h"



#include "platform/ParaGL.h"

#if defined(PLATFORM_MAC)

#define USING_NS_CC
#define CC_SAFE_DELETE(p)           do { delete (p); (p) = nullptr; } while(0)
#define CC_SAFE_DELETE_ARRAY(p)     do { if(p) { delete[] (p); (p) = nullptr; } } while(0)
#define CC_SAFE_FREE(p)             do { if(p) { free(p); (p) = nullptr; } } while(0)
#define CC_SAFE_RELEASE(p)          do { if(p) { (p)->release(); } } while(0)
#define CC_SAFE_RELEASE_NULL(p)     do { if(p) { (p)->release(); (p) = nullptr; } } while(0)
#define CC_SAFE_RETAIN(p)           do { if(p) { (p)->retain(); } } while(0)
#define CC_BREAK_IF(cond)           if(cond) break


#include "platform/mac/ImageMac.h"


#else


#include "cocos2d.h"

#endif




#endif
