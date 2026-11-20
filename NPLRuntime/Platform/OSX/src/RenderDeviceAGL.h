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
        
        NSOpenGLContext* GetGLContext() const { return m_GLContext; }
        virtual bool Present() override;
        
    private:
        NSOpenGLContext* m_GLContext;
        id m_frameObserver;
        id m_backingObserver;
    };
}
