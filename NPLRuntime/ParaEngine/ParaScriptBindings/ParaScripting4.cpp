//-----------------------------------------------------------------------------
// Class:	Mainly for jabber client scripting port
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2007.9
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "NPLRuntime.h"
#include "ParaScriptingGUI.h"

#ifdef HAS_JABBER_CLIENT
#include "ParaScriptingJabberClient.h"
#endif
#include "ParaScriptingWorld.h"
#include "ParaScriptingBlockWorld.h"
#ifdef EMSCRIPTEN_SINGLE_THREAD
#define auto_ptr unique_ptr
#endif
using namespace ParaEngine;

/** 
for luabind, The main drawback of this approach is that the compilation time will increase for the file 
that does the registration, it is therefore recommended that you register everything in the same cpp-file.
*/
extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <luabind/luabind.hpp>
#include <luabind/out_value_policy.hpp>
#include <luabind/return_reference_to_policy.hpp>
#include <luabind/copy_policy.hpp>
#include <luabind/adopt_policy.hpp>
#include <luabind/function.hpp>

namespace ParaScripting
{
	void CNPLScriptingState::LoadHAPI_UI_Extension()
	{
#ifdef PARAENGINE_CLIENT
		using namespace luabind;
		lua_State* L = GetLuaState();

		module(L)
			[
				namespace_("ParaBrowserManager")
				[
					// class declarations
					class_<ParaHTMLBrowser>("ParaHTMLBrowser")
					.def(constructor<>())
					//.property("User",&ParaHTMLBrowser::GetUser,&ParaHTMLBrowser::SetUser)

					.def("GetName", &ParaHTMLBrowser::GetName)
					.def("Release", &ParaHTMLBrowser::Release)
					.def("setSize", &ParaHTMLBrowser::setSize)
					.def("getBrowserWidth", &ParaHTMLBrowser::getBrowserWidth)
					.def("getBrowserHeight", &ParaHTMLBrowser::getBrowserHeight)
					.def("scrollByLines", &ParaHTMLBrowser::scrollByLines)
					.def("setBackgroundColor", &ParaHTMLBrowser::setBackgroundColor)
					.def("setEnabled", &ParaHTMLBrowser::setEnabled)
					.def("navigateTo", &ParaHTMLBrowser::navigateTo)
					.def("navigateStop", &ParaHTMLBrowser::navigateStop)
					.def("canNavigateBack", &ParaHTMLBrowser::canNavigateBack)
					.def("navigateBack", &ParaHTMLBrowser::navigateBack)
					.def("canNavigateForward", &ParaHTMLBrowser::canNavigateForward)
					.def("navigateForward", &ParaHTMLBrowser::navigateForward)
					.def("mouseDown", &ParaHTMLBrowser::mouseDown)
					.def("mouseUp", &ParaHTMLBrowser::mouseUp)
					.def("mouseMove", &ParaHTMLBrowser::mouseMove)
					.def("keyPress", &ParaHTMLBrowser::keyPress)
					.def("focusBrowser", &ParaHTMLBrowser::focusBrowser)
					.def("GetLastNavURL", &ParaHTMLBrowser::GetLastNavURL)
					.def("IsValid", &ParaHTMLBrowser::IsValid),

					// function declarations
					def("createBrowserWindow", & ParaBrowserManager::createBrowserWindow),
					def("GetBrowserWindow", & ParaBrowserManager::GetBrowserWindow),
					def("GetBrowserWindow", & ParaBrowserManager::GetBrowserWindow1),
					def("clearCache", & ParaBrowserManager::clearCache),
					def("getLastError", & ParaBrowserManager::getLastError),
					def("getVersion", & ParaBrowserManager::getVersion),
					def("GetMaxWindowNum", & ParaBrowserManager::GetMaxWindowNum),
					def("SetMaxWindowNum", & ParaBrowserManager::SetMaxWindowNum),
					def("onPageChanged", & ParaBrowserManager::onPageChanged),
					def("onNavigateBegin", & ParaBrowserManager::onNavigateBegin),
					def("onNavigateComplete", & ParaBrowserManager::onNavigateComplete),
					def("onUpdateProgress", & ParaBrowserManager::onUpdateProgress),
					def("onStatusTextChange", & ParaBrowserManager::onStatusTextChange),
					def("onLocationChange", & ParaBrowserManager::onLocationChange),
					def("onClickLinkHref", & ParaBrowserManager::onClickLinkHref),
					def("reset", & ParaBrowserManager::reset)
				]
			];
#endif
	}


	void CNPLScriptingState::LoadHAPI_Jabber()
	{
		using namespace luabind;
		lua_State* L = GetLuaState();
#ifdef HAS_JABBER_CLIENT
		module(L)
			[
				namespace_("JabberClientManager")
				[
					// class declarations
					class_<NPLJabberClient>("NPLJabberClient")
					.def(constructor<>())
					.property("User",&NPLJabberClient::GetUser,&NPLJabberClient::SetUser)
					.property("Priority",&NPLJabberClient::GetPriority,&NPLJabberClient::SetPriority)
					.property("Password",&NPLJabberClient::GetPassword,&NPLJabberClient::SetPassword)
					.property("AutoLogin",&NPLJabberClient::GetAutoLogin,&NPLJabberClient::SetAutoLogin)
					.property("AutoRoster",&NPLJabberClient::GetAutoRoster,&NPLJabberClient::SetAutoRoster)
					.property("AutoIQErrors",&NPLJabberClient::GetAutoIQErrors,&NPLJabberClient::SetAutoIQErrors)
					.property("AutoPresence",&NPLJabberClient::GetAutoPresence,&NPLJabberClient::SetAutoPresence)
					.property("Resource",&NPLJabberClient::GetResource,&NPLJabberClient::SetResource)
					//.property("IsAuthenticated",&NPLJabberClient::GetIsAuthenticated,&NPLJabberClient::SetIsAuthenticated)
					.property("Server",&NPLJabberClient::GetServer,&NPLJabberClient::SetServer)
					.property("NetworkHost",&NPLJabberClient::GetNetworkHost,&NPLJabberClient::SetNetworkHost)
					.property("Port",&NPLJabberClient::GetPort,&NPLJabberClient::SetPort)
					.property("PlaintextAuth",&NPLJabberClient::GetPlaintextAuth,&NPLJabberClient::SetPlaintextAuth)
					.property("SSL",&NPLJabberClient::GetSSL,&NPLJabberClient::SetSSL)
					.property("AutoStartTLS",&NPLJabberClient::GetAutoStartTLS,&NPLJabberClient::SetAutoStartTLS)
					.property("AutoStartCompression",&NPLJabberClient::GetAutoStartCompression,&NPLJabberClient::SetAutoStartCompression)
					.property("KeepAlive",&NPLJabberClient::GetKeepAlive,&NPLJabberClient::SetKeepAlive)
					.property("AutoReconnect",&NPLJabberClient::GetAutoReconnect,&NPLJabberClient::SetAutoReconnect)
					.property("RequiresSASL",&NPLJabberClient::GetRequiresSASL,&NPLJabberClient::SetRequiresSASL)

					.def("IsSSLon", &NPLJabberClient::IsSSLon)
					.def("IsCompressionOn", &NPLJabberClient::IsCompressionOn)
					.def("GetServerVersion", &NPLJabberClient::GetServerVersion)
					.def("Connect", &NPLJabberClient::Connect)
					.def("Close", &NPLJabberClient::Close)
					.def("activate", &NPLJabberClient::activate)
					.def("WriteNPLMessage", &NPLJabberClient::WriteNPLMessage)
					.def("WriteRawString", &NPLJabberClient::WriteRawString)
					.def("Login", &NPLJabberClient::Login)
					.def("SetPresence", &NPLJabberClient::SetPresence)
					.def("Message", &NPLJabberClient::Message)
					.def("Message", &NPLJabberClient::Message2)
					.def("GetRoster", &NPLJabberClient::GetRoster)
					.def("Subscribe", &NPLJabberClient::Subscribe)
					.def("Unsubscribe", &NPLJabberClient::Unsubscribe)
					.def("RemoveRosterItem", &NPLJabberClient::RemoveRosterItem)
					.def("AllowSubscription", &NPLJabberClient::AllowSubscription)
					.def("GetAgents", &NPLJabberClient::GetAgents)
					.def("Register", &NPLJabberClient::Register)
					.def("GetRosterItemDetail", &NPLJabberClient::GetRosterItemDetail)
					.def("GetRosterGroups", &NPLJabberClient::GetRosterGroups)
					.def("GetRosterItems", &NPLJabberClient::GetRosterItems)
					.def("AddEventListener", &NPLJabberClient::AddEventListener)
					.def("AddEventListener", &NPLJabberClient::AddEventListener1)
					.def("RemoveEventListener", &NPLJabberClient::RemoveEventListener)
					.def("RemoveEventListener", &NPLJabberClient::RemoveEventListener1)
					.def("ClearEventListener", &NPLJabberClient::ClearEventListener)
					.def("ClearEventListener", &NPLJabberClient::ClearEventListener1)
					.def("ResetAllEventListeners", &NPLJabberClient::ResetAllEventListeners)
					.def("GetIsAuthenticated", &NPLJabberClient::GetIsAuthenticated)
					.def("GetStatistics", &NPLJabberClient::GetStatistics)
					.def("RegisterCreateAccount", &NPLJabberClient::RegisterCreateAccount)
					.def("RegisterChangePassword", &NPLJabberClient::RegisterChangePassword)
					.def("RegisterRemoveAccount", &NPLJabberClient::RegisterRemoveAccount)

					.def("IsValid", &NPLJabberClient::IsValid),

					// function declarations
					def("GetJabberClient", & JabberClientManager::GetJabberClient),
					def("CreateJabberClient", & JabberClientManager::CreateJabberClient),
					def("AddStringMap", & JabberClientManager::AddStringMap),
					def("ClearStringMap", & JabberClientManager::ClearStringMap),
					def("CloseJabberClient", & JabberClientManager::CloseJabberClient)
				]
			];
#endif
	}

	void CNPLScriptingState::LoadParaWorld()
	{

		using namespace luabind;
		lua_State* L = GetLuaState();

		module(L)
			[
				namespace_("ParaBlockWorld")
				[
					// this is static factory class
					def("GetWorld", &ParaBlockWorld::GetWorld),
					// ParaBlockWorld class declarations
					def("GetVersion", &ParaBlockWorld::GetVersion),
					def("EnterWorld", &ParaBlockWorld::EnterWorld),
					def("LeaveWorld", &ParaBlockWorld::LeaveWorld),
					def("GetBlockAttributeObject", &ParaBlockWorld::GetBlockAttributeObject),
					def("SaveBlockWorld", &ParaBlockWorld::SaveBlockWorld),
					def("LoadRegion", &ParaBlockWorld::LoadRegion),
					def("UnloadRegion", &ParaBlockWorld::UnloadRegion),
					def("RegisterBlockTemplate", &ParaBlockWorld::RegisterBlockTemplate),
					def("SetBlockWorldYOffset", &ParaBlockWorld::SetBlockWorldYOffset),
					def("SetBlockId", &ParaBlockWorld::SetBlockId),
					def("GetBlockId", &ParaBlockWorld::GetBlockId),
					def("SetBlockData", &ParaBlockWorld::SetBlockData),
					def("GetBlockData", &ParaBlockWorld::GetBlockData),
					def("GetBlocksInRegion", &ParaBlockWorld::GetBlocksInRegion),
					def("SetBlockWorldSunIntensity", &ParaBlockWorld::SetBlockWorldSunIntensity),
					def("FindFirstBlock", &ParaBlockWorld::FindFirstBlock),
					def("GetFirstBlock", &ParaBlockWorld::GetFirstBlock),
					def("SetTemplateTexture", &ParaBlockWorld::SetTemplateTexture),
					def("SetTemplatePhysicsProperty", &ParaBlockWorld::SetTemplatePhysicsProperty),
					def("GetTemplatePhysicsProperty", &ParaBlockWorld::GetTemplatePhysicsProperty),
					// client only functions
					def("GetVisibleChunkRegion", &ParaBlockWorld::GetVisibleChunkRegion),
					def("Pick", &ParaBlockWorld::Pick),
					def("MousePick", &ParaBlockWorld::MousePick),
					def("SelectBlock", &ParaBlockWorld::SelectBlock),
					def("SelectBlock1", &ParaBlockWorld::SelectBlock1),
					def("DeselectAllBlock1", &ParaBlockWorld::DeselectAllBlock1),
					def("DeselectAllBlock", &ParaBlockWorld::DeselectAllBlock),
					def("SetDamagedBlock", &ParaBlockWorld::SetDamagedBlock),
					def("SetDamageDegree", &ParaBlockWorld::SetDamageDegree),
					def("GetBlockModelInfo", &ParaBlockWorld::GetBlockModelInfo) 
				]
			];

		module(L)
			[
				namespace_("ParaWorld")
				[
					// ParaMovieCtrler class declarations
					class_<ParaDataProvider>("ParaDataProvider")
					.def(constructor<>())
					.def("GetNPCCount", &ParaDataProvider::GetNPCCount)
					.def("GetNPCIDByName", &ParaDataProvider::GetNPCIDByName)
					.def("DoesAttributeExists", &ParaDataProvider::DoesAttributeExists)
					.def("GetAttribute", &ParaDataProvider::GetAttribute)
					.def("UpdateAttribute", &ParaDataProvider::UpdateAttribute)
					.def("InsertAttribute", &ParaDataProvider::InsertAttribute)
					.def("DeleteAttribute", &ParaDataProvider::DeleteAttribute)
					.def("ExecSQL", &ParaDataProvider::ExecSQL)
					.def("SetTableName", &ParaDataProvider::SetTableName)
					.def("GetTableName", &ParaDataProvider::GetTableName)
					.def("IsValid", &ParaDataProvider::IsValid),
					// function declarations
					def("NewWorld", &ParaWorld::NewWorld),
					def("NewEmptyWorld", &ParaWorld::NewEmptyWorld),
					def("DeleteWorld", &ParaWorld::DeleteWorld),
					def("SetAttributeProvider", &ParaWorld::SetAttributeProvider),
					def("GetAttributeProvider", &ParaWorld::GetAttributeProvider),
					def("SetWorldDB", &ParaWorld::SetWorldDB),
					def("SetNpcDB", &ParaWorld::SetNpcDB),
					def("GetStringbyID", &ParaWorld::GetStringbyID),
					def("GetServerState", &ParaWorld::GetServerState),
					def("SetServerState", &ParaWorld::SetServerState),
					def("SendTerrainUpdate", &ParaWorld::SendTerrainUpdate),
					def("SetEnablePathEncoding", &ParaWorld::SetEnablePathEncoding),
					def("GetEnablePathEncoding", &ParaWorld::GetEnablePathEncoding),
					def("GetWorldName", &ParaWorld::GetWorldName),
					def("GetWorldDirectory", &ParaWorld::GetWorldDirectory),
					def("GetScriptSandBox", &ParaWorld::GetScriptSandBox),
					def("SetScriptSandBox", &ParaWorld::SetScriptSandBox),
					def("InsertString", &ParaWorld::InsertString)
				]
			];

		module(L)
			[
				namespace_("ParaSelection")
				[
					// function declarations
					def("RemoveObject", &ParaSelection::RemoveObject),
					def("AddObject", &ParaSelection::AddObject),
					def("GetObject", &ParaSelection::GetObject),
					def("GetItemNumInGroup", &ParaSelection::GetItemNumInGroup),
					def("SelectGroup", &ParaSelection::SelectGroup),
					def("ClearGroup", &ParaSelection::ClearGroup),
					def("AddUIObject", &ParaSelection::AddUIObject),
					def("RemoveUIObject", &ParaSelection::RemoveUIObject),
					def("GetMaxItemNumberInGroup", &ParaSelection::GetMaxItemNumberInGroup),
					def("SetMaxItemNumberInGroup", &ParaSelection::SetMaxItemNumberInGroup)
				]
			];
	}

}//namespace ParaScripting