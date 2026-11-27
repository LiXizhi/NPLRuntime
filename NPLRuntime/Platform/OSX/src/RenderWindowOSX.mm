#include "RenderWindowOSX.h"
#include "RenderViewOSX.h"
#include "ParaEngine.h"
#include "2dengine/GUIRoot.h"
#include "2dengine/GUIEdit.h"
#include "Core/Variable.h"
#include "AutoCamera.h"
#include "SceneObject.h"
#include "ViewportManager.h"
#include "NPLRuntime.h"
#include "WindowDelegate.h"

#include <Carbon/Carbon.h> /* For kVK_ constants, and TIS functions. */
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>
#include <unordered_map>

#import "LuaObjcBridge/CCLuaObjcBridge.h"

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
        s_keymap[kVK_ANSI_Keypad5]=EVirtualKey::KEY_NUMPAD5;
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
:m_shouldClose(false)
,m_window(nullptr)
,currentBackingScaleFactor(1)
{
    [NSApplication sharedApplication];
    
    // Menu
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
    
    //NSMenu* rootMenu = [NSApp mainMenu];
    //[rootMenu removeItemAtIndex:0];

    NSInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
    NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;

    CGFloat initWidth = (CGFloat)width;
    CGFloat initHeight = (CGFloat)height;
    NSScreen* mainScreen = [NSScreen mainScreen];
    if (mainScreen)
    {
        NSRect visibleFrame = [mainScreen visibleFrame];
        if (visibleFrame.size.width > 0 && visibleFrame.size.height > 0)
        {
            initWidth = std::min(initWidth, visibleFrame.size.width);
            initHeight = std::min(initHeight, std::max((CGFloat)480.0, visibleFrame.size.height - (CGFloat)32.0));
        }
    }

    m_window = [[GLWindow alloc] initWithContentRect:CGRectMake(0, 0, initWidth, initHeight) styleMask:style backing:NSBackingStoreBuffered defer:NO];

    RenderView* renderView = [[[RenderView alloc] initWithFrame:CGRectMake(0, 0, initWidth, initHeight)] autorelease];
    [renderView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [renderView setWantsBestResolutionOpenGLSurface:YES];
    [m_window setContentView:renderView];
    [m_window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

    [m_window setTitle:@"Paracraft"];
    [m_window center];
    [NSApp activateIgnoringOtherApps:YES];
    [NSApp arrangeInFront:m_window];
    [m_window orderFront:nil];
    [m_window makeKeyWindow];
    [m_window setAcceptsMouseMovedEvents:YES];
    [m_window makeFirstResponder:renderView];
    [renderView setIMEEnabled:NO];
    //[m_window.contentView setAllowedTouchTypes:NSTouchTypeMaskDirect];

    double initScaleX = 1.0, initScaleY = 1.0;
    GetScaleFactor(initScaleX, initScaleY);
    if (initScaleX > 0.0)
        currentBackingScaleFactor = (float)initScaleX;
    
    WindowDelegate* winDelegate = [WindowDelegate sharedDelegate];
    [m_window setDelegate:winDelegate];
}

RenderWindowOSX::RenderWindowOSX(NSWindow* existingWindow)
:m_shouldClose(false)
,m_window([existingWindow retain])
,currentBackingScaleFactor(1)
{
    if (m_window && [m_window.contentView isKindOfClass:[RenderView class]])
    {
        [(RenderView*)m_window.contentView setIMEEnabled:NO];
    }
}

RenderWindowOSX::~RenderWindowOSX()
{
    [m_window setDelegate:nil];
    [m_window release];
}

intptr_t RenderWindowOSX::GetNativeHandle() const
{
    return (intptr_t)m_window;
}

unsigned int RenderWindowOSX::GetHeight() const
{   
    if (!m_window || !m_window.contentView)
        return 0;
    auto point = NSMakePoint(0, m_window.contentView.frame.size.height);
    auto backingPoint = [m_window.contentView convertPointToBacking:point];

    return (unsigned int)std::max(0.0, (double)backingPoint.y);
}

unsigned int RenderWindowOSX::GetWidth() const
{    
    if (!m_window || !m_window.contentView)
        return 0;
    auto point = NSMakePoint(m_window.contentView.frame.size.width, 0);
    auto backingPoint = [m_window.contentView convertPointToBacking:point];

    return (unsigned int)std::max(0.0, (double)backingPoint.x);
}

void RenderWindowOSX::GetScaleFactor(double &x, double &y) const
{
    if (!m_window || !m_window.contentView)
    {
        x = 1.0;
        y = 1.0;
        return;
    }

    NSSize frameSize = m_window.contentView.frame.size;
    if (frameSize.width <= 0.0 || frameSize.height <= 0.0)
    {
        CGFloat scale = [m_window backingScaleFactor];
        if (scale <= 0.0 && [NSScreen mainScreen])
            scale = [[NSScreen mainScreen] backingScaleFactor];
        if (scale <= 0.0)
            scale = 1.0;
        x = (double)scale;
        y = (double)scale;
        return;
    }

    auto point = NSMakePoint(frameSize.width, frameSize.height);
    auto backingPoint = [m_window.contentView convertPointToBacking:point];

    x = backingPoint.x / point.x;
    y = backingPoint.y / point.y;
}

void RenderWindowOSX::GetScreenResolution(float *pX, float *pY)
{
    double sx = 1.0, sy = 1.0;
    GetScaleFactor(sx, sy);
    if (sx > 0.0)
        currentBackingScaleFactor = (float)sx;
    float scale = currentBackingScaleFactor > 0.0f ? currentBackingScaleFactor : 1.0f;

    if (pX) {
        *pX = GetWidth() / scale;
    }

    if (pY) {
        *pY = GetHeight() / scale;
    }
}

void RenderWindowOSX::SetScreenResolution(CGFloat w, CGFloat h)
{
    if (m_window && w > 0 && h > 0)
    {
        [m_window setContentSize:NSMakeSize(w, h)];
        OnWindowSizeChanged();
    }
}

double RenderWindowOSX::ComputeAutoUIScale() const
{
    double sx = 1.0, sy = 1.0;
    GetScaleFactor(sx, sy);
    double backingScale = (sx > 0.0) ? sx : 1.0;

    double physW = (double)GetWidth();
    double physH = (double)GetHeight();
    if (physW <= 0.0 || physH <= 0.0)
        return backingScale;

    // Target a comfortable maximum logical UI workspace (~1440x840) on high-resolution Macs
    // (14"/16" Retina MacBook Pro, 4K external displays, 5K Studio Display, 6K Pro Display XDR)
    // so 12px Paracraft fonts stay ~13.5-14.5pt equivalent while never dropping below Retina backingScale.
    const double kComfortMaxLogicalW = 1440.0;
    const double kComfortMaxLogicalH = 840.0;
    const double kMinLogicalW = 1280.0;
    const double kMinLogicalH = 720.0;

    double resScale = std::min(physW / kComfortMaxLogicalW, physH / kComfortMaxLogicalH);
    double targetScale = std::max(backingScale, resScale);

    // Quantize to 0.05 steps to align with SystemSettingsPage UI_Scaling slider steps.
    targetScale = std::floor(targetScale * 20.0 + 0.5) / 20.0;

    // Ensure minimum logical screen size (1280x720) fits without clipping when the window is resized smaller.
    double maxScaleForMinUI = std::min(physW / kMinLogicalW, physH / kMinLogicalH);
    if (targetScale > maxScaleForMinUI && maxScaleForMinUI >= 0.5)
    {
        targetScale = maxScaleForMinUI;
    }

    return std::max(0.5, targetScale);
}

void RenderWindowOSX::UpdateAutoUIScaling(bool bForce)
{
    double sx = 1.0, sy = 1.0;
    GetScaleFactor(sx, sy);
    if (sx > 0.0)
        currentBackingScaleFactor = (float)sx;

    const uint32_t physW = GetWidth();
    const uint32_t physH = GetHeight();
    const float autoScale = (float)ComputeAutoUIScale();

    const bool scaleChanged = std::fabs(autoScale - m_lastAutoUIScale) >= 0.005f;
    const bool sizeChanged = (physW != m_lastPhysicalWidth || physH != m_lastPhysicalHeight);
    if (!bForce && !scaleChanged && !sizeChanged)
        return;

    const bool isFirstApply = (m_lastAutoUIScale <= 0.0f);
    m_lastAutoUIScale = autoScale;
    m_lastPhysicalWidth = physW;
    m_lastPhysicalHeight = physH;

    if (physW > 0 && physH > 0 && CGlobals::GetViewportManager())
    {
        CGlobals::GetViewportManager()->UpdateViewport((int)physW, (int)physH);
    }

    if (isFirstApply && CGUIRoot::GetInstance())
    {
        CGUIRoot::GetInstance()->SetUIScale(autoScale, autoScale, false, false, true);
    }

    if (CGlobals::GetApp() && CGlobals::GetApp()->GetAppState() == PEAppState_Ready && NPL::CNPLRuntime::GetInstance())
    {
        std::ostringstream oss;
        oss.imbue(std::locale::classic());
        oss << std::fixed << std::setprecision(3);
        oss << "do local s=" << autoScale << "; local bs=" << currentBackingScaleFactor << "; "
            << "if System and System.options then System.options.default_ui_scaling={bs,bs}; System.options.auto_ui_scaling={s,s}; end; "
            << "local Screen=System and System.Windows and System.Windows.Screen; "
            << "if Screen then "
            << "if not Screen._macAutoScaleWrapped and Screen.SetUserUIScaling then "
            << "Screen._macAutoScaleWrapped=true; "
            << "local orig=Screen.SetUserUIScaling; "
            << "Screen.SetUserUIScaling=function(self,val) "
            << "local def=(System and System.options and System.options.auto_ui_scaling and System.options.auto_ui_scaling[1]) or s; "
            << "if def then "
            << "local saved=nil; local pc=nil; "
            << "local gl=MyCompany and MyCompany.Aries and MyCompany.Aries.Game and MyCompany.Aries.Game.GameLogic; "
            << "if gl and gl.GetPlayerController then pc=gl.GetPlayerController(); "
            << "if pc and pc.LoadLocalData then saved=pc:LoadLocalData('Paracraft_System_SetUIScaling',nil,true); end end; "
            << "if saved==nil or saved==0 or (bs>1.05 and saved<=1.05) then "
            << "if saved~=nil and saved~=0 and pc and pc.SaveLocalData then pc:SaveLocalData('Paracraft_System_SetUIScaling',nil,true,true); end; "
            << "val=def; "
            << "end end; "
            << "return orig(self,val); "
            << "end; "
            << "end; "
            << "local dw=Screen.GetDesignUIResolution and Screen:GetDesignUIResolution(); "
            << "if not dw then "
            << "local saved=nil; "
            << "local gl=MyCompany and MyCompany.Aries and MyCompany.Aries.Game and MyCompany.Aries.Game.GameLogic; "
            << "if gl and gl.GetPlayerController then local pc=gl.GetPlayerController(); "
            << "if pc and pc.LoadLocalData then saved=pc:LoadLocalData('Paracraft_System_SetUIScaling',nil,true); end end; "
            << "if saved==nil or saved==0 or (bs>1.05 and saved<=1.05) then Screen:SetUIScale(s,s); end; "
            << "end; "
            << "elseif ParaUI then "
            << "local root=ParaUI.GetUIObject('root'); if root and root:IsValid() then root:SetField('UIScale',{s,s}); end; "
            << "end end";
        ParaEngine::LuaObjcBridge::nplActivate(oss.str(), "");
    }
}

void RenderWindowOSX::OnWindowSizeChanged()
{
    UpdateAutoUIScaling(false);
}

bool RenderWindowOSX::ShouldClose() const
{
    return m_shouldClose;
}

void RenderWindowOSX::OnMouseDown(EMouseButton button, NSEvent* event)
{
    EndTrackpadDrag();
    OnMouseEvent(button, EKeyState::PRESS, event);
}

void RenderWindowOSX::OnMouseUp(EMouseButton button, NSEvent* event)
{
    OnMouseEvent(button, EKeyState::RELEASE, event);
}

NSPoint RenderWindowOSX::GetMousePosition(NSEvent* event) const
{
    NSView* view = m_window.contentView;
    NSPoint point = [view convertPointToBacking:[view convertPoint:event.locationInWindow fromView:nil]];
    return NSMakePoint(point.x, GetHeight() - point.y);
}

void RenderWindowOSX::OnMouseMove(NSEvent* event)
{
    SyncShortcutModifiers(event.modifierFlags);
    if (m_trackpadDrag != TrackpadDrag::Active)
    {
        NSPoint point = GetMousePosition(event);
        OnMouseMove(point.x, point.y);
    }
}

void RenderWindowOSX::OnMouseEvent(EMouseButton button, EKeyState state, NSEvent* event)
{
    SyncShortcutModifiers(event.modifierFlags);
    NSPoint point = GetMousePosition(event);
    OnMouseButton(button, state, point.x, point.y);
}

void RenderWindowOSX::OnKey(EKeyState state, NSEvent* event)
{
    uint32_t keycode = (uint32_t)[event keyCode];
    EVirtualKey vk = toVirtualKey(keycode);
    uint32_t flags = (uint32_t)[event modifierFlags];
    SyncShortcutModifiers(flags);

    bool pressed = state == EKeyState::PRESS ? true : false;

    if (pressed) {
        if ((flags & NSEventModifierFlagCommand) && (m_curPressKey != EVirtualKey::KEY_UNKNOWN))
        {
            OnKey(m_curPressKey, EKeyState::RELEASE);
        }

        m_curPressKey = vk;
    } else {
        m_curPressKey = EVirtualKey::KEY_UNKNOWN;
    }

    OnKey(vk,state);
}

void RenderWindowOSX::SyncShortcutModifiers(NSEventModifierFlags flags)
{
    // Either native modifier holds the same engine Ctrl key. Mouse events carry
    // this snapshot even if AppKit delivered the flagsChanged release elsewhere.
    const auto held = flags & (NSEventModifierFlagControl | NSEventModifierFlagCommand);
    if (held == m_shortcutModifiers || !CGlobals::GetApp() || CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
        return;

    if ((m_shortcutModifiers & NSEventModifierFlagCommand) && !(held & NSEventModifierFlagCommand)
        && m_curPressKey != EVirtualKey::KEY_UNKNOWN)
    {
        OnKey(m_curPressKey, EKeyState::RELEASE);
        m_curPressKey = EVirtualKey::KEY_UNKNOWN;
    }
    if ((m_shortcutModifiers != 0) != (held != 0))
        OnKey(EVirtualKey::KEY_LCONTROL, held ? EKeyState::PRESS : EKeyState::RELEASE);
    m_shortcutModifiers = held;
}

void RenderWindowOSX::OnFlagsChanged(NSEvent* event)
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

    SyncShortcutModifiers(flags);
    
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

void RenderWindowOSX::OnScrollWheel(NSEvent* event)
{
    SyncShortcutModifiers(event.modifierFlags);
    NSEventPhase phase = [event phase];
    NSEventPhase momentumPhase = [event momentumPhase];
    
    if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
        return;
    if (m_magnifyingWorld && (phase != NSEventPhaseNone || momentumPhase != NSEventPhaseNone))
        return;
    const bool overUI = CGUIRoot::GetInstance()->IsMouseOverScrollableUI();
    const bool precise = [event hasPreciseScrollingDeltas];
    // Convert Cocoa points to wheel notches. The GUI bridge retains 120 ticks per notch.
    const float dx = precise ? [event scrollingDeltaX] / 40.f : [event deltaX];
    const float dy = precise ? [event scrollingDeltaY] / 40.f : [event deltaY];

    if (phase == NSEventPhaseNone && momentumPhase == NSEventPhaseNone)
    {
        SendWheel(dx, overUI ? dy : [event deltaY], precise && overUI);
        return;
    }
    if (phase == NSEventPhaseBegan)
    {
        EndTrackpadDrag();
        m_scrollGestureOverUI = overUI;
    }
    // Position updates stop while fingers are held still. Expose contact separately
    // so an elastic pane can wait for lift even when there is no final wheel delta.
    if (phase == NSEventPhaseBegan || phase == NSEventPhaseEnded || phase == NSEventPhaseCancelled)
    {
        CVariable tracking;
        tracking = phase == NSEventPhaseBegan && m_scrollGestureOverUI;
        CGUIRoot::GetInstance()->SetDynamicField("TrackpadScrollActive", tracking);
    }
    if (m_scrollGestureOverUI)
    {
        if (overUI && (phase == NSEventPhaseChanged || phase == NSEventPhaseBegan ||
            momentumPhase == NSEventPhaseBegan || momentumPhase == NSEventPhaseChanged))
            SendWheel(dx, dy, precise);
        if (phase == NSEventPhaseCancelled || momentumPhase == NSEventPhaseEnded || momentumPhase == NSEventPhaseCancelled)
            m_scrollGestureOverUI = false;
        return;
    }
    if (phase == NSEventPhaseBegan && [NSEvent pressedMouseButtons] == 0)
    {
        m_dragPosition = GetMousePosition(event);
        m_lastDragSample = m_dragPosition;
        m_trackpadDrag = TrackpadDrag::Active;
        OnMouseButton(EMouseButton::RIGHT, EKeyState::PRESS, m_dragPosition.x, m_dragPosition.y);
    }
    if (m_trackpadDrag != TrackpadDrag::Active)
        return;
    if (phase == NSEventPhaseBegan || phase == NSEventPhaseChanged)
    {
        // A precise scroll delta is in points. Keep fractions between frames.
        const CGFloat dragX = precise ? event.scrollingDeltaX : event.deltaX * 4;
        const CGFloat dragY = precise ? event.scrollingDeltaY : event.deltaY * 4;
        if (dragX || dragY)
        {
            m_dragPosition.x += dragX;
            m_dragPosition.y += dragY;
            OnMouseMove(m_dragPosition.x, m_dragPosition.y);
        }
    }
    else if (phase == NSEventPhaseEnded || phase == NSEventPhaseCancelled)
        EndTrackpadDrag();
}

void RenderWindowOSX::OnMagnify(NSEvent* event)
{
    if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
        return;

    auto* gui = CGUIRoot::GetInstance();
    const auto phase = event.phase;
    if (phase == NSEventPhaseBegan)
    {
        CancelScrollGesture();
        m_magnifyingWorld = !gui->IsMouseOverScrollableUI() && !gui->GetTopLevelControl();
    }
    const bool wasMagnifyingWorld = m_magnifyingWorld;
    if (phase == NSEventPhaseEnded || phase == NSEventPhaseCancelled)
        m_magnifyingWorld = false;
    if (!wasMagnifyingWorld || phase == NSEventPhaseCancelled || event.magnification == 0)
        return;

    auto* scene = CGlobals::GetScene();
    if (!scene || !scene->IsSceneEnabled() || scene->IsBlockInput() || gui->GetTopLevelControl())
        return;
    auto* camera = dynamic_cast<CAutoCamera*>(scene->GetCurrentCamera());
    if (!camera || camera->IsBlockInput())
        return;
    const auto mode = camera->GetCameraMode();
    if (mode != CameraFollowFirstPerson && mode != CameraFollowThirdPerson)
        return;
    // Paracraft can bind the wheel to inventory even while the follow camera is interactive.
    if (!camera->GetEnableMouseWheel() && !camera->IsEnableMouseRightDrag() && !camera->IsEnableMouseLeftDrag())
        return;
    const double maxDistance = camera->GetMaxCameraObjectDistance();
    if (maxDistance <= 0.5) // Fixed first-person radius in the follow camera.
        return;
    const double minDistance = std::max(camera->GetMinCameraObjectDistance(), (double)camera->GetNearPlane());
    const double distance = camera->GetCameraObjectDistance() * std::exp(-event.magnification);
    camera->SetCameraObjectDistance(std::min(maxDistance, std::max(minDistance, distance)));
}

void RenderWindowOSX::EndTrackpadDrag()
{
    if (m_trackpadDrag == TrackpadDrag::Active)
    {
        OnMouseButton(EMouseButton::RIGHT, EKeyState::RELEASE, m_dragPosition.x, m_dragPosition.y);
        m_trackpadDrag = TrackpadDrag::Released;
    }
}

void RenderWindowOSX::CancelScrollGesture()
{
    m_magnifyingWorld = false;
    EndTrackpadDrag();
    if (m_scrollGestureOverUI)
    {
        m_scrollGestureOverUI = false;
        CVariable tracking;
        tracking = false;
        CGUIRoot::GetInstance()->SetDynamicField("TrackpadScrollActive", tracking);
    }
}

bool RenderWindowOSX::SampleTrackpadDrag(DeviceMouseState& frame)
{
    if (m_trackpadDrag == TrackpadDrag::Idle)
        return false;

    // Camera motion is relative to the preceding frame, independent of cursor lock
    // and of how many Cocoa events arrived between the two samples.
    frame.x = std::lround(m_dragPosition.x) - std::lround(m_lastDragSample.x);
    frame.y = std::lround(m_dragPosition.y) - std::lround(m_lastDragSample.y);
    m_lastDragSample = m_dragPosition;
    if (m_trackpadDrag == TrackpadDrag::Active)
        frame.buttons[(int)EMouseButton::RIGHT] = EKeyState::PRESS;
    else
        m_trackpadDrag = TrackpadDrag::Idle;
    return true;
}

 void RenderWindowOSX::OnInsertText(NSString* string)
{
    auto length = [string length];
    
    for (NSUInteger i = 0; i < length; i++)
    {
        auto codepoint = [string characterAtIndex:i];
        if ((codepoint & 0xff00) == 0xf700)
            continue;
        
        if (codepoint < 32 || (codepoint > 126 && codepoint < 160))
            continue;
        
        OnChar(codepoint);
    }
}

bool RenderWindowOSX::ShouldAcceptTextInput() const
{
    if (!CGlobals::GetApp() || CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
        return false;

    auto* pRoot = CGUIRoot::GetInstance();
    if (!pRoot)
        return false;

    auto* pGUI = pRoot->GetUIKeyFocus();
    return pGUI != nullptr && pGUI->GetVisibleRecursive();
}

bool RenderWindowOSX::ShouldEnableIME() const
{
    if (!ShouldAcceptTextInput())
        return false;

    auto* pGUI = CGUIRoot::GetInstance()->GetUIKeyFocus();
    return pGUI != nullptr && pGUI->IsInputMethodEnabled();
}

void RenderWindowOSX::SyncIMEState()
{
    if (!m_window || !m_window.contentView)
        return;

    if ([m_window.contentView isKindOfClass:[RenderView class]])
    {
        RenderView* renderView = (RenderView*)m_window.contentView;
        [renderView setIMEEnabled:(ShouldEnableIME() ? YES : NO)];
    }
}

NSRect RenderWindowOSX::GetCharacterRect()
{
    const NSRect frame = [m_window frame];
    auto defRect = NSMakeRect(frame.origin.x, frame.origin.y, 0, 0);
    
    if (!CGlobals::GetApp() || CGlobals::GetApp()->GetAppState() != PEAppState_Ready || !CGUIRoot::GetInstance())
    {
        return defRect;
    }
    
    auto pGUI = CGUIRoot::GetInstance()->GetUIKeyFocus();
    auto pEdit = dynamic_cast<CGUIEditBox*>(pGUI);
    
    if (pEdit)
    {
        float fScaleX = 1.0f;
        float fScaleY = 1.0f;
        CGUIRoot::GetInstance()->GetUIScale(&fScaleX, &fScaleY);

        auto& rect = pEdit->GetCaretRect();
        int x = (int)(rect.left * fScaleX);
        int y = (int)GetHeight() - (int)((rect.bottom + 5) * fScaleY);
        
        auto point = NSMakePoint(x, y);
        point = [m_window.contentView convertPointFromBacking:point];
        point = [m_window.contentView convertPoint:point toView:nil];
        point = [m_window convertPointToScreen:point];
        
        return NSMakeRect(point.x, point.y, 0, 0);
    }
    else if (pGUI)
    {
        QPoint pt = pGUI->GetCompositionPoint();
        int x = pt.x();
        int y = (int)GetHeight() - (pt.y() + 5);

        auto point = NSMakePoint(x, y);
        point = [m_window.contentView convertPointFromBacking:point];
        point = [m_window.contentView convertPoint:point toView:nil];
        point = [m_window convertPointToScreen:point];

        return NSMakeRect(point.x, point.y, 0, 0);
    }
    
    return defRect;
}

void RenderWindowOSX::PollEvents()
{
    if (m_window == nullptr || ShouldClose()) return;

    SyncIMEState();

    if (m_startupLuaSyncFrames > 0 && CGlobals::GetApp() &&
        CGlobals::GetApp()->GetAppState() == PEAppState_Ready)
    {
        --m_startupLuaSyncFrames;
        UpdateAutoUIScaling(true);
    }

    NSEvent *event;
    auto untilDate = [NSDate distantPast];
    
    do {
        event = [NSApp nextEventMatchingMask:NSEventMaskAny
                       untilDate:untilDate
                       inMode:NSDefaultRunLoopMode
                       dequeue:YES];
        if (!event) {
            break;
        }

        [NSApp sendEvent:event];
        [NSApp updateWindows];
        [event release];
    } while(event);
}

bool RenderWindowOSX::OnShouldClose()
{
    m_shouldClose = true;
    return true;
}

void RenderWindowOSX::OnMouseButton(ParaEngine::EMouseButton button, ParaEngine::EKeyState state, int x, int y)
{
 
    if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
    {
        return;
    }

    CGUIRoot::GetInstance()
        ->GetMouse()
        ->PushMouseEvent(DeviceMouseEventPtr(new DeviceMouseButtonEvent(button,state,x,y)));
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


void RenderWindowOSX::SendWheel(float deltaX, float deltaY, bool precise)
{
    if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
    {
        return;
    }
    
    if (int ticks = static_cast<int>(std::lround(deltaX * 120)))
        CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(DeviceMouseEventPtr(new DeviceMouseWheelEvent(ticks, true, precise)));

    if (int ticks = static_cast<int>(std::lround(deltaY * 120)))
    {
        CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(DeviceMouseEventPtr(new DeviceMouseWheelEvent(ticks, false, precise)));
    }
}


void RenderWindowOSX::OnMouseMove(int x, int y)
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

const char* RenderWindowOSX::getTitle()
{
    const char* ret = [[m_window title] UTF8String];
    return ret;
}
