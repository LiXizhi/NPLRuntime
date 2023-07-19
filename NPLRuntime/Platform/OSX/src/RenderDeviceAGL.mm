#import <Cocoa/Cocoa.h>
#include "RenderDeviceAGL.h"

using namespace ParaEngine;
RenderDeviceAGL::RenderDeviceAGL(NSOpenGLContext *context)
:m_GLContext(context)
{
    
}

RenderDeviceAGL::~RenderDeviceAGL()
{
    m_GLContext = nullptr;
}

bool RenderDeviceAGL::Present()
{
    [m_GLContext makeCurrentContext];
    [m_GLContext update];
    [m_GLContext flushBuffer];
    return true;
}



