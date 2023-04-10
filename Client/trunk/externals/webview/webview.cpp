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

WebView::WebView()
{
    m_hWnd = NULL;
    m_bShow = true;
    m_x = 0;
    m_y = 0;
    m_width = 0;
    m_height = 0;

    m_on_created_callback = nullptr;
    m_on_message_callback = nullptr;
    m_on_proto_send_msg_callback = nullptr;
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
        if (msg->m_cmd == "Open") msg->m_webview->Open(msg->m_url);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
 }

bool WebView::Create(HINSTANCE hInstance, HWND hParentWnd)
{
    if (!IsSupported()) return false;

    if (m_hWnd)
    {
        if (m_on_created_callback != nullptr) m_on_created_callback();
        return true;
    }

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
        wcex.lpszClassName = s_szWindowClass;
        wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

        if (!RegisterClassEx(&wcex))
        {
            WriteLog("Error: WebView RegisterClassEx Failed!\n");
            return false;
        }

        DWORD dwStyle;
        if (hParentWnd)
        {
            dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN;
            // dwStyle = WS_POPUP;
            SetWindowLong(hParentWnd, GWL_STYLE, GetWindowLong(hParentWnd, GWL_STYLE) | WS_CLIPCHILDREN);
        }
        else {
            dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
        }

        m_hWnd = CreateWindowEx(
            0,
            s_szWindowClass,
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
            WriteLog("Error: WebView CreateWindow Failed\n");
            return false;
        }

        if (m_bShow) 
            Show();

        CreateWebView(m_hWnd);

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        return true;
        }).detach();

        return true;
}


void WebView::Show()
{
    m_bShow = true;
    if (m_hWnd == NULL) return;

    ShowWindow(m_hWnd, m_bShow ? SW_SHOW : SW_HIDE);
    UpdateWindow(m_hWnd);
}

void WebView::Hide()
{
    m_bShow = false;
    if (m_hWnd == NULL) return;

    ShowWindow(m_hWnd, m_bShow ? SW_SHOW : SW_HIDE);
    UpdateWindow(m_hWnd);
}

void WebView::SetPosition(int x, int y, int w, int h, bool bUpdateWndPosition)
{
    m_x = x; m_y = y; m_width = w; m_height = h;
    if (m_hWnd == NULL) return;

    // 设置窗口
    if (bUpdateWndPosition) SetWindowPos(m_hWnd, m_hParentWnd, x, y, w, h, (m_bShow ? SWP_SHOWWINDOW : SWP_HIDEWINDOW) | SWP_NOACTIVATE);

    // 更新到webview
    RECT bounds;
    GetClientRect(m_hWnd, &bounds);
    bounds.left = x;
    bounds.top = y;
    bounds.right = bounds.left + w;
    bounds.bottom = bounds.top + h;
    if (m_webview_controller != nullptr)
    {
        m_webview_controller->put_Bounds(bounds);
    }

    UpdateWindow(m_hWnd);
}

void WebView::Destroy()
{
    if (m_hWnd == NULL) return;

    DestroyWindow(m_hWnd);
    m_hWnd = NULL;
    m_webview_controller->Close();
    m_webview_controller = nullptr;
    m_webview = nullptr;
    m_bShow = false;
}

void WebView::Open(const std::wstring& url)
{
    m_url = url;
    if (m_webview == nullptr) return;
    m_webview->Navigate(url.c_str());
    auto ok = m_webview->Navigate(url.c_str());
    assert(ok == S_OK);
}

void WebView::SendOpenMessage(const std::wstring& url)
{
    m_url = url;
    if (m_webview == nullptr) return;
    WebViewMessage msg;
    msg.m_cmd = "Open";
    msg.m_webview = this;
    msg.m_url = url;
    SendMessage(m_hWnd, WM_WEBVIEW_MESSAGE, 0, (LPARAM)&msg);
}

bool WebView::CreateWebView(HWND hWnd)
{
    PCWSTR userDataFolder = m_user_data_folder.empty() ? nullptr : m_user_data_folder.c_str();
    HRESULT ok = CreateCoreWebView2EnvironmentWithOptions(nullptr, userDataFolder, nullptr, Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([hWnd, this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
        if (FAILED(result)) return result;

        // Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
        HRESULT ok = env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([hWnd, this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
            if (controller == nullptr) return E_FAIL;

            this->m_webview_controller = controller;
            this->m_webview_controller->get_CoreWebView2(&(this->m_webview));

            // Add a few settings for the webview
            // The demo step is redundant since the values are the default settings
            wil::com_ptr<ICoreWebView2Settings> settings;
            this->m_webview->get_Settings(&settings);
            settings->put_IsScriptEnabled(TRUE);
            settings->put_AreDefaultScriptDialogsEnabled(TRUE);
            settings->put_IsWebMessageEnabled(TRUE);

            // Resize WebView to fit the bounds of the parent window
            RECT bounds;
            GetClientRect(hWnd, &bounds);
            this->m_webview_controller->put_Bounds(bounds);

            // Schedule an async task to navigate to Bing
            // this->m_webview->Navigate(L"file:///D:/workspace/html/iframe.html");
            if (!this->m_url.empty()) this->m_webview->Navigate(this->m_url.c_str());

            // <NavigationEvents>
            // Step 4 - Navigation events
            // register an ICoreWebView2NavigationStartingEventHandler to cancel any non-https navigation
            EventRegistrationToken token;
            this->m_webview->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>(
                [this](ICoreWebView2* webview, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
                    wil::unique_cotaskmem_string uri_mem;
                    args->get_Uri(&uri_mem);
                    std::wstring uri(uri_mem.get());
                    std::wcout << L"URL:" << uri << std::endl;
                    auto pos = uri.find_first_of(L"paracraft://");
                    if (pos == 0)
                    {
                        args->put_Cancel(true);
                        this->ParseProtoUrl(uri);
                    }
                    return S_OK;
                }).Get(), &token);
            // </NavigationEvents>

            // <CommunicationHostWeb>
            // Step 6 - Communication between host and web content
            // Set an event handler for the host to return received message back to the web content
            this->m_webview->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                [this](ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                    wil::unique_cotaskmem_string message;
                    args->TryGetWebMessageAsString(&message);
                    if (this->m_on_message_callback != nullptr) this->m_on_message_callback(message.get());
                    webview->PostWebMessageAsString(message.get());
                    return S_OK;
                }).Get(), &token);

            // 创建成功回调
            if (this->m_on_created_callback != nullptr) this->m_on_created_callback();

            return S_OK;
            }).Get());
        return ok == S_OK;
        }).Get());

    return ok == S_OK;
}

void WebView::ExecuteScript(const std::wstring& script_text)
{
    if (m_webview == nullptr) return;
    this->m_webview->ExecuteScript(script_text.c_str(), Callback<ICoreWebView2ExecuteScriptCompletedHandler>([](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT {
        return S_OK;
        }).Get());
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
    auto pv = ReadRegStr(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Microsoft\\EdgeUpdate\\Clients\\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}", "pv");
    if (pv != "" && pv != "0.0.0.0") return true;
    pv = ReadRegStr(HKEY_CURRENT_USER, "Software\\Microsoft\\EdgeUpdate\\Clients\\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}", "pv");
    if (pv != "" && pv != "0.0.0.0") return true;
    pv = ReadRegStr(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\EdgeUpdate\\Clients\\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}", "pv");
    if (pv != "" && pv != "0.0.0.0") return true;

    wil::unique_cotaskmem_string webview_version = nullptr;
    // MS BUG: release version of webview2 runtime will return false. Debug version will return correctly. 
    assert(S_OK == GetAvailableCoreWebView2BrowserVersionString(nullptr, &webview_version));
    if (webview_version == nullptr)
    {
        WriteLog("Error: WebView webview2 not installed. please install Miscrosoft Edge. \n");
        return true;
    }
    else
    {
        std::string version = WStringToString(webview_version.get());
        WriteLog("Webview2 version: %s", version.c_str());
        return true;
    }
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
            WriteLog("Webview: WV2RT Install successfull\n");
            if (successed != nullptr) successed();
            return true;
        }
        else
        {
            WriteLog("Error Webview: WV2RT Install failed\n");
            if (failed != nullptr) failed();
        }
    }
    return false;
}