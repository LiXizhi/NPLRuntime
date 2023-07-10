#include "ParaEngine.h"

#include "RenderContextAGL.h"
#include "RenderWindowOSX.h"
#include "RenderDeviceAGL.h"
#include "RenderViewOSX.h"

#import <Cocoa/Cocoa.h>
using namespace ParaEngine;

IRenderContext* IRenderContext::Create()
{
    auto context = new RenderContxtAGL();
    return context;
}

bool RenderContxtAGL::ResetDevice(ParaEngine::IRenderDevice *device, const ParaEngine::RenderConfiguration &cfg)
{
    return true;
}

using namespace ParaEngine;

ParaEngine::IRenderDevice *RenderContxtAGL::CreateDevice(const ParaEngine::RenderConfiguration &cfg)
{
    RenderWindowOSX* renderWindow = static_cast<RenderWindowOSX*>(cfg.renderWindow);
    
    NSOpenGLPixelFormatAttribute attrs[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
        NSOpenGLPFAColorSize,32,
        NSOpenGLPFADepthSize,16,
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
    printf("OpenGL version supported by this platform (%s): \n",
           glGetString(GL_VERSION));
    
    NSWindow *window = (NSWindow*)renderWindow->GetNativeHandle();
    RenderView *view = [[RenderView alloc] initWithFrame:CGRectMake(0, 0, cfg.screenWidth, cfg.screenHeight)];
    
    [window setContentView:view];
    [openGLContext setView:view];
    [openGLContext update];
    
    RenderDeviceAGL *device = new RenderDeviceAGL(openGLContext);
    
    return device;
}
