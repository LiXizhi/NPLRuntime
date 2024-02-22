#ifndef __WEBVIEW_H__
#define __WEBVIEW_H__
#include <iostream>
#include <assert.h>
#include <windows.h>
#include <stdlib.h>
#include <codecvt>
#include <locale>
#include <string>
#include <thread>
#include <functional>
#include <wrl.h>
#include <wil/com.h>
#include <Urlmon.h>
#include "WebView2.h"
#include "WebView2EnvironmentOptions.h"
#pragma comment(lib, "urlmon.lib")

using namespace Microsoft::WRL;

// 定义消息ID
#define WM_WEBVIEW_MESSAGE (WM_USER + 1)


class WebView;
class WebViewMessage
{
public:
    WebView* m_webview;    // 浏览器对象
    std::string m_cmd;     // 命令
    std::wstring m_msg;    // 发送消息
    std::wstring m_url;    // 打开URL
    bool m_visible;        // 是否显示
    bool m_debug;          // 是否支持调试
    // 位置信息
    int m_x;
    int m_y;
    int m_width;
    int m_height;    
};

class WebView
{
public:
    enum WEBVIEW_STATE {
        WEBVIEW_STATE_UNINITIALIZED = -1,
        WEBVIEW_STATE_DESTROYED = 0,
        WEBVIEW_STATE_INITIALIZED = 1,
        WEBVIEW_STATE_INITIALIZING = -2,
    };
    
    static WebView* GetInstance()
    {
        static WebView s_webview;
        return &s_webview;
    }

    static bool IsSupported();

    static bool DownloadAndInstallWV2RT(std::function<void()> successed, std::function<void()> failed);
public:
    WebView(const std::string& id = "");
    ~WebView();

    bool Create(HINSTANCE hInstance, HWND hParentWnd = NULL);
    void Destroy();

    bool SetWnd(HWND hWnd);
    HWND GetWnd() { return m_hWnd; }
    const std::string& GetID() { return m_id; }
public:
    /** thread-safe */
    void SendOpenMessage(const std::wstring& url);
    /** thread-safe */
    bool IsShow();
    /** thread-safe */
    void SendWebViewMessage(WebViewMessage& msg);
    /** thread-safe */
    void SendShow();
    /** thread-safe */
    void SendHide();
    /** thread-safe */
    void SendDebug(bool debug);
    /** thread-safe: return true if window is created.  */
    bool IsInitialized();
    /** thread-safe */
    void SendSetPositionMessage(int x, int y, int w, int h);
    /** thread-safe: send from C++/NPL => JS */
    void AsyncSendWebMessage(const std::wstring& msg);

public:
    void Show();
    void Hide();
    void Open(const std::wstring& url);
    void SetPosition(int x, int y, int w, int h, bool bUpdateWndPosition = true);
    void Debug(bool enable);
public:
    void SetOnCreateCallback(std::function<void()> callback);

    // JS => C++ window.chrome.webview.postMessage("ExecuteScript")
    void OnWebMessage(std::function<void(const std::wstring&)> callback);
    // C++ => JS
    void PostWebMessage(const std::wstring& msg);
    
    // backward compatible for CEF3
    void OnProtoSendMsgCallBack(std::function<void(const std::wstring&)> callback) { m_on_proto_send_msg_callback = callback; }
    // execute any JavaScript
    void ExecuteScript(const std::wstring& script_text);
public:
    bool IsSupportWebView() { return m_webview != nullptr; }
    bool IsDebug() { return m_bDebug; }
    bool CreateWebView(HWND hWnd);
    void ParseProtoUrl(const std::wstring url);
    void InitUrlEnv();
    
    std::wstring GetCacheDirectory();
protected:
    HWND m_hWnd;
    std::string m_id;
    int m_x;
    int m_y;
    int m_width;
    int m_height;
    bool m_bShow;
    bool m_bDebug;
    WEBVIEW_STATE m_nWndState;
    std::wstring m_url;
    std::wstring m_user_data_folder;
    wil::com_ptr<ICoreWebView2Controller> m_webview_controller;
    wil::com_ptr<ICoreWebView2> m_webview;

    std::function<void()> m_on_created_callback;
    std::function<void(const std::wstring&)> m_on_message_callback;
    std::function<void(const std::wstring&)> m_on_proto_send_msg_callback;
};


#endif