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

static std::string WStringToString(std::wstring wstr)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::string str = converter.to_bytes(wstr);
	return str;
}

static std::wstring StringToWString(std::string str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::wstring wstr = converter.from_bytes(str);
	return wstr;
}

static void NPL_Activate(NPL::INPLRuntimeState* pState, std::string activateFile, NPLInterface::NPLObjectProxy& data) 
{
    if (!pState || activateFile.empty()) return;
    std::string data_string;
    NPLInterface::NPLHelper::NPLTableToString("msg", data, data_string);
    pState->activate(activateFile.c_str(), data_string.c_str(), (int)data_string.length());
}

static NPL::INPLRuntimeState* g_pStaticState = nullptr;
static WebView* g_webview = WebView::GetInstance();
static std::function<void()> g_webview_on_created_callback = []() {
	g_webview->ExecuteScript(LR"(
window.chrome.webview.addEventListener("message", function(event){
	var data = JSON.parse(event.data);
	if (typeof data != "object") 
	{
		console.log("无效数据:", event.data);
		return;
	}
	if (typeof window.NPL == "object" && typeof window.NPL.receive == "function")
	{
		window.NPL.receive(data.file, data.params);
	}
});
	)");
};

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
	g_pStaticState->activate(filename.c_str(), msg.c_str(), (int)msg.length());
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
	if (!WebView::IsSupported())
	{
		std::thread(WebView::DownloadAndInstallWV2RT, nullptr, nullptr).detach();
	}
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

extern "C" 
{
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
}

// ParaWebViewd.dll
CORE_EXPORT_DECL void LibActivate(int nType, void* pVoid)
{
	WriteLog("=====================ParaWebView==========================");
	static std::string s_id = "";
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

		if (cmd == "Support")
		{
			tabMsg["ok"] = WebView::IsSupported();
			NPL_Activate(pState, callback_file, tabMsg);
			return;
		}

        HWND parent_handle = parent_handle_s.empty() ? NULL : (HWND)std::stoull(parent_handle_s.c_str());

		int parent_window_x = 0;
		int parent_window_y = 0;
		if (parent_handle)
		{
			RECT rect;
			GetClientRect(parent_handle, &rect);
			ClientToScreen(parent_handle, (LPPOINT)&rect.left);
			ClientToScreen(parent_handle, (LPPOINT)&rect.right);
			parent_window_x = rect.left;
			parent_window_y = rect.top;
		}
		
		double scale = params.zoom < 0.0001 ? 1.0 : params.zoom;
		int x = parent_window_x + params.x * scale;
		int y = parent_window_y + params.y * scale;
		int width = params.width * scale;
		int height = params.height * scale;
		if (cmd == "Start") 
		{
			s_id = id;
			g_webview->OnCreated(g_webview_on_created_callback);
			g_webview->OnWebMessage(g_webview_on_msg_callback);            // window webview 模式
			g_webview->OnProtoSendMsgCallBack(g_webview_on_msg_callback);  // cef 模式 
			g_webview->Create(g_hInstance, parent_handle);	
			g_webview->SetPosition(x, y, width, height);
			g_webview->Open(StringToWString(params.url));
			g_webview->Show();
		} 
		else 
		{
			if (s_id != id) return;

            if (cmd == "Quit")
            {
				g_webview->Hide();
            }
			
			if (cmd == "CallJsFunc")
			{
				g_webview->SendWebMessage(StringToWString(tabMsg["message_content"]));
			}

			if (cmd == "Show")
			{
				if (params.visible)
				{
					g_webview->Show();
					g_webview->SetPosition(x, y, width, height);
				}
				else
				{
					g_webview->Hide();
				}
			}

			if (cmd == "ChangePosSize") 
			{
				g_webview->SetPosition(x, y, width, height);
			}

			if (cmd == "Open")
			{
				g_webview->Open(StringToWString(params.url));
            	g_webview->SetPosition(x, y, width, height);
			}
		}
	}
}



