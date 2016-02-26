//-----------------------------------------------------------------------------
// Class:	IObjectScriptingInterface
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Co.
// Date:	2013.2.1
//-----------------------------------------------------------------------------

#include"ParaEngine.h"
#include "NPLRuntime.h"
#include "util/StringHelper.h"

#include "IObjectScriptingInterface.h"



using namespace ParaEngine;

IObjectScriptingInterface::IObjectScriptingInterface()
{

}

bool IObjectScriptingInterface::AddScriptCallback( int func_type, const string& script_func )
{
	if(script_func.empty())
	{
		RemoveScriptCallback(func_type);
		return true;
	}
	ObjectCallbackPool_Type::iterator curIt, endIt = m_callback_scripts.end();
	for (curIt=m_callback_scripts.begin(); curIt!=endIt; ++curIt)
	{
		if((*curIt).func_type == func_type){
			(*curIt).SetScriptFunc(script_func);
			return true;
		}
	}
	m_callback_scripts.push_back(ScriptCallback());
	ScriptCallback& item = *(m_callback_scripts.end()-1);
	item.func_type = func_type;
	item.SetScriptFunc(script_func);
	return true;
}

IObjectScriptingInterface::ScriptCallback* IObjectScriptingInterface::GetScriptCallback( int func_type )
{
	ObjectCallbackPool_Type::iterator curIt, endIt = m_callback_scripts.end();
	for (curIt=m_callback_scripts.begin(); curIt!=endIt; ++curIt)
	{
		if((*curIt).func_type == func_type){
			return &(*curIt);
		}
	}
	return NULL;
}

bool IObjectScriptingInterface::RemoveScriptCallback( int func_type )
{
	ObjectCallbackPool_Type::iterator curIt, endIt = m_callback_scripts.end();
	for (curIt=m_callback_scripts.begin(); curIt!=endIt; ++curIt)
	{
		if((*curIt).func_type == func_type){
			m_callback_scripts.erase(curIt);
			return true;
		}
	}
	return false;
}

void ParaEngine::IObjectScriptingInterface::ScriptCallback::SetScriptFunc( const std::string& script )
{
	script_func = script;
	m_sFilename.clear();
	m_sCode.clear();
	StringHelper::DevideString(script_func, m_sFilename, m_sCode, ';');
}

int ParaEngine::IObjectScriptingInterface::ScriptCallback::ActivateAsync(const std::string& code)
{
	if (!code.empty())
		return CGlobals::GetNPLRuntime()->Activate(CGlobals::GetNPLRuntime()->GetMainRuntimeState().get(), m_sFilename.c_str(), code.c_str(), code.size());
	else
		return CGlobals::GetNPLRuntime()->Activate(CGlobals::GetNPLRuntime()->GetMainRuntimeState().get(), m_sFilename.c_str());
}

int ParaEngine::IObjectScriptingInterface::ScriptCallback::ActivateLocalNow(const std::string& script)
{
	if (!script.empty())
		return CGlobals::GetNPLRuntime()->ActivateLocalNow(m_sFilename.c_str(), script.c_str(), script.size());
	else
		return CGlobals::GetNPLRuntime()->ActivateLocalNow(m_sFilename.c_str());
}
