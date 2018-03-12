#pragma once
#include "RenderDeviceOpenGL.h"
@class NSOpenGLContext;
namespace ParaEngine
{
    class RenderDeviceAGL : public RenderDeviceOpenGL
    {
    public:
        RenderDeviceAGL(NSOpenGLContext* context);
        ~RenderDeviceAGL();
        
        virtual bool Present() override;
        
    private:
        NSOpenGLContext* m_GLContext;
    };
}
