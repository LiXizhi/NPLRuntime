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
#include <dcomp.h>
#include "WebView2.h"
#include "WebView2EnvironmentOptions.h"
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "dcomp.lib")

using namespace Microsoft::WRL;

#define WM_WEBVIEW_MESSAGE (WM_USER + 1)

class WebView;
class WebViewMessage
{
public:
    WebView* m_webview;
    std::string m_cmd;
    std::wstring m_msg;
    std::wstring m_url;
    bool m_visible;
    bool m_debug;
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
    HWND GetParentWnd() { return m_hParentWnd; }
    const std::string& GetID() { return m_id; }
    int GetX() const { return m_x; }
    int GetY() const { return m_y; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

public:
    void SendOpenMessage(const std::wstring& url);
    bool IsShow();
    void SendWebViewMessage(WebViewMessage& msg);
    void SendShow();
    void SendHide();
    void SendDebug(bool debug);
    void SetTransparentFlag(bool transparent) { m_bTransparent = transparent; }
    bool IsInitialized();
    void SendSetPositionMessage(int x, int y, int w, int h);
    void AsyncSendWebMessage(const std::wstring& msg);

public:
    void Show();
    void Hide();
    void Open(const std::wstring& url);
    void SetPosition(int x, int y, int w, int h, bool bUpdateWndPosition = true);
    void Debug(bool enable);

public:
    void SetOnCreateCallback(std::function<void()> callback);
    void OnWebMessage(std::function<void(const std::wstring&)> callback);
    void PostWebMessage(const std::wstring& msg);
    void OnProtoSendMsgCallBack(std::function<void(const std::wstring&)> callback) { m_on_proto_send_msg_callback = callback; }
    void ExecuteScript(const std::wstring& script_text);

public:
    bool IsSupportWebView() { return m_webview != nullptr; }
    bool IsDebug() { return m_bDebug; }
    bool IsCompositionMode() { return m_composition_controller != nullptr; }
    ICoreWebView2CompositionController* GetCompositionController() { return m_composition_controller.get(); }
    bool CreateWebView(HWND hWnd);
    bool CreateCompositionWebView(HWND hWnd);
    void SetupWebViewAfterCreation(HWND hWnd);
    void ParseProtoUrl(const std::wstring url);
    void InitUrlEnv();
    std::wstring GetCacheDirectory();

protected:
    HWND m_hWnd;
    HWND m_hParentWnd;
    std::string m_id;
    int m_x;
    int m_y;
    int m_width;
    int m_height;
    bool m_bShow;
    bool m_bDebug;
    bool m_bTransparent;
    WEBVIEW_STATE m_nWndState;
    std::wstring m_url;
    std::wstring m_navigated_url;
    std::wstring m_user_data_folder;
    wil::com_ptr<ICoreWebView2Controller> m_webview_controller;
    wil::com_ptr<ICoreWebView2> m_webview;
    wil::com_ptr<ICoreWebView2CompositionController> m_composition_controller;
    wil::com_ptr<IDCompositionDevice> m_dcomp_device;
    wil::com_ptr<IDCompositionTarget> m_dcomp_target;
    wil::com_ptr<IDCompositionVisual> m_dcomp_visual;

    std::function<void()> m_on_created_callback;
    std::function<void(const std::wstring&)> m_on_message_callback;
    std::function<void(const std::wstring&)> m_on_proto_send_msg_callback;
};

#endif
