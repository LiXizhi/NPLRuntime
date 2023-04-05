#ifndef __WEBVIEW_H__
#define __WEBVIEW_H__
#include <iostream>
#include <assert.h>
#include <windows.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <functional>
#include <wrl.h>
#include <wil/com.h>
#include <Urlmon.h>
#include "WebView2.h"

#pragma comment(lib, "urlmon.lib")

using namespace Microsoft::WRL;


class WebView
{
public:
    static WebView* GetInstance()
    {
        static WebView s_webview;
        return &s_webview;
    } 

    static bool IsSupported()
    {
        wchar_t* webview_version = nullptr;
        assert(S_OK == GetAvailableCoreWebView2BrowserVersionString(nullptr, &webview_version));
        if (webview_version == nullptr)
        {
            std::cout << "当前系统暂不支持 webview2 请安装 Microsoft Edge!!!" << std::endl;
            return false;
        }
        else
        {
            std::wcout << L"webview2 version: " << webview_version << std::endl; 
            return true;
        }
    }

    static bool DownloadAndInstallWV2RT(std::function<void()> successed, std::function<void()> failed)
    {
        HRESULT hr = URLDownloadToFile(NULL, "https://msedge.sf.dl.delivery.mp.microsoft.com/filestreamingservice/files/fe757736-da6f-40aa-801b-184c7e959409/MicrosoftEdgeWebview2Setup.exe", ".\\MicrosoftEdgeWebview2Setup.exe", 0, 0);
        if (hr == S_OK)
        {
            SHELLEXECUTEINFO shExInfo = {0};
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
                std::cout << "WV2RT Install successfull" << std::endl;
                if (successed != nullptr) successed();
                return true;
            }
            else
            {
                std::cout << "WV2RT Install failed" << std::endl;
                if (failed != nullptr) failed();
            }
        }
        return false;
    }
public:
    WebView();
    ~WebView();

    bool Create(HINSTANCE hInstance, HWND hParentWnd = NULL);
    void Destroy();

    void Open(const std::wstring& url);
    bool IsShow() { return m_bShow; }
    void Show();
    void Hide();
    void SetPosition(int x, int y, int w, int h, bool bUpdateWndPosition = true);
    bool SetWnd(HWND hWnd);
public:
    void OnCreated(std::function<void()> callback) { m_on_created_callback = callback; }

    // JS => C++ window.chrome.webview.postMessage("ExecuteScript")
    void OnWebMessage(std::function<void(const std::wstring&)> callback) { m_on_message_callback = callback; }
    // C++ => JS
    void SendWebMessage(const std::wstring& msg) { if (m_webview != nullptr) m_webview->PostWebMessageAsString(msg.c_str()); }
    // 兼容CEF
    void OnProtoSendMsgCallBack(std::function<void(const std::wstring&)> callback)  { m_on_proto_send_msg_callback = callback; }
    // 执行JavaScript
    void ExecuteScript(const std::wstring& script_text);
protected:
    bool CreateWebView(HWND hWnd);
    void ParseProtoUrl(const std::wstring url);
protected:
    HWND m_hWnd;
    HWND m_hParentWnd;
    int m_x;
    int m_y;
    int m_width;
    int m_height;
    bool m_bShow;
    std::wstring m_url;
    std::wstring m_user_data_folder;
    wil::com_ptr<ICoreWebView2Controller> m_webview_controller;
    wil::com_ptr<ICoreWebView2> m_webview;

    std::function<void()> m_on_created_callback;
    std::function<void(const std::wstring&)> m_on_message_callback;
    std::function<void(const std::wstring&)> m_on_proto_send_msg_callback;
};


#endif