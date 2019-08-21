#include "ParaAppiOS.h"
#include "RenderWindowiOS.h"


IParaEngineApp* CreateParaEngineApp()
{
    return new CParaEngineAppiOS();
}


namespace  ParaEngine {


CParaEngineAppiOS::CParaEngineAppiOS()
{
    SetTouchInputting(true);
}

void CParaEngineAppiOS::GameToClient(int& inout_x, int & inout_y, bool bInBackbuffer /*= true*/)
{
    //throw std::logic_error("The method or operation is not implemented.");
}

void CParaEngineAppiOS::ClientToGame(int& inout_x, int & inout_y, bool bInBackbuffer /*= true*/)
{
    //throw std::logic_error("The method or operation is not implemented.");
}

void CParaEngineAppiOS::SetRefreshTimer(float fTimeInterval, int nFrameRateControl /*= 0*/)
{
    //throw std::logic_error("The method or operation is not implemented.");
}

bool CParaEngineAppiOS::AppHasFocus()
{
    //throw std::logic_error("The method or operation is not implemented.");
    return true;
}

void CParaEngineAppiOS::GetStats(string& output, DWORD dwFields)
{
    //throw std::logic_error("The method or operation is not implemented.");
}

bool CParaEngineAppiOS::WriteRegStr(const string& root_key, const string& sSubKey, const string& name, const string& value)
{
    //throw std::logic_error("The method or operation is not implemented.");
    return true;
}

void CParaEngineAppiOS::SetAutoLowerFrameRateWhenNotFocused(bool bEnabled)
{
    //throw std::logic_error("The method or operation is not implemented.");
}

const char* CParaEngineAppiOS::ReadRegStr(const string& root_key, const string& sSubKey, const string& name)
{
    //throw std::logic_error("The method or operation is not implemented.");
    return "";
}

bool CParaEngineAppiOS::WriteRegDWORD(const string& root_key, const string& sSubKey, const string& name, DWORD value)
{
    //throw std::logic_error("The method or operation is not implemented.");
    return true;
}

DWORD CParaEngineAppiOS::ReadRegDWORD(const string& root_key, const string& sSubKey, const string& name)
{
    //throw std::logic_error("The method or operation is not implemented.");
    return  0;
}

bool CParaEngineAppiOS::GetAutoLowerFrameRateWhenNotFocused()
{
    //throw std::logic_error("The method or operation is not implemented.");
    return true;
}

void CParaEngineAppiOS::SetToggleSoundWhenNotFocused(bool bEnabled)
{
    //    throw std::logic_error("The method or operation is not implemented.");
    
}

bool CParaEngineAppiOS::GetToggleSoundWhenNotFocused()
{
    //throw std::logic_error("The method or operation is not implemented.");
    return true;
}

int CParaEngineAppiOS::Run(HINSTANCE hInstance)
{
 
    return 0;
}

void CParaEngineAppiOS::setIMEKeyboardState(bool bOpen)
{
     auto delegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
    [delegate setIMEKeyboardState:bOpen];
}

} // namespace  ParaEngine



