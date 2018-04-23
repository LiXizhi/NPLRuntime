#import <UIKit/UIKit.h>

#include "ParaEngine.h"
#include "RenderContextAEGL.h"
#include "RenderWindowiOS.h"
#include "RenderDeviceOpenGL.h"
#include "RenderDeviceAEGL.h"

using namespace ParaEngine;

ParaEngine::IRenderContext* ParaEngine::IRenderContext::Create()
{
	return new RenderContextAEGL();
}

IRenderDevice* RenderContextAEGL::CreateDevice(const RenderConfiguration & cfg)
{
    RenderWindowiOS* renderWindow = static_cast<RenderWindowiOS*>(cfg.renderWindow);
    UIView* view = renderWindow->GetView();
    
    CAEAGLLayer* glLayer = (CAEAGLLayer*)view.layer;
    glLayer.opaque = YES;
    glLayer.contentsScale = [UIScreen mainScreen].scale;
    glLayer.drawableProperties = @{
                                   kEAGLDrawablePropertyRetainedBacking :[NSNumber numberWithBool:NO],
                                   kEAGLDrawablePropertyColorFormat : kEAGLColorFormatRGBA8
                                   };
    
    
    EAGLContext * glContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:glContext];
    
    RenderDeviceAEGL* device = new RenderDeviceAEGL(glContext,glLayer);
    
    return device;
}

bool RenderContextAEGL::ResetDevice(IRenderDevice* device, const RenderConfiguration & cfg)
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
