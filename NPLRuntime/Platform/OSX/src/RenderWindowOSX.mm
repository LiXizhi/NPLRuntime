#include "ParaEngine.h"
#include "2dengine/GUIRoot.h"
#include "RenderWindowOSX.h"
#include "WindowDelegate.h"



@interface GLWindow : NSWindow
{

}
@end

@implementation GLWindow
-(BOOL) canBecomeKeyWindow
{
    return YES;
}

@end



using namespace ParaEngine;


IRenderWindow* CreateParaRenderWindow(const int width, const int height)
{
	return new RenderWindowOSX(width, height);
}

RenderWindowOSX::RenderWindowOSX(const int width, const int height)
:m_window(nullptr)
,m_shouldClose(false)
{
    [NSApplication sharedApplication];
    
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
    NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable | NSWindowStyleMaskBorderless;
    
    m_window = [[GLWindow alloc] initWithContentRect:CGRectMake(0, 0, width,height) styleMask:style backing:NSBackingStoreBuffered defer:NO];
    [m_window setTitle:@"Paracraft"];
    [NSApp activateIgnoringOtherApps:YES];
    [NSApp arrangeInFront:m_window];
    [m_window orderFront:nil];
    [m_window makeKeyWindow];
    [m_window makeFirstResponder:m_window];
    

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
    return (intptr_t)m_window;
}


unsigned int RenderWindowOSX::GetHeight() const
{
    return m_window.contentView.frame.size.height;
}


unsigned int RenderWindowOSX::GetWidth() const
{
    return m_window.contentView.frame.size.width;
}

bool RenderWindowOSX::ShouldClose() const
{
    return m_shouldClose;
}


void RenderWindowOSX::PollEvents() { 
    if(m_window == nullptr || ShouldClose()) return;
    NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:[NSDate distantPast]
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];

    uint32_t mx = (uint32_t)[event locationInWindow].x;
    uint32_t my = GetHeight() - (uint32_t)[event locationInWindow].y;
    switch([(NSEvent *)event type])
    {
            
        case NSEventTypeLeftMouseDown:
            OnMouseButton(EMouseButton::LEFT, EKeyState::PRESS, mx, my);
            break;
        case NSEventTypeLeftMouseUp:
            OnMouseButton(EMouseButton::LEFT, EKeyState::RELEASE, mx, my);
            break;
        case NSEventTypeRightMouseDown:
            OnMouseButton(EMouseButton::RIGHT, EKeyState::PRESS, mx, my);
            break;
        case NSEventTypeRightMouseUp:
            OnMouseButton(EMouseButton::RIGHT, EKeyState::RELEASE, mx, my);
            break;
        case NSEventTypeMouseMoved:
        case NSEventTypeLeftMouseDragged:
        case NSEventTypeRightMouseDragged:
        case NSEventTypeOtherMouseDragged:
            OnMouseMove(mx, my);
            break;
        case NSEventTypeScrollWheel:
            OnMouseWhell([event deltaX], [event deltaY]);
            break;
        case NSEventTypeOtherMouseDown:
            OnMouseButton(EMouseButton::MIDDLE, EKeyState::PRESS, mx, my);
            break;
        case NSEventTypeOtherMouseUp:
            OnMouseButton(EMouseButton::MIDDLE, EKeyState::RELEASE, mx, my);
            break;
        case NSEventTypeKeyDown:
            NSLog(@"Key Down!");
            break;
        case NSEventTypeKeyUp:
            NSLog(@"Key Up!");
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

void RenderWindowOSX::OnMouseButton(ParaEngine::EMouseButton button, ParaEngine::EKeyState state, uint32_t x, uint32_t y)
{
 
    if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
    {
        return;
    }
    
    CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(DeviceMouseEventPtr(new DeviceMouseButtonEvent(button,state,x,y)));
}

void RenderWindowOSX::OnChar(unsigned int character)
{
    
}


void RenderWindowOSX::OnKey(ParaEngine::EVirtualKey key, ParaEngine::EKeyState state)
{
    
}


void RenderWindowOSX::OnMouseWhell(float deltaX, float deltaY)
{
    if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
    {
        return;
    }
    CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(DeviceMouseEventPtr(new DeviceMouseWheelEvent(deltaY)));
}


void RenderWindowOSX::OnMouseMove(uint32_t x, uint32_t y)
{
    if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
    {
        return;
    }
    CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(DeviceMouseEventPtr(new DeviceMouseMoveEvent(x, y)));
}







