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

    static bool IsSupported();

    static bool DownloadAndInstallWV2RT(std::function<void()> successed, std::function<void()> failed);
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
    HWND GetWnd() { return m_hWnd; }
public:
    void OnCreated(std::function<void()> callback) { m_on_created_callback = callback; }

    // JS => C++ window.chrome.webview.postMessage("ExecuteScript")
    void OnWebMessage(std::function<void(const std::wstring&)> callback) { m_on_message_callback = callback; }
    // C++ => JS
    void SendWebMessage(const std::wstring& msg) { if (m_webview != nullptr) m_webview->PostWebMessageAsString(msg.c_str()); }
    // 兼容CEF
    void OnProtoSendMsgCallBack(std::function<void(const std::wstring&)> callback) { m_on_proto_send_msg_callback = callback; }
    // 执行JavaScript
    void ExecuteScript(const std::wstring& script_text);
public:
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