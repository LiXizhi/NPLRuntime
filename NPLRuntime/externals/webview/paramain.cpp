//-----------------------------------------------------------------------------
// Class:	para webview main
// Authors:	wxa, LiXizhi
// Date:	2023.4
//-----------------------------------------------------------------------------
#include "PluginAPI.h"

#include <memory>
#include <algorithm>
#include <iostream>
#include <windows.h>
#include <wchar.h>
#include <string.h>
#include <thread>
#include <sstream>
#include <fstream>
#include <ShellAPI.h>
#include <map>
#include <codecvt>
#include <locale>
#include <string>
#include <mutex>
#include <unordered_map>
#include "INPLRuntimeState.h"
#include "IParaEngineCore.h"
#include "IParaEngineApp.h"
#include "IAttributeFields.h"
#include "NPLInterface.hpp"

#include "json/json.h"
#include "webview.h"
using namespace ParaEngine;

std::string WStringToString(std::wstring wstr)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(wstr);
}

std::wstring StringToWString(std::string str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.from_bytes(str);
}

static void NPL_Activate(NPL::INPLRuntimeState* pState, std::string activateFile, NPLInterface::NPLObjectProxy data)
{
	if (!pState || activateFile.empty()) return;
	std::string data_string;
	NPLInterface::NPLHelper::NPLTableToString("msg", data, data_string);
	pState->activate(activateFile.c_str(), data_string.c_str(), (int)data_string.length());
}
static void NPL_Call(NPL::INPLRuntimeState* pState, std::string activateFile, NPLInterface::NPLObjectProxy data)
{
	if (!pState || activateFile.empty()) return;
	std::string data_string;
	NPLInterface::NPLHelper::NPLTableToString("msg", data, data_string);
	pState->call(activateFile.c_str(), data_string.c_str(), (int)data_string.length());
}

static NPL::INPLRuntimeState* g_pStaticState = nullptr;
static std::mutex g_mutex;
static std::unordered_map<std::string, std::shared_ptr<WebView>> g_webviews;

static WebView* GetWebViewByID(const std::string& id, bool bCreateIfNotExist)
{
	std::lock_guard<std::mutex> _lock(g_mutex);
	auto it = g_webviews.find(id);
	if (it != g_webviews.end()) return it->second.get();
	if (!bCreateIfNotExist) return nullptr;
	auto webview = std::make_shared<WebView>(id);
	g_webviews.insert_or_assign(id, webview);
	return webview.get();
}

static void DeleteWebViewByID(const std::string& id)
{
	std::lock_guard<std::mutex> _lock(g_mutex);
	g_webviews.erase(id);
}

static std::function<void(const std::wstring&)> g_webview_on_msg_callback = [](const std::wstring& msg_json_wstr)
{
	std::string msg_json_str = WStringToString(msg_json_wstr);

	Json::Value msg_json;
	Json::Reader reader(Json::Features().strictMode());
	bool parsingSuccessful = reader.parse(msg_json_str.c_str(), msg_json_str.c_str() + msg_json_str.size(), msg_json, false);
	if (!parsingSuccessful || !msg_json.isObject() || !msg_json.isMember("filename"))
	{
		return;
	}
	Json::Value filename_json = msg_json["filename"];
	if (!filename_json.isString())
	{
		return;
	}
	std::string filename = filename_json.asString();
	if (g_pStaticState == nullptr) return;
	NPLInterface::NPLObjectProxy data;
	data["msg"] = msg_json_str;
	NPL_Activate(g_pStaticState, filename, data);
};

struct WebViewParams
{
	std::string cmd;
	std::string callback_file;
	std::string cmdline;
	std::string client_name;
	std::string parent_handle;
	std::string cefclient_config_filename;
	std::string pid;
	std::string id;
	std::string url;
	int x = 0;
	int y = 0;
	int width = 800;
	int height = 600;
	bool visible = true;
	bool resize = true;
	bool enabled = true;
	bool transparent = false;
	double zoom = 0.0;
	std::string message_content;
	std::string message_to;
};

#pragma region PE_DLL

#ifdef WIN32
#define CORE_EXPORT_DECL    __declspec(dllexport)
#else
#define CORE_EXPORT_DECL
#endif

#ifdef __cplusplus
extern "C" {
#endif
	CORE_EXPORT_DECL const char* LibDescription();
	CORE_EXPORT_DECL int LibNumberClasses();
	CORE_EXPORT_DECL unsigned long LibVersion();
	CORE_EXPORT_DECL ParaEngine::ClassDescriptor* LibClassDesc(int i);
	CORE_EXPORT_DECL void LibInit();
	CORE_EXPORT_DECL void LibActivate(int nType, void* pVoid);
	CORE_EXPORT_DECL void LibInitParaEngine(ParaEngine::IParaEngineCore* pCoreInterface);
#ifdef __cplusplus
}
#endif

HINSTANCE g_hInstance = NULL;

ClassDescriptor* ParaWebView_GetClassDesc();
typedef ClassDescriptor* (*GetClassDescMethod)();

GetClassDescMethod Plugins[] =
{
	ParaWebView_GetClassDesc,
};

#define ParaWebView_CLASS_ID Class_ID(0x3b305a29, 0x47a409ce)

class ParaWebViewDesc : public ClassDescriptor
{
public:
	void* Create(bool)
	{
		return NULL;
	}
	const char* ClassName()
	{
		return "ParaWebView";
	}
	SClass_ID SuperClassID()
	{
		return OBJECT_MODIFIER_CLASS_ID;
	}
	Class_ID ClassID()
	{
		return ParaWebView_CLASS_ID;
	}
	const char* Category()
	{
		return "ParaWebView";
	}
	const char* InternalName()
	{
		return "ParaWebView";
	}
	HINSTANCE HInstance()
	{
		return g_hInstance;
	}
};

ClassDescriptor* ParaWebView_GetClassDesc()
{
	static ParaWebViewDesc s_desc;
	return &s_desc;
}

CORE_EXPORT_DECL const char* LibDescription()
{
	return "ParaEngine WebView Version 1.0.0";
}

CORE_EXPORT_DECL unsigned long LibVersion()
{
	return 1;
}

CORE_EXPORT_DECL int LibNumberClasses()
{
	return sizeof(Plugins) / sizeof(Plugins[0]);
}

CORE_EXPORT_DECL ClassDescriptor* LibClassDesc(int i)
{
	if (i < LibNumberClasses() && Plugins[i])
	{
		return Plugins[i]();
	}
	return NULL;
}

ParaEngine::IParaEngineCore* g_pCoreInterface = NULL;
ParaEngine::IParaEngineCore* GetCoreInterface()
{
	return g_pCoreInterface;
}

CORE_EXPORT_DECL void LibInitParaEngine(IParaEngineCore* pCoreInterface)
{
	g_pCoreInterface = pCoreInterface;
}

CORE_EXPORT_DECL void LibInit()
{
}

#ifdef WIN32
BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG, LPVOID)
{
	g_hInstance = hinstDLL;
	return TRUE;
}
#endif
#pragma endregion PE_DLL

void WriteLog(const char* sFormat, ...)
{
	if (GetCoreInterface() && GetCoreInterface()->GetAppInterface())
	{
		char buf_[1024 + 1] = { 0 };
		va_list args;
		va_start(args, sFormat);
		vsnprintf(buf_, 1024, sFormat, args);
		GetCoreInterface()->GetAppInterface()->WriteToLog(buf_);
		va_end(args);
	}
}

CORE_EXPORT_DECL void LibActivate(int nType, void* pVoid)
{
	if (nType == ParaEngine::PluginActType_STATE)
	{
		NPL::INPLRuntimeState* pState = (NPL::INPLRuntimeState*)pVoid;
		if (g_pStaticState == nullptr || g_pStaticState != pState) g_pStaticState = pState;

		const char* sMsg = pState->GetCurrentMsg();
		NPLInterface::NPLObjectProxy tabMsg = NPLInterface::NPLHelper::MsgStringToNPLTable(sMsg);

		WebViewParams params;
		params.cmd = tabMsg["cmd"];
		params.callback_file = tabMsg["callback_file"];
		params.cmdline = tabMsg["cmdline"];
		params.client_name = tabMsg["client_name"];
		params.parent_handle = tabMsg["parent_handle"];
		params.cefclient_config_filename = tabMsg["cefclient_config_filename"];
		params.pid = tabMsg["pid"];
		params.id = tabMsg["id"];
		params.url = tabMsg["url"];
		params.resize = tabMsg["resize"];
		params.visible = tabMsg["visible"];
		params.enabled = tabMsg["enabled"];
		params.transparent = tabMsg["transparent"];
		params.x = (int)(double)tabMsg["x"];
		params.y = (int)(double)tabMsg["y"];
		params.width = (int)(double)tabMsg["width"];
		params.height = (int)(double)tabMsg["height"];
		params.zoom = (double)tabMsg["zoom"];
		params.message_content = tabMsg["message_content"];

		std::string id = params.id;
		std::string cmd = params.cmd;
		std::string callback_file = params.callback_file;
		std::string parent_handle_s = params.parent_handle;
		HWND parent_handle = NULL;
		if (!parent_handle_s.empty())
		{
			try
			{
				size_t parsedIdx = 0;
				unsigned long long hVal = std::stoull(parent_handle_s, &parsedIdx, 0);
				if (parsedIdx == parent_handle_s.size() && hVal != 0)
				{
					parent_handle = (HWND)(uintptr_t)hVal;
				}
				else
				{
					double dVal = std::stod(parent_handle_s);
					if (dVal > 0)
						parent_handle = (HWND)(uintptr_t)dVal;
				}
			}
			catch (...)
			{
				parent_handle = NULL;
			}
		}
		if ((parent_handle == NULL || !IsWindow(parent_handle)) && GetCoreInterface() != nullptr)
		{
			HWND coreHwnd = GetCoreInterface()->GetParaEngineHWnd();
			if (coreHwnd != NULL && IsWindow(coreHwnd))
				parent_handle = coreHwnd;
		}

		if (cmd == "Support")
		{
			NPLInterface::NPLObjectProxy msg;
			bool auto_install = tabMsg["auto_install"];
			bool ok = WebView::IsSupported();
			msg["cmd"] = "Support";
			msg["ok"] = ok;
			NPL_Call(pState, callback_file, msg);

			if (!ok && auto_install)
			{
				std::thread(WebView::DownloadAndInstallWV2RT, []() {}, []() {}).detach();
			}
			return;
		}

		double scale = params.zoom < 0.0001 ? 1.0 : params.zoom;
		int x = (int)(params.x * scale);
		int y = (int)(params.y * scale);
		int width = (int)(params.width * scale);
		int height = (int)(params.height * scale);
		if (cmd == "Start")
		{
			auto webview = GetWebViewByID(id, true);
			if (params.transparent)
				webview->SetTransparentFlag(true);
			if (!params.url.empty())
				webview->SendOpenMessage(StringToWString(params.url));

			webview->SendSetPositionMessage(x, y, width, height);
			webview->SendShow();
			webview->SendDebug(tabMsg["debug"]);
			webview->SetOnCreateCallback([pState, callback_file, webview]() {
				if (!webview->IsSupportWebView())
				{
					webview->Destroy();
				}
				NPLInterface::NPLObjectProxy msg;
				msg["cmd"] = "WebViewStarted";
				msg["ok"] = webview->IsSupportWebView();
				msg["id"] = webview->GetID();
				NPL_Activate(pState, callback_file, msg);
			});
			webview->OnWebMessage(g_webview_on_msg_callback);
			webview->OnProtoSendMsgCallBack(g_webview_on_msg_callback);
			webview->Create(g_hInstance, parent_handle);
		}
		else
		{
			auto webview = GetWebViewByID(id, false);
			if (webview == nullptr)
				return;

			if (cmd == "Quit")
			{
				webview->SendHide();
				DeleteWebViewByID(id);
			}
			else if (cmd == "CallJsFunc")
			{
				webview->AsyncSendWebMessage(StringToWString(tabMsg["message_content"]));
			}
			else if (cmd == "Show")
			{
				if (params.visible)
				{
					webview->SendShow();
					if (width > 0)
						webview->SendSetPositionMessage(x, y, width, height);
				}
				else
				{
					webview->SendHide();
				}
			}
			else if (cmd == "ChangePosSize")
			{
				webview->SendSetPositionMessage(x, y, width, height);
			}
			else if (cmd == "Open")
			{
				webview->SendOpenMessage(StringToWString(params.url));
				if (width > 0 && height > 0)
					webview->SendSetPositionMessage(x, y, width, height);
				if (tabMsg["visible"].GetType() == NPLInterface::NPLObjectBase::NPLObjectType_Bool)
				{
					if (params.visible)
						webview->SendShow();
					else
						webview->SendHide();
				}
			}
		}
	}
}
