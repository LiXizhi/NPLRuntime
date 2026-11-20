#include "glad/glad.h"
#import <Cocoa/Cocoa.h>
#include "RenderDeviceAGL.h"

using namespace ParaEngine;
RenderDeviceAGL::RenderDeviceAGL(NSOpenGLContext *context)
:m_GLContext(context)
{
    // Updating the drawable queries WindowServer synchronously. AppKit
    // notifies the attached GL view when its global frame or screen changes.
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    NSView* view = [context view];
    void (^updateDrawable)(NSNotification*) = ^(NSNotification* note) {
        [context update];
    };
    m_frameObserver = [center addObserverForName:NSViewGlobalFrameDidChangeNotification
                                          object:view queue:nil usingBlock:updateDrawable];
    m_backingObserver = [center addObserverForName:NSWindowDidChangeBackingPropertiesNotification
                                            object:[view window] queue:nil usingBlock:updateDrawable];
}

RenderDeviceAGL::~RenderDeviceAGL()
{
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center removeObserver:m_backingObserver];
    [center removeObserver:m_frameObserver];
    m_GLContext = nullptr;
}

bool RenderDeviceAGL::Present()
{
    [m_GLContext makeCurrentContext];
    [m_GLContext flushBuffer];
    return true;
}



