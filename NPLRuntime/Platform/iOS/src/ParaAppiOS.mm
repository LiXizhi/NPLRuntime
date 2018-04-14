
#include "ParaAppiOS.h"
#include "RenderWindowiOS.h"
using namespace  ParaEngine;

IParaEngineApp* CreateParaEngineApp()
{
    return new CParaEngineAppiOS();
}

ParaEngine::CParaEngineAppiOS::CParaEngineAppiOS()
{
    // SetTouchInputting(true);
}

void ParaEngine::CParaEngineAppiOS::GameToClient(int& inout_x, int & inout_y, bool bInBackbuffer /*= true*/)
{
    //throw std::logic_error("The method or operation is not implemented.");
}

void ParaEngine::CParaEngineAppiOS::ClientToGame(int& inout_x, int & inout_y, bool bInBackbuffer /*= true*/)
{
    //throw std::logic_error("The method or operation is not implemented.");
}

void ParaEngine::CParaEngineAppiOS::SetRefreshTimer(float fTimeInterval, int nFrameRateControl /*= 0*/)
{
    //throw std::logic_error("The method or operation is not implemented.");
}

bool ParaEngine::CParaEngineAppiOS::AppHasFocus()
{
    //throw std::logic_error("The method or operation is not implemented.");
    return true;
}

void ParaEngine::CParaEngineAppiOS::GetStats(string& output, DWORD dwFields)
{
    //throw std::logic_error("The method or operation is not implemented.");
}

bool ParaEngine::CParaEngineAppiOS::WriteRegStr(const string& root_key, const string& sSubKey, const string& name, const string& value)
{
    //throw std::logic_error("The method or operation is not implemented.");
    return true;
}

void ParaEngine::CParaEngineAppiOS::SetAutoLowerFrameRateWhenNotFocused(bool bEnabled)
{
    //throw std::logic_error("The method or operation is not implemented.");
}

const char* ParaEngine::CParaEngineAppiOS::ReadRegStr(const string& root_key, const string& sSubKey, const string& name)
{
    //throw std::logic_error("The method or operation is not implemented.");
    return "";
}

bool ParaEngine::CParaEngineAppiOS::WriteRegDWORD(const string& root_key, const string& sSubKey, const string& name, DWORD value)
{
    //throw std::logic_error("The method or operation is not implemented.");
    return true;
}

DWORD ParaEngine::CParaEngineAppiOS::ReadRegDWORD(const string& root_key, const string& sSubKey, const string& name)
{
    //throw std::logic_error("The method or operation is not implemented.");
    return  0;
}

bool ParaEngine::CParaEngineAppiOS::GetAutoLowerFrameRateWhenNotFocused()
{
    //throw std::logic_error("The method or operation is not implemented.");
    return true;
}

void ParaEngine::CParaEngineAppiOS::SetToggleSoundWhenNotFocused(bool bEnabled)
{
    //    throw std::logic_error("The method or operation is not implemented.");
    
}

bool ParaEngine::CParaEngineAppiOS::GetToggleSoundWhenNotFocused()
{
    //throw std::logic_error("The method or operation is not implemented.");
    return true;
}

int ParaEngine::CParaEngineAppiOS::Run(HINSTANCE hInstance)
{
    return 0;
}
