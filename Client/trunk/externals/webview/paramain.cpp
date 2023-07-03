//-----------------------------------------------------------------------------
// Class:	para webview main
// Authors:	wxa, LiXizhi
// Date: 2023.4
// Desc: 
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

// #include "json.hpp"
#include "json/json.h"

#include "webview.h"
using namespace ParaEngine;

std::string WStringToString(std::wstring wstr)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::string str = converter.to_bytes(wstr);
	return str;
}

std::wstring StringToWString(std::string str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::wstring wstr = converter.from_bytes(str);
	return wstr;
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
static WebView* g_webview = WebView::GetInstance();
// static int g_support_webview = 0; // 0 -- 未检测  1 -- 支持   2 -- 不支持
static std::mutex g_mutex;
static std::unordered_map<std::string, std::shared_ptr<WebView>> g_webviews;

static WebView* GetWebViewByID(const std::string& id, bool bCreateIfNotExist)
{
	// return g_webview;
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

	Json::Value msg_json;   // will contains the root value after parsing.
	// strict mode: no comments are allowed, root must be array or object, and string must be in utf8
	Json::Reader reader(Json::Features().strictMode());
	bool parsingSuccessful = reader.parse(msg_json_str.c_str(), msg_json_str.c_str() + msg_json_str.size(), msg_json, false);
	if (!parsingSuccessful || !msg_json.isObject() || !msg_json.isMember("filename"))
	{
		// report to the user the failure and their locations in the document.
		std::cout << "invalid json data: " << msg_json_str << std::endl;
		return;
	}
	Json::Value filename_json = msg_json["filename"];
	if (!filename_json.isString()) 
	{
		std::cout << "filename not exist!!!" << std::endl;
		return;
	}
	std::string filename = filename_json.asString();
	std::string msg = msg_json_str;
	if (g_pStaticState == nullptr) return;
	NPLInterface::NPLObjectProxy data;
	data["msg"] = msg;
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
	std::string pid; //process id
	std::string id;
	std::string url;
	int x = 0;
	int y = 0;
	int width = 800;
	int height = 600;
	bool visible = true;
	bool resize = true;
	bool enabled = true;
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

// forware declare of exported functions. 
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
}   /* extern "C" */
#endif

HINSTANCE g_hInstance = NULL;

ClassDescriptor* ParaWebView_GetClassDesc();
typedef ClassDescriptor* (*GetClassDescMethod)();

GetClassDescMethod Plugins[] =
{
	ParaWebView_GetClassDesc,
};

#define ParaWebView_CLASS_ID Class_ID(0x3b305a29, 0x47a409ce)

class ParaWebViewDesc :public ClassDescriptor
{
public:

	void* Create(bool loading)
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
		static ParaWebViewDesc s_para_webview_desc;
		unsigned long long s_class_id = (unsigned long long)(&s_para_webview_desc);
		return Class_ID(s_class_id & 0xffffffff, s_class_id >> 32);
	}

	const char* Category()
	{
		return "ParaWebView Category";
	}

	const char* InternalName()
	{
		return "ParaWebView InternalName";
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
	else
	{
		return NULL;
	}
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
BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved)
#else
void __attribute__((constructor)) DllMain()
#endif
{
#ifdef WIN32
	g_hInstance = hinstDLL;				// Hang on to this DLL's instance handle.
	return (TRUE);
#endif
}
#pragma endregion PE_DLL 


void WriteLog(const char* sFormat, ...) {
	if (GetCoreInterface() && GetCoreInterface()->GetAppInterface()) {
		char buf_[1024 + 1];
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
		int nMsgLength = pState->GetCurrentMsgLength();
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
		HWND parent_handle = parent_handle_s.empty() ? NULL : (HWND)std::stoull(parent_handle_s.c_str());

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
				std::thread(WebView::DownloadAndInstallWV2RT, [pState, callback_file]() {
					// 如果安装成功就可以使用, 可以再次发送支持webview 保险做法当前使用cef3 下次启动使用webview
					// NPLInterface::NPLObjectProxy msg;
					// msg["cmd"] = "Install";
					// msg["ok"] = webview->IsSupportWebView();
					// NPL_Activate(pState, callback_file, msg);
				}, [pState, callback_file]() {
					// 如果安装失败通知
					// NPLInterface::NPLObjectProxy msg;
					// msg["cmd"] = "Install";
					// msg["ok"] = false;
					// NPL_Activate(pState, callback_file, msg);
				}).detach();
			}

			// bool auto_install = tabMsg["auto_install"];
			// webview->OnCreated([pState, callback_file, auto_install]() {
			// 	NPLInterface::NPLObjectProxy msg;
			// 	bool ok = webview->IsSupportWebView();
			// 	msg["cmd"] = "Support";
			// 	msg["ok"] = ok;
			// 	NPL_Activate(pState, callback_file, msg);

			// 	if (!ok && auto_install) 
			// 	{
			// 		std::thread(WebView::DownloadAndInstallWV2RT, [pState, callback_file](){
			// 			// 如果安装成功就可以使用, 可以再次发送支持webview 保险做法当前使用cef3 下次启动使用webview
			// 			// NPLInterface::NPLObjectProxy msg;
			// 			// msg["cmd"] = "Install";
			// 			// msg["ok"] = webview->IsSupportWebView();
			// 			// NPL_Activate(pState, callback_file, msg);
			// 		}, [pState, callback_file](){
			// 			// 如果安装失败通知
			// 			// NPLInterface::NPLObjectProxy msg;
			// 			// msg["cmd"] = "Install";
			// 			// msg["ok"] = false;
			// 			// NPL_Activate(pState, callback_file, msg);
			// 		}).detach();
			// 	}
			// });
			// webview->Create(g_hInstance, parent_handle);	
			return;
		}

		double scale = params.zoom < 0.0001 ? 1.0 : params.zoom;
		int x = params.x * scale;
		int y = params.y * scale;
		int width = params.width * scale;
		int height = params.height * scale;
		if (cmd == "Start")
		{
			auto webview = GetWebViewByID(id, true);
			if (!params.url.empty())
				webview->SendOpenMessage(StringToWString(params.url));
			
			webview->SendSetPositionMessage(x, y, width, height);
			webview->SendShow();
			webview->SendDebug(tabMsg["debug"]);
			webview->SetOnCreateCallback([x, y, width, height, pState, callback_file, webview]() {
				if (!webview->IsSupportWebView())
				{
					webview->Destroy();
				}
				// send callback to NPL runtime
				NPLInterface::NPLObjectProxy msg;
				msg["cmd"] = "WebViewStarted";
				msg["ok"] = webview->IsSupportWebView();
				msg["id"] = webview->GetID();
				NPL_Activate(pState, callback_file, msg);
			});
			webview->OnWebMessage(g_webview_on_msg_callback); // for window webview2
			webview->OnProtoSendMsgCallBack(g_webview_on_msg_callback);  // for cef3
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
					if(width > 0)
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
				webview->SendSetPositionMessage(x, y, width, height);
			}
		}
	}
}



