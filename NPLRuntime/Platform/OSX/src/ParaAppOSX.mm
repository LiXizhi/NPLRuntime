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
    //throw std::logic_error("The method or operation is not implemented.");
}

bool ParaEngine::CParaEngineAppOSX::AppHasFocus()
{
    //throw std::logic_error("The method or operation is not implemented.");
    return true;
}

void ParaEngine::CParaEngineAppOSX::GetStats(string& output, DWORD dwFields)
{
    //throw std::logic_error("The method or operation is not implemented.");
}

bool ParaEngine::CParaEngineAppOSX::WriteRegStr(const string& root_key, const string& sSubKey, const string& name, const string& value)
{
    //throw std::logic_error("The method or operation is not implemented.");
    return true;
}

void ParaEngine::CParaEngineAppOSX::SetAutoLowerFrameRateWhenNotFocused(bool bEnabled)
{
    //throw std::logic_error("The method or operation is not implemented.");
}

const char* ParaEngine::CParaEngineAppOSX::ReadRegStr(const string& root_key, const string& sSubKey, const string& name)
{
    //throw std::logic_error("The method or operation is not implemented.");
    return "";
}

bool ParaEngine::CParaEngineAppOSX::WriteRegDWORD(const string& root_key, const string& sSubKey, const string& name, DWORD value)
{
    //throw std::logic_error("The method or operation is not implemented.");
    return true;
}

DWORD ParaEngine::CParaEngineAppOSX::ReadRegDWORD(const string& root_key, const string& sSubKey, const string& name)
{
    //throw std::logic_error("The method or operation is not implemented.");
    return  0;
}

bool ParaEngine::CParaEngineAppOSX::GetAutoLowerFrameRateWhenNotFocused()
{
    //throw std::logic_error("The method or operation is not implemented.");
    return true;
}

void ParaEngine::CParaEngineAppOSX::SetToggleSoundWhenNotFocused(bool bEnabled)
{
    //	throw std::logic_error("The method or operation is not implemented.");

}

bool ParaEngine::CParaEngineAppOSX::GetToggleSoundWhenNotFocused()
{
    //throw std::logic_error("The method or operation is not implemented.");
    return true;
}

void ParaEngine::CParaEngineAppOSX::handle_mainloop_timer(const boost::system::error_code& err)
{
    if (!err)
    {
        auto pWindow = (RenderWindowOSX*)m_pRenderWindow;
        if (!pWindow->ShouldClose())
        {
            pWindow->PollEvents();
            this->DoWork();

            double fNextInterval = 0.01;
            //fNextInterval = this->GetRefreshTimer() - (ParaTimer::GetAbsoluteTime() - this->GetAppTime());
            //fNextInterval = (std::min)(0.1, (std::max)(0.0, fNextInterval));
            
            NextLoop((int)(fNextInterval * 1000), &CParaEngineAppOSX::handle_mainloop_timer, this);
        }
    }
}

int ParaEngine::CParaEngineAppOSX::Run(HINSTANCE hInstance)
{
    NextLoop(50, &CParaEngineAppOSX::handle_mainloop_timer, this);
    MainLoopRun();
    
    return 0;
}

void ParaEngine::CParaEngineAppOSX::Exit(int nReturnCode /*= 0*/)
{
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
