//-----------------------------------------------------------------------------
// RenderDeviceEGL.cpp
// Authors: LanZhihong, big
// CreateDate: 2019.7.16
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

#include "RenderDeviceEGL.h"

namespace ParaEngine {
#ifndef PICO_VR
    void RenderDeviceEGL::SetProfilingPresentation(bool enabled)
    {
        // GLSurfaceView owns this EGL surface and starts it at the EGL default of one.
        EGLDisplay display = eglGetCurrentDisplay();
        if (display != EGL_NO_DISPLAY && eglGetCurrentSurface(EGL_DRAW) != EGL_NO_SURFACE)
            eglSwapInterval(display, enabled ? 0 : 1);
    }
#endif

    RenderDeviceEGL::RenderDeviceEGL()
    {
    }

    RenderDeviceEGL::~RenderDeviceEGL()
    {
    }

    void RenderDeviceEGL::Reset()
    {
    }

    bool RenderDeviceEGL::Present()
    {
        return true;
    }
}
