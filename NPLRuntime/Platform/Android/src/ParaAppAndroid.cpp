#include "ParaAppAndroid.h"
using namespace  ParaEngine;



ParaEngine::CParaEngineAppAndroid::CParaEngineAppAndroid(struct android_app* state):m_appState(state)
{
	SetTouchInputting(true);
}

void ParaEngine::CParaEngineAppAndroid::GameToClient(int& inout_x, int & inout_y, bool bInBackbuffer /*= true*/)
{
	//throw std::logic_error("The method or operation is not implemented.");
}

void ParaEngine::CParaEngineAppAndroid::ClientToGame(int& inout_x, int & inout_y, bool bInBackbuffer /*= true*/)
{
	//throw std::logic_error("The method or operation is not implemented.");
}

void ParaEngine::CParaEngineAppAndroid::SetRefreshTimer(float fTimeInterval, int nFrameRateControl /*= 0*/)
{
	//throw std::logic_error("The method or operation is not implemented.");
}

bool ParaEngine::CParaEngineAppAndroid::AppHasFocus()
{
	//throw std::logic_error("The method or operation is not implemented.");
	return true;
}

void ParaEngine::CParaEngineAppAndroid::GetStats(string& output, DWORD dwFields)
{
	//throw std::logic_error("The method or operation is not implemented.");
}

bool ParaEngine::CParaEngineAppAndroid::WriteRegStr(const string& root_key, const string& sSubKey, const string& name, const string& value)
{
	//throw std::logic_error("The method or operation is not implemented.");
	return true;
}

void ParaEngine::CParaEngineAppAndroid::SetAutoLowerFrameRateWhenNotFocused(bool bEnabled)
{
	//throw std::logic_error("The method or operation is not implemented.");
}

const char* ParaEngine::CParaEngineAppAndroid::ReadRegStr(const string& root_key, const string& sSubKey, const string& name)
{
	//throw std::logic_error("The method or operation is not implemented.");
	return "";
}

bool ParaEngine::CParaEngineAppAndroid::WriteRegDWORD(const string& root_key, const string& sSubKey, const string& name, DWORD value)
{
	//throw std::logic_error("The method or operation is not implemented.");
	return true;
}

DWORD ParaEngine::CParaEngineAppAndroid::ReadRegDWORD(const string& root_key, const string& sSubKey, const string& name)
{
	//throw std::logic_error("The method or operation is not implemented.");
	return  0;
}

bool ParaEngine::CParaEngineAppAndroid::GetAutoLowerFrameRateWhenNotFocused()
{
	//throw std::logic_error("The method or operation is not implemented.");
	return true;
}

void ParaEngine::CParaEngineAppAndroid::SetToggleSoundWhenNotFocused(bool bEnabled)
{
	//	throw std::logic_error("The method or operation is not implemented.");

}

bool ParaEngine::CParaEngineAppAndroid::GetToggleSoundWhenNotFocused()
{
	//throw std::logic_error("The method or operation is not implemented.");
	return true;
}
