#import <Cocoa/Cocoa.h>
#include "ParaAppOSX.h"
#include "RenderWindowOSX.h"
#import "MidiPlayer/ParaEngineMediaPlayer.h"
#include "MidiMsg.h"

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
    m_fRefreshTimerInterval = fTimeInterval;
    m_nFrameRateControl = nFrameRateControl;
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
        output = "OpenGL (macOS)";
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

void ParaEngine::CParaEngineAppOSX::handle_mainloop_timer(const boost::system::error_code& err)
{
    if (!err)
    {
        auto pWindow = (RenderWindowOSX*)m_pRenderWindow;
        if (pWindow && !pWindow->ShouldClose())
        {
            pWindow->PollEvents();
            this->DoWork();

            double fNextInterval = 0.01;
            NextLoop((int)(fNextInterval * 1000), &CParaEngineAppOSX::handle_mainloop_timer, this);
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
    renderWindow->SetScreenResolution(vSize.x, vSize.y);
}
