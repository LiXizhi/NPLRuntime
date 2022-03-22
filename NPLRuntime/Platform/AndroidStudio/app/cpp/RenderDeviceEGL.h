//-----------------------------------------------------------------------------
// RenderDeviceEGL.h
// Authors: LanZhihong, big
// CreateDate: 2019.7.16
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

#pragma once

#include "RenderDeviceOpenGL.h"
#include <EGL/egl.h>

namespace ParaEngine
{
    class RenderDeviceEGL : public RenderDeviceOpenGL
    {
    public:
        RenderDeviceEGL();
        virtual ~RenderDeviceEGL();

        void Reset();
        virtual bool Present() override;
    };
}
