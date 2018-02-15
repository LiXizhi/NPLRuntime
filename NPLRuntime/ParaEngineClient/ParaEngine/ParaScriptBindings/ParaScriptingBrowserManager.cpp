//----------------------------------------------------------------------
// ParaScriptingGUI
// Authors:	LiXizhi
// Company: ParaEngine
// Date:	2005.9.8
// Desc: 
// Containing the interface between script and the engine. 
//----------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_DIRECTX_RENDERER
#include "HTMLBrowserManager.h"
#include "ParaWorldAsset.h"
#include "NPLHelper.h"
#include "util/StringHelper.h"
#include "ParaScriptingBrowserManager.h"

extern "C"
{
#include <lua.h>
}
#include <luabind/luabind.hpp>

using namespace ParaScripting;

ParaScripting::ParaHTMLBrowser ParaBrowserManager::createBrowserWindow(const char* sFileName, int browserWindowWidth, int browserWindowHeight)
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().createBrowserWindow(sFileName, browserWindowWidth, browserWindowHeight);
	return ParaHTMLBrowser((pBrowser != 0) ? pBrowser->GetBrowserWindowID() : -1);
}

ParaScripting::ParaHTMLBrowser ParaBrowserManager::GetBrowserWindow(const char* sFileName)
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(sFileName);
	return ParaHTMLBrowser((pBrowser != 0) ? pBrowser->GetBrowserWindowID() : -1);
}

ParaScripting::ParaHTMLBrowser ParaBrowserManager::GetBrowserWindow1(int nWindowID)
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(nWindowID);
	return ParaHTMLBrowser((pBrowser != 0) ? pBrowser->GetBrowserWindowID() : -1);
}

bool ParaBrowserManager::reset()
{
	return CGlobals::GetAssetManager()->GetHTMLBrowserManager().reset();
}

bool ParaBrowserManager::clearCache()
{
	return CGlobals::GetAssetManager()->GetHTMLBrowserManager().clearCache();
}

int ParaBrowserManager::getLastError()
{
	return CGlobals::GetAssetManager()->GetHTMLBrowserManager().getLastError();
}

const char* ParaBrowserManager::getVersion()
{
	static string g_str = CGlobals::GetAssetManager()->GetHTMLBrowserManager().getVersion();
	return g_str.c_str();
}

int ParaBrowserManager::GetMaxWindowNum()
{
	return CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetMaxWindowNum();
}

void ParaBrowserManager::SetMaxWindowNum(int nNum)
{
	CGlobals::GetAssetManager()->GetHTMLBrowserManager().SetMaxWindowNum(nNum);
}

void ParaBrowserManager::onPageChanged(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	if (strScriptName == NULL) {
		CGlobals::GetAssetManager()->GetHTMLBrowserManager().SetEventScript(CHTMLBrowserManager::EM_onPageChanged, NULL);
		return;
	}
	string temp(strScriptName);
	SimpleScript script;
	ParaEngine::StringHelper::DevideString(temp, script.szFile, script.szCode);
	CGlobals::GetAssetManager()->GetHTMLBrowserManager().SetEventScript(CHTMLBrowserManager::EM_onPageChanged, &script);
}

void ParaBrowserManager::onNavigateBegin(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	if (strScriptName == NULL) {
		CGlobals::GetAssetManager()->GetHTMLBrowserManager().SetEventScript(CHTMLBrowserManager::EM_onNavigateBegin, NULL);
		return;
	}
	string temp(strScriptName);
	SimpleScript script;
	ParaEngine::StringHelper::DevideString(temp, script.szFile, script.szCode);
	CGlobals::GetAssetManager()->GetHTMLBrowserManager().SetEventScript(CHTMLBrowserManager::EM_onNavigateBegin, &script);
}

void ParaBrowserManager::onNavigateComplete(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	if (strScriptName == NULL) {
		CGlobals::GetAssetManager()->GetHTMLBrowserManager().SetEventScript(CHTMLBrowserManager::EM_onNavigateComplete, NULL);
		return;
	}
	string temp(strScriptName);
	SimpleScript script;
	ParaEngine::StringHelper::DevideString(temp, script.szFile, script.szCode);
	CGlobals::GetAssetManager()->GetHTMLBrowserManager().SetEventScript(CHTMLBrowserManager::EM_onNavigateComplete, &script);
}

void ParaBrowserManager::onUpdateProgress(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	if (strScriptName == NULL) {
		CGlobals::GetAssetManager()->GetHTMLBrowserManager().SetEventScript(CHTMLBrowserManager::EM_onUpdateProgress, NULL);
		return;
	}
	string temp(strScriptName);
	SimpleScript script;
	ParaEngine::StringHelper::DevideString(temp, script.szFile, script.szCode);
	CGlobals::GetAssetManager()->GetHTMLBrowserManager().SetEventScript(CHTMLBrowserManager::EM_onUpdateProgress, &script);
}

void ParaBrowserManager::onStatusTextChange(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	if (strScriptName == NULL) {
		CGlobals::GetAssetManager()->GetHTMLBrowserManager().SetEventScript(CHTMLBrowserManager::EM_onStatusTextChange, NULL);
		return;
	}
	string temp(strScriptName);
	SimpleScript script;
	ParaEngine::StringHelper::DevideString(temp, script.szFile, script.szCode);
	CGlobals::GetAssetManager()->GetHTMLBrowserManager().SetEventScript(CHTMLBrowserManager::EM_onStatusTextChange, &script);
}

void ParaBrowserManager::onLocationChange(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	if (strScriptName == NULL) {
		CGlobals::GetAssetManager()->GetHTMLBrowserManager().SetEventScript(CHTMLBrowserManager::EM_onLocationChange, NULL);
		return;
	}
	string temp(strScriptName);
	SimpleScript script;
	ParaEngine::StringHelper::DevideString(temp, script.szFile, script.szCode);
	CGlobals::GetAssetManager()->GetHTMLBrowserManager().SetEventScript(CHTMLBrowserManager::EM_onLocationChange, &script);
}

void ParaBrowserManager::onClickLinkHref(const object& objScriptName)
{
	const char* strScriptName = NPL::NPLHelper::LuaObjectToString(objScriptName);
	if (strScriptName == NULL) {
		CGlobals::GetAssetManager()->GetHTMLBrowserManager().SetEventScript(CHTMLBrowserManager::EM_onClickLinkHref, NULL);
		return;
	}
	string temp(strScriptName);
	SimpleScript script;
	ParaEngine::StringHelper::DevideString(temp, script.szFile, script.szCode);
	CGlobals::GetAssetManager()->GetHTMLBrowserManager().SetEventScript(CHTMLBrowserManager::EM_onClickLinkHref, &script);
}

#endif



