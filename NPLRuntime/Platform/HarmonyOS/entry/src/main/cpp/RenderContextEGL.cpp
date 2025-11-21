#include "RenderContextEGL.h"
#include "RenderDeviceOpenGL.h"
#include "RenderSystem/opengl/OpenGL.h"

namespace ParaEngine
{
    IRenderContext* IRenderContext::Create()
    {
        return new RenderContextEGL();
    }

    RenderContextEGL::RenderContextEGL()
    {
    }

    RenderContextEGL::~RenderContextEGL()
    {
    }

    IRenderDevice* RenderContextEGL::CreateDevice(const RenderConfiguration& cfg)
    {
        if (!loadGL())
        {
            // LOG: Unable to load GL extensions
        }
        return new RenderDeviceOpenGL();
    }

    bool RenderContextEGL::ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg)
    {
        return true;
    }
}
