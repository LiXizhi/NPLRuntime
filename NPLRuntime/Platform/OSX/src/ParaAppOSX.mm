#include "ParaAppOSX.h"
#include "RenderWindowOSX.h"
using namespace  ParaEngine;

IParaEngineApp* CreateParaEngineApp()
{
	return new CParaEngineAppOSX();
}

ParaEngine::CParaEngineAppOSX::CParaEngineAppOSX()
{
	SetTouchInputting(true);
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

int ParaEngine::CParaEngineAppOSX::Run(HINSTANCE hInstance)
{
	auto pWindow = (RenderWindowOSX*)m_pRenderWindow;

	while (!pWindow->ShouldClose())
	{
		pWindow->PollEvents();
		this->DoWork();
	}
	
	return 0;
}
