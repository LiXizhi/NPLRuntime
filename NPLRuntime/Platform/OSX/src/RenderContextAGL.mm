#include "glad/glad.h"
#import <Cocoa/Cocoa.h>
#include "RenderContextAGL.h"
#include "ParaEngine.h"
#include "RenderWindowOSX.h"
#include "RenderDeviceAGL.h"
#include "RenderViewOSX.h"
#include "RenderSystem/rhi_core/RHIBackendRegistry.h"
#ifdef PARA_RHI_HAS_METAL
#include "RenderSystem/metal/RenderContextMetal.h"
#endif

using namespace ParaEngine;

namespace ParaEngine
{
    IRenderContext* CreateRenderContextAGL()
    {
        return new RenderContxtAGL();
    }
}

namespace
{
    struct OSXBackendAutoRegistrar
    {
        OSXBackendAutoRegistrar()
        {
#ifdef PARA_RHI_HAS_METAL
            RHIBackendRegistry::GetInstance().RegisterBackend(ERHIBackend::Metal, "Metal", &CreateRenderContextMetal, 100);
#endif
            RHIBackendRegistry::GetInstance().RegisterBackend(ERHIBackend::OpenGL, "OpenGL", &CreateRenderContextAGL, 50);
        }
    } s_osxBackendRegistrar;
}

#ifndef PARA_RHI_MULTI_BACKEND
IRenderContext* IRenderContext::Create()
{
    return CreateRenderContextAGL();
}
#endif

bool RenderContxtAGL::ResetDevice(ParaEngine::IRenderDevice *device, const ParaEngine::RenderConfiguration &cfg)
{
    return true;
}

using namespace ParaEngine;

ParaEngine::IRenderDevice *RenderContxtAGL::CreateDevice(const ParaEngine::RenderConfiguration &cfg)
{
    RenderWindowOSX* renderWindow = static_cast<RenderWindowOSX*>(cfg.renderWindow);
    
    NSOpenGLPixelFormatAttribute attrs[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        NSOpenGLPFAColorSize, 32,
        NSOpenGLPFADepthSize, 24,
        NSOpenGLPFAStencilSize, 8,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAAccelerated,
        0
    };
    
    NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    if (pixelFormat == nil)
    {
        NSLog(@"No valid matching OpenGL Pixel Format found");
        return nullptr;
    }
    
    NSOpenGLContext *openGLContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
    [openGLContext makeCurrentContext];
    if (!gladLoadGL())
    {
        NSLog(@"gladLoadGL failed");
        return nullptr;
    }
    GLuint initialVao = 0;
    if (glad_glGenVertexArrays != nullptr && glad_glBindVertexArray != nullptr)
    {
        glGenVertexArrays(1, &initialVao);
        glBindVertexArray(initialVao);
    }
    printf("OpenGL version supported by this platform (%s): \n",
           glGetString(GL_VERSION));
    
    NSWindow *window = (NSWindow*)renderWindow->GetNativeHandle();
    NSView *existingView = [window contentView];
    RenderView *view = nil;
    if (existingView && [existingView isKindOfClass:[RenderView class]])
    {
        view = (RenderView*)existingView;
    }
    else
    {
        NSRect bounds = existingView ? [existingView bounds] : CGRectMake(0, 0, cfg.screenWidth, cfg.screenHeight);
        view = [[[RenderView alloc] initWithFrame:bounds] autorelease];
        [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        [window setContentView:view];
    }
    [view setWantsBestResolutionOpenGLSurface:YES];
    [window makeFirstResponder:view];
    [openGLContext setView:view];
    [openGLContext update];
    
    RenderDeviceAGL *device = new RenderDeviceAGL(openGLContext);
    
    return device;
}
