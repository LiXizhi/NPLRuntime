//-----------------------------------------------------------------------------
// Class:	para webview main
// Authors:	wxa, LiXizhi
// Date: 2023.4
// Desc: 
//-----------------------------------------------------------------------------
#include <Windows.h>
#include <VersionHelpers.h>
#include "WebView.h"

extern void WriteLog(const char* sFormat, ...);
extern std::string WStringToString(std::wstring wstr);

static std::string ReadRegStr(HKEY root_key, std::string sub_key, std::string name)
{
	HKEY hKey;
	DWORD dwSize = 0;
	std::string value;
	if (RegOpenKeyEx(root_key, sub_key.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS)
	{
		if (ERROR_SUCCESS == RegQueryValueEx(hKey, name.c_str(), NULL, NULL, NULL, &dwSize))
		{
			value.resize(dwSize - 1);
			if (RegGetValue(hKey, NULL, name.c_str(), RRF_RT_REG_SZ, NULL, (void*)value.data(), &dwSize) != ERROR_SUCCESS) value = "";
		}
		RegCloseKey(hKey);
	}
	return value;
}

WebView::WebView(const std::string& id)
{
	m_id = id;
	m_hWnd = NULL;
	m_bShow = true;
    m_bDebug = false;
	m_x = 0;
	m_y = 0;
	m_width = 0;
	m_height = 0;

	m_webview = nullptr;
	m_webview_controller = nullptr;
	m_on_created_callback = nullptr;
	m_on_message_callback = nullptr;
	m_on_proto_send_msg_callback = nullptr;
	m_nWndState = WEBVIEW_STATE_UNINITIALIZED;
}

WebView::~WebView()
{
	Destroy();
}

bool WebView::SetWnd(HWND hWnd)
{
	if (m_hWnd == hWnd) return true;
	if (m_webview_controller != nullptr)
	{
		m_webview_controller->Close();
		m_webview_controller = nullptr;
		m_webview = nullptr;
	}
	m_hWnd = hWnd;
	return CreateWebView(m_hWnd);
}

LRESULT CALLBACK WebViewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WebViewMessage* msg = nullptr;
	switch (message)
	{
	case WM_WEBVIEW_MESSAGE:
		msg = (WebViewMessage*)lParam;
		if (msg->m_cmd == "Open")
		{
			msg->m_webview->Open(msg->m_url);
		}
		else if (msg->m_cmd == "Show")
		{
			if (msg->m_visible)
				msg->m_webview->Show();
			else
				msg->m_webview->Hide();
		}
		else if (msg->m_cmd == "SetPosition")
		{
			msg->m_webview->SetPosition(msg->m_x, msg->m_y, msg->m_width, msg->m_height);
		}
		else if (msg->m_cmd == "SendWebMessage")
		{
			msg->m_webview->PostWebMessage(msg->m_msg);
		}
        else if (msg->m_cmd == "Debug")
        {
            msg->m_webview->Debug(msg->m_debug);
        }
		else if (msg->m_cmd == "Quit")
		{
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}

bool WebView::Create(HINSTANCE hInstance, HWND hParentWnd)
{
	if (!IsSupported()) {
		if (m_on_created_callback != nullptr)
			m_on_created_callback();
		return false;
	}

	if (m_hWnd || m_nWndState != WEBVIEW_STATE_UNINITIALIZED)
	{
		if (m_on_created_callback != nullptr) m_on_created_callback();
		return true;
	}
	m_nWndState = WEBVIEW_STATE_INITIALIZING;
	
	static const char* s_szWindowClass = "ParaCraftWebView";
	static const char* s_szTitle = "ParaCraftWebView";
	std::thread([this, hInstance, hParentWnd]() -> bool {
		// Initializes the COM library on the current thread and identifies the concurrency model as single-thread apartment (STA).
		CoInitialize(NULL);

		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WebViewWndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		std::string sClassName = std::string(s_szWindowClass) + GetID();
		wcex.lpszClassName = sClassName.c_str();
		wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

		if (!RegisterClassEx(&wcex))
		{
			WriteLog("Error: ParaWebView RegisterClassEx(%s) Failed!\n", wcex.lpszClassName);
			if (m_on_created_callback != nullptr)
				m_on_created_callback();
			return false;
		}

		DWORD dwStyle;
		if (hParentWnd)
		{
			// dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN;
			dwStyle = WS_CHILD | WS_CLIPCHILDREN;
			SetWindowLong(hParentWnd, GWL_STYLE, GetWindowLong(hParentWnd, GWL_STYLE) | WS_CLIPCHILDREN);
		}
		else {
			dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
		}

		m_hWnd = CreateWindowEx(
			0,
			wcex.lpszClassName,
			s_szTitle,
			dwStyle,
			m_x, m_y, m_width, m_height,
			hParentWnd,
			NULL,
			hInstance,
			NULL
		);


		if (!m_hWnd)
		{
			WriteLog("Error: ParaWebView CreateWindow Failed\n");
			DWORD dwError = GetLastError();
			LPVOID lpMsgBuf;
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				dwError,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&lpMsgBuf,
				0, NULL);
			WriteLog("Error ParaWebView Msg: %s\n", (LPCTSTR)lpMsgBuf);
			// MessageBox(NULL, (LPCTSTR)lpMsgBuf, "CreateWindowEx failed", MB_OK | MB_ICONERROR);
			LocalFree(lpMsgBuf);

			if (m_on_created_callback != nullptr)
				m_on_created_callback();
			return false;
		}

		if (! CreateWebView(m_hWnd))
		{
			WriteLog("Error ParaWebView: failed to create window\n");
			
			if (m_on_created_callback != nullptr)
				m_on_created_callback();
			return false;
		}

		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		m_nWndState = WEBVIEW_STATE_DESTROYED;
		return true;
	}).detach();

	return true;
}


void WebView::SendShow()
{
	m_bShow = true;
	if (!IsInitialized())
		return;

	WebViewMessage msg;
	msg.m_visible = true;
	msg.m_cmd = "Show";
	SendWebViewMessage(msg);
}

void WebView::SendHide()
{
	m_bShow = false;
	if (!IsInitialized())
		return;
	WebViewMessage msg;
	msg.m_visible = false;
	msg.m_cmd = "Show";
	SendWebViewMessage(msg);
}

void WebView::SendDebug(bool debug)
{
    m_bDebug = debug;
	if (!IsInitialized())
		return;
	WebViewMessage msg;
	msg.m_debug = debug;
	msg.m_cmd = "Debug";
	SendWebViewMessage(msg);
}

/** thread-safe: return true if window is created.  */

bool WebView::IsInitialized()
{
	return m_nWndState == WEBVIEW_STATE_INITIALIZED;
}

void WebView::Show()
{
	m_bShow = true;
	if (m_hWnd == NULL || !IsInitialized()) return;

	ShowWindow(m_hWnd, m_bShow ? SW_SHOW : SW_HIDE);
	UpdateWindow(m_hWnd);

	if (m_webview_controller)
		m_webview_controller->put_IsVisible(IsShow() ? TRUE : FALSE);
}

void WebView::Hide()
{
	m_bShow = false;
	if (m_hWnd == NULL || !IsInitialized()) return;

	ShowWindow(m_hWnd, m_bShow ? SW_SHOW : SW_HIDE);
	UpdateWindow(m_hWnd);

	if (m_webview_controller)
		m_webview_controller->put_IsVisible(IsShow() ? TRUE : FALSE);
}

void WebView::Debug(bool enable)
{
    wil::com_ptr<ICoreWebView2Settings> settings;
    m_webview->get_Settings(&settings);
    if (enable)
    {
        settings->put_AreDevToolsEnabled(TRUE); 
		m_webview->OpenDevToolsWindow();
    }
    else
    {
        settings->put_AreDevToolsEnabled(FALSE); 
    }
}

void WebView::SendSetPositionMessage(int x, int y, int w, int h)
{
    m_x = x; m_y = y; m_width = w; m_height = h;
	if (m_webview_controller == nullptr) return;
	WebViewMessage msg;
	msg.m_cmd = "SetPosition";
	msg.m_webview = this;
	msg.m_x = x;
	msg.m_y = y;
	msg.m_width = w;
	msg.m_height = h;
	SendMessage(m_hWnd, WM_WEBVIEW_MESSAGE, 0, (LPARAM)&msg);
}

void WebView::AsyncSendWebMessage(const std::wstring& msg)
{
	WebViewMessage webmsg;
	webmsg.m_cmd = "SendWebMessage";
	webmsg.m_webview = this;
	webmsg.m_msg = msg;
	SendMessage(m_hWnd, WM_WEBVIEW_MESSAGE, 0, (LPARAM)&webmsg);
}

void WebView::SetPosition(int x, int y, int w, int h, bool bUpdateWndPosition)
{
	m_x = x; m_y = y; m_width = w; m_height = h;
	if (m_hWnd == NULL) return;

	if (bUpdateWndPosition) 
	{
		if (!SetWindowPos(m_hWnd, NULL, x, y, w, h, (m_bShow ? SWP_SHOWWINDOW : SWP_HIDEWINDOW) | SWP_NOACTIVATE))
		{
			WriteLog("Error: ParaWebView failed to set position!\n");
		}
	}
	if (!UpdateWindow(m_hWnd)) 
	{
		WriteLog("Error: ParaWebView failed to set UpdateWindow!\n");
	}

	RECT bounds;
	GetClientRect(m_hWnd, &bounds);
	if (m_webview_controller != nullptr)
	{
		m_webview_controller->put_Bounds(bounds);
	}
}

void WebView::SetOnCreateCallback(std::function<void()> callback)
{
	m_on_created_callback = callback;
}

void WebView::Destroy()
{
	if (m_hWnd == NULL) return;

	DestroyWindow(m_hWnd);
	m_hWnd = NULL;
	if (m_webview_controller)
	{
		m_webview_controller->Close();
		m_webview_controller = nullptr;
	}
	m_webview = nullptr;
	m_bShow = false;
	m_nWndState = WEBVIEW_STATE_DESTROYED;
}

void WebView::Open(const std::wstring& url)
{
	m_url = url;
	if (m_webview == nullptr || url == L"" || !IsInitialized())
		return;
	auto ok = m_webview->Navigate(url.c_str());
	assert(ok == S_OK);
	InitUrlEnv();
}

void WebView::SendOpenMessage(const std::wstring& url)
{
	m_url = url;
	if (m_webview == nullptr || !IsInitialized())
		return;
	WebViewMessage msg;
	msg.m_cmd = "Open";
	msg.m_webview = this;
	msg.m_url = url;
	SendMessage(m_hWnd, WM_WEBVIEW_MESSAGE, 0, (LPARAM)&msg);
}

/** thread-safe */

bool WebView::IsShow()
{
	return m_bShow;
}

bool WebView::CreateWebView(HWND hWnd)
{
	std::wstring user_data_folder = m_user_data_folder.empty() ? GetCacheDirectory() : m_user_data_folder;
	auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
	options->put_AdditionalBrowserArguments(L"--enable-features=AllowAutoplay --autoplay-policy=no-user-gesture-required");  // 视屏自动播放开启
	HRESULT ok = CreateCoreWebView2EnvironmentWithOptions(nullptr, user_data_folder.c_str(), options.Get(), Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([hWnd, this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
		if (FAILED(result)) {
			WriteLog("error: ParaWebView failed to call CreateCoreWebView2EnvironmentWithOptions\n");
			if (m_on_created_callback != nullptr)
				m_on_created_callback();
			return result;
		}

		// Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
		HRESULT ok = env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([hWnd, this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
			if (controller == nullptr) {
				WriteLog("error: ParaWebView failed to CreateCoreWebView2Controller\n");
				if (m_on_created_callback != nullptr)
					m_on_created_callback();
				return E_FAIL;
			}

			m_webview_controller = controller;
			m_webview_controller->get_CoreWebView2(&(m_webview));

			// Add a few settings for the webview
			// The demo step is redundant since the values are the default settings
			wil::com_ptr<ICoreWebView2Settings> settings;
			m_webview->get_Settings(&settings);
			settings->put_IsScriptEnabled(TRUE);
			settings->put_AreDefaultScriptDialogsEnabled(TRUE);
			settings->put_IsWebMessageEnabled(TRUE);
			wil::com_ptr<ICoreWebView2Settings3> settings3 = settings.try_query<ICoreWebView2Settings3>();
			if(settings3)
				settings3->put_AreBrowserAcceleratorKeysEnabled(IsDebug() ? TRUE : FALSE);
			settings->put_AreDevToolsEnabled(IsDebug() ? TRUE : FALSE);  // 调试工具禁用

			// Resize WebView to fit the bounds of the parent window
			RECT bounds;
			GetClientRect(hWnd, &bounds);
			m_webview_controller->put_Bounds(bounds);

			// <NavigationEvents>
			// Step 4 - Navigation events
			// register an ICoreWebView2NavigationStartingEventHandler to cancel any non-https navigation
			EventRegistrationToken token;
			m_webview->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>(
				[this](ICoreWebView2* webview, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
				wil::unique_cotaskmem_string uri_mem;
				args->get_Uri(&uri_mem);
				std::wstring uri(uri_mem.get());
				std::wcout << L"URL:" << uri << std::endl;
				auto pos = uri.find_first_of(L"paracraft://");
				if (pos == 0)
				{
					args->put_Cancel(true);
					ParseProtoUrl(uri);
				}
				return S_OK;
			}).Get(), &token);
			// </NavigationEvents>

			// <CommunicationHostWeb>
			// Step 6 - Communication between host and web content
			// Set an event handler for the host to return received message back to the web content
			m_webview->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
				[this](ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
				wil::unique_cotaskmem_string message;
				args->TryGetWebMessageAsString(&message);
				if (m_on_message_callback != nullptr) m_on_message_callback(message.get());
				webview->PostWebMessageAsString(message.get());
				return S_OK;
			}).Get(), &token);

			// better use a mutex
			m_nWndState = WEBVIEW_STATE_INITIALIZED;

			// open last opened url
			if (!m_url.empty())
				Open(m_url);
			// m_webview->OpenDevToolsWindow();

			if (IsShow())
				Show();
			else
				Hide();

			if (m_on_created_callback != nullptr)
				m_on_created_callback();


			return S_OK;
		}).Get());
		return ok == S_OK;
	}).Get());

	return ok == S_OK;
}

void WebView::OnWebMessage(std::function<void(const std::wstring&)> callback)
{
	m_on_message_callback = callback;
}

void WebView::PostWebMessage(const std::wstring& msg)
{
	if (m_webview != nullptr)
		m_webview->PostWebMessageAsString(msg.c_str());
}

void WebView::ExecuteScript(const std::wstring& script_text)
{
	if (m_webview == nullptr) return;
	auto ok = m_webview->ExecuteScript(script_text.c_str(), Callback<ICoreWebView2ExecuteScriptCompletedHandler>([](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT {
		return S_OK;
	}).Get());
	assert(ok == S_OK);
}

void WebView::ParseProtoUrl(const std::wstring url)
{
	static std::wstring s_sendmsg_proto = L"paracraft://sendmsg";
	if (url.find(s_sendmsg_proto) == 0 && m_on_proto_send_msg_callback != nullptr)
	{
		m_on_proto_send_msg_callback(url.substr(s_sendmsg_proto.length() + 1)); // 取?最后数据
	}
}

bool WebView::IsSupported()
{
	if (true || IsWindowsVersionOrGreater(9,0,0))
	{
		// for windows 7, it crashes with calling GetAvailableCoreWebView2BrowserVersionString when uninstalled
		auto pv = ReadRegStr(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Microsoft\\EdgeUpdate\\Clients\\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}", "pv");
		if (pv != "" && pv != "0.0.0.0") return true;
		pv = ReadRegStr(HKEY_CURRENT_USER, "Software\\Microsoft\\EdgeUpdate\\Clients\\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}", "pv");
		if (pv != "" && pv != "0.0.0.0") return true;
		pv = ReadRegStr(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\EdgeUpdate\\Clients\\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}", "pv");
		if (pv != "" && pv != "0.0.0.0") return true;
		return false;
	}
	else
	{
		// for windows 10 or 11
		wil::unique_cotaskmem_string webview_version = nullptr;
		try
		{
			// for win7, this method throw exceptions
			if (S_OK == GetAvailableCoreWebView2BrowserVersionString(nullptr, &webview_version) || webview_version == nullptr)
			{
				std::string version = WStringToString(webview_version.get());
				WriteLog("Webview2 version: %s\n", version.c_str());
				return true;
			}
			else if (webview_version == nullptr)
			{
				WriteLog("Error: ParaWebView webview2 not installed. please install Miscrosoft Edge. \n");
				return true;
			}
		}
		catch (const std::exception&)
		{
			auto pv = ReadRegStr(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Microsoft\\EdgeUpdate\\Clients\\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}", "pv");
			if (pv != "" && pv != "0.0.0.0") return true;
			pv = ReadRegStr(HKEY_CURRENT_USER, "Software\\Microsoft\\EdgeUpdate\\Clients\\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}", "pv");
			if (pv != "" && pv != "0.0.0.0") return true;
			pv = ReadRegStr(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\EdgeUpdate\\Clients\\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}", "pv");
			if (pv != "" && pv != "0.0.0.0") return true;
			return false;
		}
	}
	return false;
}

bool WebView::DownloadAndInstallWV2RT(std::function<void()> successed, std::function<void()> failed)
{
	HRESULT hr = URLDownloadToFile(NULL, "https://msedge.sf.dl.delivery.mp.microsoft.com/filestreamingservice/files/fe757736-da6f-40aa-801b-184c7e959409/MicrosoftEdgeWebview2Setup.exe", ".\\MicrosoftEdgeWebview2Setup.exe", 0, 0);
	if (hr == S_OK)
	{
		SHELLEXECUTEINFO shExInfo = { 0 };
		shExInfo.cbSize = sizeof(shExInfo);
		shExInfo.fMask = SEE_MASK_NOASYNC;
		shExInfo.hwnd = 0;
		shExInfo.lpVerb = "runas";
		shExInfo.lpFile = "MicrosoftEdgeWebview2Setup.exe";
		shExInfo.lpParameters = " /silent /install";
		shExInfo.lpDirectory = 0;
		shExInfo.nShow = 0;
		shExInfo.hInstApp = 0;

		if (ShellExecuteEx(&shExInfo))
		{
			WriteLog("ParaWebView: WV2RT Install successfull\n");
			if (successed != nullptr) successed();
			return true;
		}
		else
		{
			WriteLog("Error ParaWebView: WV2RT Install failed\n");
			if (failed != nullptr) failed();
		}
	}
	return false;
}

void WebView::InitUrlEnv()
{
	if (m_webview == nullptr) return;

	m_webview->AddScriptToExecuteOnDocumentCreated(LR"(
if (!window.chrome_webview_message_inited_cpp)
{
	window.chrome_webview_message_inited_cpp = true;
	window.chrome.webview.addEventListener("message", function(event){
		var msg = undefined;
		try
		{
			msg = JSON.parse(event.data);
		}
		catch(e)
		{
			console.log(e);
			console.log(event.data);
		}
		if (typeof msg != "object") 
		{
			console.log("无效数据:", event.data);
			return;
		}
		if (typeof window.NPL == "object" && typeof window.NPL.receive == "function")
		{
			window.NPL.receive(msg.file, msg.params);
		}
	}); 
	document.addEventListener('contextmenu', event => event.preventDefault());
}
    )", nullptr);
}

void WebView::SendWebViewMessage(WebViewMessage& msg)
{
	if (m_hWnd == NULL) return;
	msg.m_webview = this;
	SendMessage(m_hWnd, WM_WEBVIEW_MESSAGE, 0, (LPARAM)&msg);
}

std::wstring WebView::GetCacheDirectory()
{
	wchar_t szExePath[1024];
	GetModuleFileNameW(nullptr, szExePath, sizeof(szExePath));
	std::wstring path = szExePath;
#ifdef _DEBUG
	return path.substr(0, path.find_last_of(L"\\") + 1) + L"WindowWebView2CacheDebug";
#else
	return path.substr(0, path.find_last_of(L"\\") + 1) + L"WindowWebView2Cache";
#endif
}
