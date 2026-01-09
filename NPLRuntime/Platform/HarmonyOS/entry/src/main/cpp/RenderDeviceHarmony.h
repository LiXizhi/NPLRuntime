#pragma once
#include "RenderSystem/opengl/RenderDeviceOpenGL.h"

namespace ParaEngine
{
    class RenderDeviceHarmony : public RenderDeviceOpenGL
    {
    public:
        RenderDeviceHarmony();
        virtual ~RenderDeviceHarmony();
        
        virtual bool Present() override;
    };
}
