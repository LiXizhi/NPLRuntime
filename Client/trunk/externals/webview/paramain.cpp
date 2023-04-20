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


#include "INPLRuntimeState.h"
#include "IParaEngineCore.h"
#include "IParaEngineApp.h"
#include "IAttributeFields.h"
#include "NPLInterface.hpp"

#include "json.hpp"
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

static NPL::INPLRuntimeState* g_pStaticState = nullptr;
static WebView* g_webview = WebView::GetInstance();
static int g_support_webview = 0; // 0 -- 未检测  1 -- 支持   2 -- 不支持

static std::function<void(const std::wstring&)> g_webview_on_msg_callback = [](const std::wstring& msg_json_str)
{
	std::string msg = WStringToString(msg_json_str);
	auto msg_json = nlohmann::json::parse(msg, nullptr, false);
	if (msg_json.is_discarded())
	{
		std::cout << "the input is invalid JSON!!!" << std::endl;
		return;
	}
	std::string filename = msg_json["filename"];
	if (g_pStaticState == nullptr) return;
	NPLInterface::NPLObjectProxy data;
	data["msg"] = msg;
	NPL_Activate(g_pStaticState, filename, data);
	// g_pStaticState->activate(filename.c_str(), data);
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

nlohmann::json ToJson(WebViewParams p)
{
	nlohmann::json out;
	out["cmd"] = p.cmd;
	out["callback_file"] = p.callback_file;
	out["cmdline"] = p.cmdline;
	out["client_name"] = p.client_name;
	out["parent_handle"] = p.parent_handle;
	out["cefclient_config_filename"] = p.cefclient_config_filename;
	out["pid"] = p.pid;
	out["id"] = p.id;
	out["url"] = p.url;
	out["x"] = p.x;
	out["y"] = p.y;
	out["width"] = p.width;
	out["height"] = p.height;
	out["visible"] = p.visible;
	out["resize"] = p.resize;
	out["enabled"] = p.enabled;
	out["zoom"] = p.zoom;
	out["message_content"] = p.message_content;
	return out;
}

nlohmann::json Read(NPLInterface::NPLObjectProxy tabMsg)
{
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
	double x = tabMsg["x"];
	double y = tabMsg["y"];
	double width = tabMsg["width"];
	double height = tabMsg["height"];
	double zoom = tabMsg["zoom"];
	params.x = (int)x;
	params.y = (int)y;
	params.width = (int)width;
	params.height = (int)height;
	params.zoom = zoom;
	params.message_content = tabMsg["message_content"];

	return ToJson(params);
}



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
	// TODO: dll start up code here
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


// ParaWebViewd.dll
CORE_EXPORT_DECL void LibActivate(int nType, void* pVoid)
{
	static std::string s_id = "";
	if (g_support_webview == 2)
		return;
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
			NPL_Activate(pState, callback_file, msg);

			if (!ok && auto_install)
			{
				std::thread(WebView::DownloadAndInstallWV2RT, [pState, callback_file]() {
					// 如果安装成功就可以使用, 可以再次发送支持webview 保险做法当前使用cef3 下次启动使用webview
					// NPLInterface::NPLObjectProxy msg;
					// msg["cmd"] = "Install";
					// msg["ok"] = g_webview->IsSupportWebView();
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
			// g_webview->OnCreated([pState, callback_file, auto_install]() {
			// 	NPLInterface::NPLObjectProxy msg;
			// 	bool ok = g_webview->IsSupportWebView();
			// 	msg["cmd"] = "Support";
			// 	msg["ok"] = ok;
			// 	NPL_Activate(pState, callback_file, msg);

			// 	if (!ok && auto_install) 
			// 	{
			// 		std::thread(WebView::DownloadAndInstallWV2RT, [pState, callback_file](){
			// 			// 如果安装成功就可以使用, 可以再次发送支持webview 保险做法当前使用cef3 下次启动使用webview
			// 			// NPLInterface::NPLObjectProxy msg;
			// 			// msg["cmd"] = "Install";
			// 			// msg["ok"] = g_webview->IsSupportWebView();
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
			// g_webview->Create(g_hInstance, parent_handle);	
			return;
		}

		double scale = params.zoom < 0.0001 ? 1.0 : params.zoom;
		int x = params.x * scale;
		int y = params.y * scale;
		int width = params.width * scale;
		int height = params.height * scale;
		if (cmd == "Start")
		{
			s_id = id;
			if (!params.url.empty())
				g_webview->Open(StringToWString(params.url));

			g_webview->SetOnCreateCallback([x, y, width, height, pState, callback_file]() {
				if (g_webview->IsSupportWebView())
				{
					g_webview->SendSetPositionMessage(x, y, width, height);
				}
				else
				{
					g_webview->Destroy();
				}

				if (g_support_webview == 0)
				{
					g_support_webview = g_webview->IsSupportWebView() ? 1 : 2;

					NPLInterface::NPLObjectProxy msg;
					msg["cmd"] = "SupportWebView";
					msg["ok"] = g_webview->IsSupportWebView();
					NPL_Activate(pState, callback_file, msg);
				}
			});
			g_webview->OnWebMessage(g_webview_on_msg_callback);            // window webview 模式
			g_webview->OnProtoSendMsgCallBack(g_webview_on_msg_callback);  // cef 模式 
			g_webview->Create(g_hInstance, parent_handle);
		}
		else
		{
			// if (s_id != id) return;

			if (cmd == "Quit")
			{
				g_webview->SendHide();
			}

			if (cmd == "CallJsFunc")
			{
				g_webview->AsyncSendWebMessage(StringToWString(tabMsg["message_content"]));
			}

			if (cmd == "Show")
			{
				if (params.visible)
				{
					g_webview->SendShow();
				}
				else
				{
					g_webview->SendHide();
				}
			}

			if (cmd == "ChangePosSize")
			{
				g_webview->SendSetPositionMessage(x, y, width, height);
			}

			if (cmd == "Open")
			{
				g_webview->SendOpenMessage(StringToWString(params.url));
				g_webview->SendSetPositionMessage(x, y, width, height);
			}
		}
	}
}



