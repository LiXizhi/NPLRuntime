#include "ParaEngine.h"
#include "2dengine/GUIRoot.h"
#include "RenderWindowOSX.h"
#include "WindowDelegate.h"

#include <Carbon/Carbon.h> /* For kVK_ constants, and TIS functions. */
#include <unordered_map>


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


EVirtualKey toVirtualKey(int32_t keycode)
{
    static std::unordered_map<int32_t, EVirtualKey> s_keymap;
    if (s_keymap.size() == 0)
    {
        s_keymap[kVK_ANSI_A]=EVirtualKey::KEY_A;
        s_keymap[kVK_ANSI_S]=EVirtualKey::KEY_S;
        s_keymap[kVK_ANSI_D]=EVirtualKey::KEY_D;
        s_keymap[kVK_ANSI_F]=EVirtualKey::KEY_F;
        s_keymap[kVK_ANSI_H]=EVirtualKey::KEY_H;
        s_keymap[kVK_ANSI_G]=EVirtualKey::KEY_G;
        s_keymap[kVK_ANSI_Z]=EVirtualKey::KEY_Z;
        s_keymap[kVK_ANSI_X]=EVirtualKey::KEY_X;
        s_keymap[kVK_ANSI_C]=EVirtualKey::KEY_C;
        s_keymap[kVK_ANSI_V]=EVirtualKey::KEY_V;
        s_keymap[kVK_ANSI_B]=EVirtualKey::KEY_B;
        s_keymap[kVK_ANSI_Q]=EVirtualKey::KEY_Q;
        s_keymap[kVK_ANSI_W]=EVirtualKey::KEY_W;
        s_keymap[kVK_ANSI_E]=EVirtualKey::KEY_E;
        s_keymap[kVK_ANSI_R]=EVirtualKey::KEY_R;
        s_keymap[kVK_ANSI_Y]=EVirtualKey::KEY_Y;
        s_keymap[kVK_ANSI_T]=EVirtualKey::KEY_T;
        s_keymap[kVK_ANSI_1]=EVirtualKey::KEY_1;
        s_keymap[kVK_ANSI_2]=EVirtualKey::KEY_2;
        s_keymap[kVK_ANSI_3]=EVirtualKey::KEY_3;
        s_keymap[kVK_ANSI_4]=EVirtualKey::KEY_4;
        s_keymap[kVK_ANSI_6]=EVirtualKey::KEY_6;
        s_keymap[kVK_ANSI_5]=EVirtualKey::KEY_5;
        s_keymap[kVK_ANSI_Equal]=EVirtualKey::KEY_EQUALS;
        s_keymap[kVK_ANSI_9]=EVirtualKey::KEY_9;
        s_keymap[kVK_ANSI_7]=EVirtualKey::KEY_7;
        s_keymap[kVK_ANSI_Minus]=EVirtualKey::KEY_MINUS;
        s_keymap[kVK_ANSI_8]=EVirtualKey::KEY_8;
        s_keymap[kVK_ANSI_0]=EVirtualKey::KEY_0;
        s_keymap[kVK_ANSI_RightBracket]=EVirtualKey::KEY_RBRACKET;
        s_keymap[kVK_ANSI_O]=EVirtualKey::KEY_O;
        s_keymap[kVK_ANSI_U]=EVirtualKey::KEY_U;
        s_keymap[kVK_ANSI_LeftBracket]=EVirtualKey::KEY_LBRACKET;
        s_keymap[kVK_ANSI_I]=EVirtualKey::KEY_I;
        s_keymap[kVK_ANSI_P]=EVirtualKey::KEY_P;
        s_keymap[kVK_ANSI_L]=EVirtualKey::KEY_L;
        s_keymap[kVK_ANSI_J]=EVirtualKey::KEY_J;
        s_keymap[kVK_ANSI_Quote]=EVirtualKey::KEY_GRAVE;
        s_keymap[kVK_ANSI_K]=EVirtualKey::KEY_K;
        s_keymap[kVK_ANSI_Semicolon]=EVirtualKey::KEY_SEMICOLON;
        s_keymap[kVK_ANSI_Backslash]=EVirtualKey::KEY_BACKSLASH;
        s_keymap[kVK_ANSI_Comma]=EVirtualKey::KEY_COMMA;
        s_keymap[kVK_ANSI_Slash]=EVirtualKey::KEY_SLASH;
        s_keymap[kVK_ANSI_N]=EVirtualKey::KEY_N;
        s_keymap[kVK_ANSI_M]=EVirtualKey::KEY_M;
        s_keymap[kVK_ANSI_Period]=EVirtualKey::KEY_PERIOD;
        s_keymap[kVK_ANSI_Grave]=EVirtualKey::KEY_GRAVE;
        s_keymap[kVK_ANSI_KeypadDecimal]=EVirtualKey::KEY_DECIMAL;
        s_keymap[kVK_ANSI_KeypadMultiply]=EVirtualKey::KEY_MULTIPLY;
        s_keymap[kVK_ANSI_KeypadPlus]=EVirtualKey::KEY_ADD;
        s_keymap[kVK_ANSI_KeypadClear]=EVirtualKey::KEY_UNKNOWN;
        s_keymap[kVK_ANSI_KeypadDivide]=EVirtualKey::KEY_DIVIDE;
        s_keymap[kVK_ANSI_KeypadEnter]= EVirtualKey::KEY_NUMPADENTER;
        s_keymap[kVK_ANSI_KeypadMinus]=EVirtualKey::KEY_MINUS;
        s_keymap[kVK_ANSI_KeypadEquals]=EVirtualKey::KEY_NUMPADEQUALS;
        s_keymap[kVK_ANSI_Keypad0]=EVirtualKey::KEY_NUMPAD0;
        s_keymap[kVK_ANSI_Keypad1]=EVirtualKey::KEY_NUMPAD1;
        s_keymap[kVK_ANSI_Keypad2]=EVirtualKey::KEY_NUMPAD2;
        s_keymap[kVK_ANSI_Keypad3]=EVirtualKey::KEY_NUMPAD3;
        s_keymap[kVK_ANSI_Keypad4]=EVirtualKey::KEY_NUMPAD4;
        s_keymap[kVK_ANSI_Keypad5]=EVirtualKey::KEY_NUMPAD4;
        s_keymap[kVK_ANSI_Keypad6]=EVirtualKey::KEY_NUMPAD6;
        s_keymap[kVK_ANSI_Keypad7]=EVirtualKey::KEY_NUMPAD7;
        s_keymap[kVK_ANSI_Keypad8]=EVirtualKey::KEY_NUMPAD8;
        s_keymap[kVK_ANSI_Keypad9]=EVirtualKey::KEY_NUMPAD9;
        
        
        s_keymap[kVK_Return]= EVirtualKey::KEY_RETURN;
        s_keymap[kVK_Tab]=EVirtualKey::KEY_TAB;
        s_keymap[kVK_Space]=EVirtualKey::KEY_SPACE;
        s_keymap[kVK_Delete]=EVirtualKey::KEY_BACK;//EVirtualKey::KEY_DELETE;
        s_keymap[kVK_Escape]=EVirtualKey::KEY_ESCAPE;
        s_keymap[kVK_Command]=EVirtualKey::KEY_LWIN;
        s_keymap[kVK_Shift]=EVirtualKey::KEY_LSHIFT;
        s_keymap[kVK_CapsLock]=EVirtualKey::KEY_CAPITAL;
        s_keymap[kVK_Option]=EVirtualKey::KEY_LMENU;
        s_keymap[kVK_Control]=EVirtualKey::KEY_LCONTROL;
        s_keymap[kVK_RightCommand]=EVirtualKey::KEY_RWIN;
        s_keymap[kVK_RightShift]=EVirtualKey::KEY_RSHIFT;
        s_keymap[kVK_RightOption]=EVirtualKey::KEY_RMENU;
        s_keymap[kVK_RightControl]=EVirtualKey::KEY_RCONTROL;
        s_keymap[kVK_Function]=EVirtualKey::KEY_UNKNOWN;
        s_keymap[kVK_F17]= EVirtualKey::KEY_UNKNOWN;
        s_keymap[kVK_VolumeUp]=EVirtualKey::KEY_VOLUMEUP;
        s_keymap[kVK_VolumeDown]=EVirtualKey::KEY_VOLUMEDOWN;
        s_keymap[kVK_Mute]=EVirtualKey::KEY_MUTE;
        s_keymap[kVK_F18]=EVirtualKey::KEY_UNKNOWN;
        s_keymap[kVK_F19]=EVirtualKey::KEY_UNKNOWN;
        s_keymap[kVK_F20]=EVirtualKey::KEY_UNKNOWN;
        s_keymap[kVK_F5]=EVirtualKey::KEY_F5;
        s_keymap[kVK_F6]=EVirtualKey::KEY_F6;
        s_keymap[kVK_F7]=EVirtualKey::KEY_F7;
        s_keymap[kVK_F3]=EVirtualKey::KEY_F3;
        s_keymap[kVK_F8]=EVirtualKey::KEY_F8;
        s_keymap[kVK_F9]=EVirtualKey::KEY_F9;
        s_keymap[kVK_F11]=EVirtualKey::KEY_F11;
        s_keymap[kVK_F13]=EVirtualKey::KEY_F13;
        s_keymap[kVK_F16]=EVirtualKey::KEY_UNKNOWN;
        s_keymap[kVK_F14]=EVirtualKey::KEY_F14;
        s_keymap[kVK_F10]=EVirtualKey::KEY_F10;
        s_keymap[kVK_F12]=EVirtualKey::KEY_F12;
        s_keymap[kVK_F15]=EVirtualKey::KEY_F15;
        s_keymap[kVK_Help]=EVirtualKey::KEY_UNKNOWN;
        s_keymap[kVK_Home]=EVirtualKey::KEY_HOME;
        s_keymap[kVK_PageUp]=EVirtualKey::KEY_PERIOD;
        s_keymap[kVK_ForwardDelete]=EVirtualKey::KEY_DELETE; //EVirtualKey::KEY_BACK;
        s_keymap[kVK_F4]=EVirtualKey::KEY_F4;
        s_keymap[kVK_End]=EVirtualKey::KEY_END;
        s_keymap[kVK_F2]=EVirtualKey::KEY_F2;
        s_keymap[kVK_PageDown]=EVirtualKey::KEY_NEXT;
        s_keymap[kVK_F1]=EVirtualKey::KEY_F1;
        s_keymap[kVK_LeftArrow]=EVirtualKey::KEY_LEFT;
        s_keymap[kVK_RightArrow]=EVirtualKey::KEY_RIGHT;
        s_keymap[kVK_DownArrow]=EVirtualKey::KEY_DOWN;
        s_keymap[kVK_UpArrow]=EVirtualKey::KEY_UP;
    }
    
    auto ret = s_keymap.find(keycode);
    if (ret != s_keymap.end())
    {
        return ret->second;
    }
    return EVirtualKey::KEY_UNKNOWN;
    
}


IRenderWindow* CreateParaRenderWindow(const int width, const int height)
{
	return new RenderWindowOSX(width, height);
}

RenderWindowOSX::RenderWindowOSX(const int width, const int height)
:m_window(nullptr)
,m_shouldClose(false)
,m_scrollMouseX(0)
,m_scrollMouseY(0)
{
    [NSApplication sharedApplication];
    
    // Menu
    //*
    NSString* appName = [NSString stringWithFormat:@"%s", "Paracraft"];
    id menubar = [[NSMenu alloc] initWithTitle:appName];
    id appMenuItem = [NSMenuItem new];
    [menubar addItem: appMenuItem];
    
    
    id appMenu = [NSMenu new];
    id quitMenuItem = [[NSMenuItem alloc] initWithTitle:@"Quit"
                                                 action:@selector(terminate:)
                                          keyEquivalent:@"q"];
  
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];
    
    [NSApp setMainMenu:menubar];
    //*/
    
    //NSMenu* rootMenu = [NSApp mainMenu];
    //[rootMenu removeItemAtIndex:0];
    
    
    NSInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
    NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable | NSWindowStyleMaskBorderless;
    
    m_window = [[GLWindow alloc] initWithContentRect:CGRectMake(0, 0, width,height) styleMask:style backing:NSBackingStoreBuffered defer:NO];
    [m_window setTitle:@"Paracraft"];
    [NSApp activateIgnoringOtherApps:YES];
    [NSApp arrangeInFront:m_window];
    [m_window orderFront:nil];
    [m_window makeKeyWindow];
    [m_window setAcceptsMouseMovedEvents:YES];
    [m_window makeFirstResponder:m_window];
    //[m_window.contentView setAllowedTouchTypes:NSTouchTypeMaskDirect];

    
    WindowDelegate* winDelegate = [WindowDelegate sharedDelegate];
    [m_window setDelegate:winDelegate];

}


RenderWindowOSX::~RenderWindowOSX()
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
    NSEvent *event;
    auto untilDate = [NSDate distantPast];
    bool bIsProcessed = false;
    do{
    event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:untilDate
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];
    if(!event)
        break;
        
    uint32_t mx = (uint32_t)[event locationInWindow].x;
    uint32_t my = GetHeight() - (uint32_t)[event locationInWindow].y;
    
    
   /* NSEventSubtype subType = [event subtype];
    
    if(subType == NSEventSubtypeMouseEvent)
    {
        NSLog(@"Mouse event");
    }
    if(subType == NSEventSubtypeTouch)
    {
        NSLog(@"touch event");
    }*/
    
    
    switch([(NSEvent *)event type])
    {
        case NSEventTypePressure:
            //NSLog(@"Pressure");
        {

        }
        break;
            
        case NSEventTypeLeftMouseDown:
            if(event.window == m_window)
                OnMouseButton(EMouseButton::LEFT, EKeyState::PRESS, mx, my);
            break;
        case NSEventTypeLeftMouseUp:
            if(event.window == m_window)
                OnMouseButton(EMouseButton::LEFT, EKeyState::RELEASE, mx, my);
            break;
        case NSEventTypeRightMouseDown:
            if(event.window == m_window)
                OnMouseButton(EMouseButton::RIGHT, EKeyState::PRESS, mx, my);
            break;
        case NSEventTypeRightMouseUp:
            if(event.window == m_window)
                OnMouseButton(EMouseButton::RIGHT, EKeyState::RELEASE, mx, my);
            break;

            
        case NSEventTypeMouseMoved:
        case NSEventTypeLeftMouseDragged:
        case NSEventTypeRightMouseDragged:
        case NSEventTypeOtherMouseDragged:
        {
            if(event.window == m_window){
                OnMouseMove(mx, my);
                m_scrollMouseX = mx;
                m_scrollMouseY = my;
            }
            break;
        }
        case NSEventTypeSwipe:
        {
            if(event.window == m_window){
                float deltaX = [event deltaX];
                float deltaY = [event deltaY];
                if(deltaY != 0){
                    OnMouseWhell(deltaX, deltaY);
                }
            }
            break;
        }
        case NSEventTypeScrollWheel:
        {
            if(event.window == m_window)
            {
                NSEventPhase phase = [event phase];
                NSEventPhase momentumPhase = [event momentumPhase];
                
                static bool s_bMouseOverScrollableUI = false;
                if (CGlobals::GetApp()->GetAppState() == PEAppState_Ready)
                {
                    s_bMouseOverScrollableUI = CGUIRoot::GetInstance()->IsMouseOverScrollableUI();
                }
                
                if(phase == NSEventPhaseNone && momentumPhase == NSEventPhaseNone)
                {
                    OnMouseWhell([event deltaX], [event deltaY]);
                }
                else if(phase != NSEventPhaseNone)
                {
                    m_scrollMouseX+= [event deltaX]*4;
                    m_scrollMouseY+= [event deltaY]*4;
                    
                    if(s_bMouseOverScrollableUI)
                    {
                        if(phase == NSEventPhaseChanged)
                        {
                            OnMouseWhell([event deltaX], [event deltaY]);
                        }
                    }
                    else
                    {
                        switch (phase) {
                            //case NSEventPhaseMayBegin:
                            case NSEventPhaseBegan:
                                OnMouseButton(EMouseButton::RIGHT, EKeyState::PRESS, m_scrollMouseX, m_scrollMouseY);
                                break;
                            case NSEventPhaseChanged:
                            {
                                OnMouseMove(m_scrollMouseX, m_scrollMouseY);
                            }
                                break;
                            case NSEventPhaseEnded:
                            //case NSEventPhaseCancelled:
                                OnMouseButton(EMouseButton::RIGHT, EKeyState::RELEASE, m_scrollMouseX, m_scrollMouseY);
                                break;
                            default:
                                break;
                        }
                    }
                }
            }
        }
            break;
        case NSEventTypeOtherMouseDown:
            if(event.window == m_window)
                OnMouseButton(EMouseButton::MIDDLE, EKeyState::PRESS, mx, my);
            break;
        case NSEventTypeOtherMouseUp:
            if(event.window == m_window)
                OnMouseButton(EMouseButton::MIDDLE, EKeyState::RELEASE, mx, my);
            break;
        case NSEventTypeKeyDown:
        {
            if(event.window == m_window)
            {

                NSString *chrs = [event characters];

                uint32_t keycode = (uint32_t)[event keyCode];
                EVirtualKey vk = toVirtualKey(keycode);
                
                if([chrs length]>0)
                {
                    int unicode = [chrs characterAtIndex:0];
                    if(
                       !isPressCommand &&
                       (unicode >= 32 && unicode <= 126) ||
                       (vk!=EVirtualKey::KEY_UP && vk!=EVirtualKey::KEY_DOWN && vk!=EVirtualKey::KEY_LEFT && vk!=EVirtualKey::KEY_RIGHT && unicode > 255)
                    )
                    {
                        OnChar(unicode);
                    }
                }

                OnKey(vk, EKeyState::PRESS);

                bIsProcessed = (event.modifierFlags & NSEventModifierFlagCommand) == 0;
            }
        }
            break;
        case NSEventTypeKeyUp:
        {
            
            /*
            NSString *chrs = [event characters];

            if([chrs length]>0)
            {
                int unicode = [chrs characterAtIndex:0];
                if(unicode>=0 && unicode<256)
                {
                   OnChar(unicode);
                }

            }
            */
            if(event.window == m_window)
            {
                uint32_t keycode = (uint32_t)[event keyCode];
                EVirtualKey vk = toVirtualKey(keycode);
                OnKey(vk, EKeyState::RELEASE);
                
                bIsProcessed = (event.modifierFlags & NSEventModifierFlagCommand) == 0;
            }
        }
            break;
        case NSEventTypeFlagsChanged:
        {
            if(event.window == m_window)
            {
                static uint32_t last_flags = 0;
                uint32_t flags = (uint32_t)[event modifierFlags];
                //////
                if((flags & NSEventModifierFlagCapsLock) && !(last_flags & NSEventModifierFlagCapsLock))
                {
                    OnKey(EVirtualKey::KEY_CAPITAL,EKeyState::PRESS);
                }
                
                if(!(flags & NSEventModifierFlagCapsLock) && (last_flags & NSEventModifierFlagCapsLock))
                {
                    OnKey(EVirtualKey::KEY_CAPITAL, EKeyState::RELEASE);
                }
                ////////
                if((flags & NSEventModifierFlagHelp) && !(last_flags & NSEventModifierFlagHelp))
                {
                    NSLog(@"HELP press");
                }
                
                if(!(flags & NSEventModifierFlagHelp) && (last_flags & NSEventModifierFlagHelp))
                {
                    NSLog(@"HELP release");
                }
                
                ///////
                if((flags & NSEventModifierFlagShift) && !(last_flags & NSEventModifierFlagShift))
                {
                    OnKey(EVirtualKey::KEY_LSHIFT, EKeyState::PRESS);
                }
                
                if(!(flags & NSEventModifierFlagShift) && (last_flags & NSEventModifierFlagShift))
                {
                    OnKey(EVirtualKey::KEY_LSHIFT, EKeyState::RELEASE);
                }
                
                /////////
                if((flags & NSEventModifierFlagOption) && !(last_flags & NSEventModifierFlagOption))
                {
                    OnKey(EVirtualKey::KEY_LMENU, EKeyState::PRESS);
                }
                
                if(!(flags & NSEventModifierFlagOption) && (last_flags & NSEventModifierFlagOption))
                {
                    OnKey(EVirtualKey::KEY_LMENU, EKeyState::RELEASE);
                }

                /////////
                if((flags & NSEventModifierFlagControl) && !(last_flags & NSEventModifierFlagControl))
                {
                    OnKey(EVirtualKey::KEY_LCONTROL,EKeyState::PRESS);
                }

                if(!(flags & NSEventModifierFlagControl) && (last_flags & NSEventModifierFlagControl))
                {
                    OnKey(EVirtualKey::KEY_LCONTROL, EKeyState::RELEASE);
                }

                ////////////
                if((flags & NSEventModifierFlagCommand) && !(last_flags & NSEventModifierFlagCommand))
                {
                    isPressCommand = true;
                    OnKey(EVirtualKey::KEY_LCONTROL, EKeyState::PRESS);
                }
                
                if(!(flags & NSEventModifierFlagCommand) && (last_flags & NSEventModifierFlagCommand))
                {
                    isPressCommand = false;
                    OnKey(EVirtualKey::KEY_LCONTROL, EKeyState::RELEASE);
                }
                
                ///////////////
                if((flags & NSEventModifierFlagFunction) && !(last_flags & NSEventModifierFlagFunction))
                {
                    NSLog(@"Function press");
                }
                
                if(!(flags & NSEventModifierFlagFunction) && (last_flags & NSEventModifierFlagFunction))
                {
                    NSLog(@"Function release");
                }
                
                ////////////////
                if((flags & NSEventModifierFlagNumericPad) && !(last_flags & NSEventModifierFlagNumericPad))
                {
                    OnKey(EVirtualKey::KEY_NUMLOCK, EKeyState::PRESS);
                }
                
                if(!(flags & NSEventModifierFlagNumericPad) && (last_flags & NSEventModifierFlagNumericPad))
                {
                    OnKey(EVirtualKey::KEY_NUMLOCK, EKeyState::RELEASE);
                }
                last_flags = flags;
            }
            break;
        }
        default:
            break;
    }
    if(!bIsProcessed)
    {
        [NSApp sendEvent:event];
    }
    [NSApp updateWindows];
    [event release];
    }while(event);
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
    
    if(button == EMouseButton::RIGHT)
    {
        if(state == EKeyState::PRESS)
        {
            //NSLog(@"OnRightMouse PRESS: %d,%d",x,y);
        }else{
            //NSLog(@"OnRightMouse RELEASE: %d,%d",x,y);
        }
    }
    
    CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(DeviceMouseEventPtr(new DeviceMouseButtonEvent(button,state,x,y)));
}

void RenderWindowOSX::OnChar(unsigned int character)
{
    if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
    {
        return;
    }
    auto pGUI = CGUIRoot::GetInstance()->GetUIKeyFocus();
    if (pGUI)
    {
        std::wstring s;
        s += (WCHAR)character;
        pGUI->OnHandleWinMsgChars(s);
    }
}


void RenderWindowOSX::OnKey(ParaEngine::EVirtualKey key, ParaEngine::EKeyState state)
{
    if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
    {
        return;
    }

    bool pressed = state == EKeyState::PRESS ? true : false;
    if(pressed)
    {
        CGUIRoot::GetInstance()->SendKeyDownEvent(key);
    }else{
        CGUIRoot::GetInstance()->SendKeyUpEvent(key);
    }
}


void RenderWindowOSX::OnMouseWhell(float deltaX, float deltaY)
{
    if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
    {
        return;
    }
    
    if(deltaY != 0.f)
    {
        CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(DeviceMouseEventPtr(new DeviceMouseWheelEvent(deltaY * 120)));
    }
}


void RenderWindowOSX::OnMouseMove(uint32_t x, uint32_t y)
{
    //NSLog(@"OnMouseMove: %d,%d",x,y);
    if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
    {
        return;
    }
    CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(DeviceMouseEventPtr(new DeviceMouseMoveEvent(x, y)));
}

void RenderWindowOSX::setTitle(const char* title)
{
    NSString* titleNS = [NSString stringWithUTF8String:title];
    [m_window setTitle:titleNS];
}

const char*  RenderWindowOSX::getTitle()
{
    const char* ret = [[m_window title] UTF8String];
    return ret;
}





