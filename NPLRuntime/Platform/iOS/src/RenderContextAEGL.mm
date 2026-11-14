#import <UIKit/UIKit.h>

#include "ParaEngine.h"
#include "RenderContextAEGL.h"
#include "RenderWindowiOS.h"
#include "RenderDeviceOpenGL.h"
#include "RenderDeviceAEGL.h"
#include "RenderSystem/rhi_core/RHIBackendRegistry.h"
#ifdef PARA_RHI_HAS_METAL
#include "RenderSystem/metal/RenderContextMetal.h"
#endif

using namespace ParaEngine;

namespace ParaEngine
{
    IRenderContext* CreateRenderContextAEGL()
    {
        return new RenderContextAEGL();
    }
}

namespace
{
    struct IOSBackendAutoRegistrar
    {
        IOSBackendAutoRegistrar()
        {
#ifdef PARA_RHI_HAS_METAL
            RHIBackendRegistry::GetInstance().RegisterBackend(ERHIBackend::Metal, "Metal", &CreateRenderContextMetal, 100);
#endif
            RHIBackendRegistry::GetInstance().RegisterBackend(ERHIBackend::OpenGL, "OpenGL", &CreateRenderContextAEGL, 50);
        }
    } s_iosBackendRegistrar;
}

#ifndef PARA_RHI_MULTI_BACKEND
ParaEngine::IRenderContext* ParaEngine::IRenderContext::Create()
{
	return CreateRenderContextAEGL();
}
#endif

IRenderDevice* RenderContextAEGL::CreateDevice(const RenderConfiguration & cfg)
{
    RenderWindowiOS* renderWindow = static_cast<RenderWindowiOS*>(cfg.renderWindow);
    if (!renderWindow) {
        return nullptr;
    }
    
    UIView* view = renderWindow->GetView();
    if (!view) {
        return nullptr;
    }

    CAEAGLLayer* glLayer = (CAEAGLLayer*)view.layer;
    glLayer.opaque = YES;
    glLayer.contentsScale = [UIScreen mainScreen].scale;
    glLayer.drawableProperties =
      @{
        kEAGLDrawablePropertyRetainedBacking :[NSNumber numberWithBool:NO],
        kEAGLDrawablePropertyColorFormat : kEAGLColorFormatRGBA8
      };

    EAGLContext * glContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:glContext];

    RenderDeviceAEGL* device = new RenderDeviceAEGL(glContext, glLayer);

    return device;
}

bool RenderContextAEGL::ResetDevice(IRenderDevice *device, const RenderConfiguration &cfg)
{
	return true;
}

ParaEngine::RenderContextAEGL::RenderContextAEGL()
{
}

ParaEngine::RenderContextAEGL::~RenderContextAEGL()
{
	ReleaseContext();
}

void ParaEngine::RenderContextAEGL::ReleaseContext()
{
}
