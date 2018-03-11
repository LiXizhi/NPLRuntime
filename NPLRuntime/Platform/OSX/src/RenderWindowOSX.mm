#include "RenderWindowOSX.h"
#include "WindowDelegate.h"

using namespace ParaEngine;




RenderWindowOSX::RenderWindowOSX(const int width, const int height)
:m_window(nullptr)
,m_shouldClose(false)
{
    [NSApplication sharedApplication];
    [NSApp activateIgnoringOtherApps:YES];
    
    // Menu
    /*NSString* appName = [NSString stringWithFormat:@"%s", "Paracraft"];
    id menubar = [[NSMenu alloc] initWithTitle:appName];
    id appMenuItem = [NSMenuItem new];
    [menubar addItem: appMenuItem];
    [NSApp setMainMenu:menubar];
    
    id appMenu = [NSMenu new];
    id quitMenuItem = [[NSMenuItem alloc] initWithTitle:@"Quit"
                                                 action:@selector(terminate:)
                                          keyEquivalent:@"q"];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];*/
    
    
    NSInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
    NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
    
    
    m_window = [[NSWindow alloc] initWithContentRect:CGRectMake(0, 0, width,height) styleMask:style backing:NSBackingStoreBuffered defer:NO];
    [m_window setTitle:@"Paracraft"];
    [m_window makeKeyAndOrderFront:nil];
    RenderWindowOSX* renderWindow = this;
    WindowDelegate* winDelegate = [[WindowDelegate alloc] InitWithRenderWindow:renderWindow];
    [m_window setDelegate:winDelegate];
    
}

RenderWindowOSX::~RenderWindowOSX() noexcept
{
    [m_window release];
}


intptr_t RenderWindowOSX::GetNativeHandle() const
{
    return 0;
}


unsigned int RenderWindowOSX::GetHeight() const
{
    return 600;
}


unsigned int RenderWindowOSX::GetWidth() const
{
    return 800;
}

bool RenderWindowOSX::ShouldClose() const
{
    return m_shouldClose;
}


void RenderWindowOSX::PollEvents() { 
    if(m_window == nullptr || ShouldClose()) return;
    NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:nil
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];
    
    switch([(NSEvent *)event type])
    {
        case NSEventTypeKeyDown:
            NSLog(@"Key Down Event Received!");
            break;
        default:
            break;
    }
    [NSApp sendEvent:event];
    [NSApp updateWindows];
    [event release];
}

bool RenderWindowOSX::OnShouldClose()
{
    m_shouldClose = true;
    return true;
}




