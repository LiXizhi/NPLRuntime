//-----------------------------------------------------------------------------
// RenderContextEGL.cpp
// Authors: LanZhihong, big
// CreateDate: 2019.7.16
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "RenderContextEGL.h"
#include "RenderWindowAndroid.h"
#include "RenderDeviceOpenGL.h"
#include "RenderDeviceEGL.h"
#include <EGL/egl.h>
#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "ParaEngine", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "ParaEngine", __VA_ARGS__))

namespace ParaEngine {
    IRenderContext* IRenderContext::Create()
    {
        return new RenderContextEGL();
    }

    IRenderDevice* RenderContextEGL::CreateDevice(const RenderConfiguration & cfg)
    {
        if (!loadGL())
        {
            LOGW("Unable to load gl ext.");
        }

        auto version = glGetString(GL_VERSION);
        LOGI("GL_VERSION:%s", version);

        return new RenderDeviceEGL();
    }

    RenderContextEGL::RenderContextEGL()
    {

    }

    RenderContextEGL::~RenderContextEGL()
    {

    }

    bool RenderContextEGL::ResetDevice(IRenderDevice* device, const RenderConfiguration & cfg)
    {
        if (!loadGL())
        {
            LOGW("Unable to load gl ext.");
        }

        auto version = glGetString(GL_VERSION);
        LOGI("GL_VERSION:%s", version);

        RenderDeviceEGL* eglDevice = static_cast<RenderDeviceEGL*>(device);
        eglDevice->Reset();

        return true;
    }
}
