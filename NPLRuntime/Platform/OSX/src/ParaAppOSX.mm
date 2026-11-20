#import <Cocoa/Cocoa.h>
#include "ParaAppOSX.h"
#include "RenderWindowOSX.h"
#import "MidiPlayer/ParaEngineMediaPlayer.h"
#include "MidiMsg.h"
#include "FramePacing.h"
#import <QuartzCore/CAMetalLayer.h>
#include "RenderDeviceAGL.h"
#include <chrono>

using namespace  ParaEngine;

IParaEngineApp* CreateParaEngineApp()
{
    return new CParaEngineAppOSX();
}

ParaEngine::CParaEngineAppOSX::CParaEngineAppOSX()
{
    // SetTouchInputting(true);
    CMidiMsg::GetSingleton().SetMediaPlayer(ParaEngineMediaPlayer::GetSingleton());
}

bool ParaEngine::CParaEngineAppOSX::InitApp(IRenderWindow* pWindow, const char* sCommandLine /*= nullptr*/)
{
    if (!CParaEngineAppBase::InitApp(pWindow, sCommandLine))
        return false;

    if (strcmp(GetAppCommandLineByParam("profile_unlimited_fps", "false"), "true") == 0)
        CGlobals::GetSettings().SetProfileUnlimitedFPS(true);

    if (auto* osxWindow = static_cast<RenderWindowOSX*>(pWindow))
    {
        osxWindow->UpdateAutoUIScaling(true);
    }
    return true;
}

void ParaEngine::CParaEngineAppOSX::GameToClient(int& inout_x, int & inout_y, bool bInBackbuffer /*= true*/)
{
    //throw std::logic_error("The method or operation is not implemented.");
}

void ParaEngine::CParaEngineAppOSX::ClientToGame(int& inout_x, int & inout_y, bool bInBackbuffer /*= true*/)
{
    //throw std::logic_error("The method or operation is not implemented.");
}

void ParaEngine::CParaEngineAppOSX::SetRefreshTimer(float fTimeInterval, int nFrameRateControl /*= 0*/)
{
    CParaEngineAppBase::SetRefreshTimer(fTimeInterval, nFrameRateControl);
    if (m_profileUnlimitedFPS)
        m_doWorkFRC.SetType(CFrameRateController::FRC_NONE);
}

bool ParaEngine::CParaEngineAppOSX::AppHasFocus()
{
    if ([NSApp isActive]) {
        return true;
    } else {
        return false;
    }
}

void ParaEngine::CParaEngineAppOSX::GetStats(string& output, DWORD dwFields)
{
    if (dwFields == 0)
    {
        if (m_pRenderDevice && m_pRenderDevice->GetCaps().backendName)
            output = std::string(m_pRenderDevice->GetCaps().backendName) + " (macOS)";
        else
            output = "Metal (macOS)";
    }
}

static NSString* MakeOSXRegDefaultsKey(const string& root_key, const string& sSubKey, const string& name)
{
    std::string combined = "ParaReg/" + root_key + "/" + sSubKey + "/" + name;
    for (size_t i = 0; i < combined.size(); ++i)
    {
        if (combined[i] == '\\')
            combined[i] = '/';
    }
    return [NSString stringWithUTF8String:combined.c_str()];
}

bool ParaEngine::CParaEngineAppOSX::WriteRegStr(const string& root_key, const string& sSubKey, const string& name, const string& value)
{
    NSString* nsKey = MakeOSXRegDefaultsKey(root_key, sSubKey, name);
    NSString* nsVal = [NSString stringWithUTF8String:value.c_str()];
    if (!nsKey || !nsVal)
        return false;
    [[NSUserDefaults standardUserDefaults] setObject:nsVal forKey:nsKey];
    [[NSUserDefaults standardUserDefaults] synchronize];
    return true;
}

void ParaEngine::CParaEngineAppOSX::SetAutoLowerFrameRateWhenNotFocused(bool bEnabled)
{
    m_bAutoLowerFrameRateWhenNotFocused = bEnabled;
}

const char* ParaEngine::CParaEngineAppOSX::ReadRegStr(const string& root_key, const string& sSubKey, const string& name)
{
    static std::string s_lastRegStr;
    NSString* nsKey = MakeOSXRegDefaultsKey(root_key, sSubKey, name);
    if (!nsKey)
        return nullptr;
    NSString* nsVal = [[NSUserDefaults standardUserDefaults] stringForKey:nsKey];
    if (!nsVal)
        return nullptr;
    s_lastRegStr = [nsVal UTF8String];
    return s_lastRegStr.c_str();
}

bool ParaEngine::CParaEngineAppOSX::WriteRegDWORD(const string& root_key, const string& sSubKey, const string& name, DWORD value)
{
    NSString* nsKey = MakeOSXRegDefaultsKey(root_key, sSubKey, name);
    if (!nsKey)
        return false;
    [[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithUnsignedLong:value] forKey:nsKey];
    [[NSUserDefaults standardUserDefaults] synchronize];
    return true;
}

DWORD ParaEngine::CParaEngineAppOSX::ReadRegDWORD(const string& root_key, const string& sSubKey, const string& name)
{
    NSString* nsKey = MakeOSXRegDefaultsKey(root_key, sSubKey, name);
    if (!nsKey)
        return 0;
    id obj = [[NSUserDefaults standardUserDefaults] objectForKey:nsKey];
    if ([obj isKindOfClass:[NSNumber class]])
        return (DWORD)[(NSNumber*)obj unsignedLongValue];
    return 0;
}

bool ParaEngine::CParaEngineAppOSX::GetAutoLowerFrameRateWhenNotFocused()
{
    return m_bAutoLowerFrameRateWhenNotFocused;
}

void ParaEngine::CParaEngineAppOSX::SetToggleSoundWhenNotFocused(bool bEnabled)
{
    m_bToggleSoundWhenNotFocused = bEnabled;
}

bool ParaEngine::CParaEngineAppOSX::GetToggleSoundWhenNotFocused()
{
    return m_bToggleSoundWhenNotFocused;
}

void ParaEngine::CParaEngineAppOSX::UpdateProfilingFrameRate()
{
    const bool enabled = CGlobals::GetSettings().GetProfileUnlimitedFPS();
    if (enabled == m_profileUnlimitedFPS)
        return;

    m_profileUnlimitedFPS = enabled;
    // Keep script changes to RefreshTimer and its control mode for when profiling ends.
    m_doWorkFRC.SetType(!enabled && m_nFrameRateControl == 1
        ? CFrameRateController::FRC_CONSTANT_OR_BELOW : CFrameRateController::FRC_NONE);
    if (m_pRenderDevice && m_pRenderDevice->GetBackendType() == ERHIBackend::OpenGL)
    {
        NSOpenGLContext* context = static_cast<RenderDeviceAGL*>(m_pRenderDevice)->GetGLContext();
        if (enabled)
            [context getValues:&m_savedSwapInterval forParameter:NSOpenGLContextParameterSwapInterval];
        GLint interval = enabled ? 0 : m_savedSwapInterval;
        [context setValues:&interval forParameter:NSOpenGLContextParameterSwapInterval];
    }
    else if (m_pRenderDevice && m_pRenderDevice->GetBackendType() == ERHIBackend::Metal)
    {
        NSWindow* window = (NSWindow*)m_pRenderWindow->GetNativeHandle();
        CAMetalLayer* layer = (CAMetalLayer*)window.contentView.layer;
        if ([layer respondsToSelector:@selector(setDisplaySyncEnabled:)])
        {
            if (enabled)
                m_savedSwapInterval = layer.displaySyncEnabled;
            layer.displaySyncEnabled = enabled ? NO : (m_savedSwapInterval != 0);
        }
    }
    OUTPUT_LOG("Profiling unlimited FPS %s\n", enabled ? "enabled" : "disabled");
}

void ParaEngine::CParaEngineAppOSX::handle_mainloop_timer(const boost::system::error_code& err)
{
    if (!err)
    {
        auto pWindow = (RenderWindowOSX*)m_pRenderWindow;
        if (pWindow && !pWindow->ShouldClose())
        {
            const auto frameStart = std::chrono::steady_clock::now();
            pWindow->PollEvents();
            if (m_pRenderDevice && GetAppState() == PEAppState_Ready)
            {
                const uint32_t winW = pWindow->GetWidth();
                const uint32_t winH = pWindow->GetHeight();
                if (winW > 0 && winH > 0 &&
                    (winW != m_pRenderDevice->GetBackBufferWidth() || winH != m_pRenderDevice->GetBackBufferHeight()))
                {
                    m_pRenderDevice->Flush();
                    ResetRenderEnvironment();
                    ParaViewport vp = m_pRenderDevice->GetViewport();
                    vp.X = 0;
                    vp.Y = 0;
                    vp.Width = winW;
                    vp.Height = winH;
                    m_pRenderDevice->SetViewport(vp);
                    pWindow->UpdateAutoUIScaling(true);
                }
            }
            UpdateProfilingFrameRate();
            this->DoWork();

            // Render/Present may already consume the frame budget (including
            // display sync). Wait only for the remainder, not another 10 ms.
            const double elapsed = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - frameStart).count();
            const int delayMs = m_profileUnlimitedFPS ? 0 : GetRemainingFrameDelayMs(GetRefreshTimer(), elapsed);
            NextLoop(delayMs, &CParaEngineAppOSX::handle_mainloop_timer, this);
        }
        else
        {
            MainLoopStop();
        }
    }
}

int ParaEngine::CParaEngineAppOSX::Run(HINSTANCE hInstance)
{
    NextLoop(50, &CParaEngineAppOSX::handle_mainloop_timer, this);
    MainLoopRun();
    FinalCleanup();
    
    return GetReturnCode();
}

void ParaEngine::CParaEngineAppOSX::StopApp()
{
    CParaEngineAppBase::StopApp();
    MainLoopStop();
}

void ParaEngine::CParaEngineAppOSX::Exit(int nReturnCode /*= 0*/)
{
    SetReturnCode(nReturnCode);
    StopApp();
    [[NSRunningApplication currentApplication] terminate];
}

/** set the window title when at windowed mode */
void ParaEngine::CParaEngineAppOSX::SetWindowText(const char* pChar)
{
    auto pWindow = (RenderWindowOSX*)m_pRenderWindow;
    pWindow->setTitle(pChar);
    
}

/** get the window title when at windowed mode */
const char* ParaEngine::CParaEngineAppOSX::GetWindowText()
{
    auto pWindow = (RenderWindowOSX*)m_pRenderWindow;
    return pWindow->getTitle();
}

void ParaEngine::CParaEngineAppOSX::GetScreenResolution(Vector2* pOut)
{
    if (pOut) {
        RenderWindowOSX *renderWindow = (RenderWindowOSX*)m_pRenderWindow;
        renderWindow->GetScreenResolution(&(pOut->x), &(pOut->y));
    }
}

void ParaEngine::CParaEngineAppOSX::SetScreenResolution(const Vector2& vSize)
{
    RenderWindowOSX *renderWindow = (RenderWindowOSX*)m_pRenderWindow;
    if (renderWindow)
        renderWindow->SetScreenResolution(vSize.x, vSize.y);
}

void ParaEngine::CParaEngineAppOSX::GetVisibleSize(Vector2* pOut)
{
    if (pOut && m_pRenderWindow)
    {
        pOut->x = static_cast<float>(m_pRenderWindow->GetWidth());
        pOut->y = static_cast<float>(m_pRenderWindow->GetHeight());
    }
}

bool ParaEngine::CParaEngineAppOSX::IsWindowMaximized()
{
    if (!m_pRenderWindow)
        return false;
    NSWindow* win = (NSWindow*)m_pRenderWindow->GetNativeHandle();
    if (!win)
        return false;
    return [win isZoomed] || (([win styleMask] & NSWindowStyleMaskFullScreen) != 0);
}

void ParaEngine::CParaEngineAppOSX::SetWindowMaximized(bool isMaximized)
{
    if (!m_pRenderWindow)
        return;
    NSWindow* win = (NSWindow*)m_pRenderWindow->GetNativeHandle();
    if (!win)
        return;
    const bool currentlyZoomed = ([win isZoomed] != NO);
    if (currentlyZoomed != isMaximized && (([win styleMask] & NSWindowStyleMaskFullScreen) == 0))
    {
        [win zoom:nil];
    }
}
