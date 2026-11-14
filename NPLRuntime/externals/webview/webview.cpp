//-----------------------------------------------------------------------------
// Class:	para webview
// Authors:	wxa, LiXizhi
// Date:	2023.4
//-----------------------------------------------------------------------------
#include <Windows.h>
#include <windowsx.h>
#include <VersionHelpers.h>
#include "webview.h"

#include <unordered_map>
#include <vector>
#include <algorithm>

extern void WriteLog(const char* sFormat, ...);
extern std::string WStringToString(std::wstring wstr);

static std::unordered_map<HWND, WebView*> s_hwndToWebView;
static std::unordered_map<HWND, WNDPROC> s_parentOrigProc;
static std::unordered_map<HWND, std::vector<WebView*>> s_parentToPopups;

static void RepositionPopupsForParent(HWND hParentWnd)
{
	auto it = s_parentToPopups.find(hParentWnd);
	if (it == s_parentToPopups.end()) return;
	for (auto* wv : it->second)
	{
		if (wv->GetWnd() && wv->IsCompositionMode())
		{
			wv->SetPosition(wv->GetX(), wv->GetY(), wv->GetWidth(), wv->GetHeight());
		}
	}
}

static bool IsWebViewRelatedWindow(HWND hWnd)
{
	if (!hWnd) return false;
	if (s_hwndToWebView.count(hWnd)) return true;
	HWND hRoot = GetAncestor(hWnd, GA_ROOT);
	if (hRoot && s_hwndToWebView.count(hRoot)) return true;
	return false;
}

static bool s_suppressingWebViewFocus = false;

static LRESULT CALLBACK ParentSubclassProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto it = s_parentOrigProc.find(hWnd);
	if (it == s_parentOrigProc.end())
		return DefWindowProc(hWnd, message, wParam, lParam);

	switch (message)
	{
	case WM_MOVE:
	case WM_MOVING:
	case WM_WINDOWPOSCHANGED:
	{
		LRESULT result = CallWindowProc(it->second, hWnd, message, wParam, lParam);
		RepositionPopupsForParent(hWnd);
		return result;
	}
	case WM_ACTIVATE:
	{
		WORD fActive = LOWORD(wParam);
		HWND hOther = (HWND)lParam;
		if (fActive == WA_INACTIVE)
		{
			bool webViewRelated = IsWebViewRelatedWindow(hOther);
			if (!webViewRelated && hOther == NULL)
			{
				HWND hFG = GetForegroundWindow();
				webViewRelated = s_suppressingWebViewFocus || IsWebViewRelatedWindow(hFG);
			}
			if (webViewRelated)
			{
				s_suppressingWebViewFocus = true;
				return 0;
			}
			s_suppressingWebViewFocus = false;
		}
		else
		{
			s_suppressingWebViewFocus = false;
		}
		break;
	}
	case WM_ACTIVATEAPP:
	{
		if (wParam == FALSE)
		{
			if (s_suppressingWebViewFocus)
				return 0;
			HWND hFG = GetForegroundWindow();
			if (IsWebViewRelatedWindow(hFG))
			{
				s_suppressingWebViewFocus = true;
				return 0;
			}
		}
		break;
	}
	case WM_KILLFOCUS:
	{
		HWND hNewFocus = (HWND)wParam;
		if (s_suppressingWebViewFocus || IsWebViewRelatedWindow(hNewFocus))
			return 0;
		break;
	}
	case WM_NCACTIVATE:
	{
		if (wParam == FALSE && s_suppressingWebViewFocus)
			return CallWindowProc(it->second, hWnd, message, TRUE, lParam);
		break;
	}
	case WM_DESTROY:
	{
		WNDPROC origProc = it->second;
		s_parentOrigProc.erase(hWnd);
		s_parentToPopups.erase(hWnd);
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)origProc);
		return CallWindowProc(origProc, hWnd, message, wParam, lParam);
	}
	}
	return CallWindowProc(it->second, hWnd, message, wParam, lParam);
}

static void SubclassParentWindow(HWND hParentWnd, WebView* popup)
{
	if (s_parentOrigProc.find(hParentWnd) == s_parentOrigProc.end())
	{
		WNDPROC origProc = (WNDPROC)SetWindowLongPtr(hParentWnd, GWLP_WNDPROC, (LONG_PTR)ParentSubclassProc);
		s_parentOrigProc[hParentWnd] = origProc;
	}
	auto& popups = s_parentToPopups[hParentWnd];
	if (std::find(popups.begin(), popups.end(), popup) == popups.end())
		popups.push_back(popup);
}

static void UnsubclassParentWindow(HWND hParentWnd, WebView* popup)
{
	auto it = s_parentToPopups.find(hParentWnd);
	if (it != s_parentToPopups.end())
	{
		auto& v = it->second;
		v.erase(std::remove(v.begin(), v.end(), popup), v.end());
		if (v.empty())
		{
			s_parentToPopups.erase(hParentWnd);
			auto origIt = s_parentOrigProc.find(hParentWnd);
			if (origIt != s_parentOrigProc.end())
			{
				if (IsWindow(hParentWnd))
					SetWindowLongPtr(hParentWnd, GWLP_WNDPROC, (LONG_PTR)origIt->second);
				s_parentOrigProc.erase(origIt);
			}
		}
	}
}

static std::string ReadRegStr(HKEY root_key, const std::string& sub_key, const std::string& name)
{
	HKEY hKey = nullptr;
	DWORD dwSize = 0;
	std::string value;
	if (RegOpenKeyExA(root_key, sub_key.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS)
	{
		if (ERROR_SUCCESS == RegQueryValueExA(hKey, name.c_str(), NULL, NULL, NULL, &dwSize) && dwSize > 0)
		{
			value.resize(dwSize - 1);
			if (RegGetValueA(hKey, NULL, name.c_str(), RRF_RT_REG_SZ, NULL, (void*)value.data(), &dwSize) != ERROR_SUCCESS)
				value.clear();
		}
		RegCloseKey(hKey);
	}
	return value;
}

WebView::WebView(const std::string& id)
{
	m_id = id;
	m_hWnd = NULL;
	m_hParentWnd = NULL;
	m_bShow = true;
	m_bDebug = false;
	m_bTransparent = false;
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

static COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS GetVirtualKeys()
{
	UINT vkeys = COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_NONE;
	if (GetKeyState(VK_LBUTTON) & 0x8000) vkeys |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_LEFT_BUTTON;
	if (GetKeyState(VK_RBUTTON) & 0x8000) vkeys |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_RIGHT_BUTTON;
	if (GetKeyState(VK_SHIFT) & 0x8000)   vkeys |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_SHIFT;
	if (GetKeyState(VK_CONTROL) & 0x8000) vkeys |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_CONTROL;
	if (GetKeyState(VK_MBUTTON) & 0x8000) vkeys |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_MIDDLE_BUTTON;
	return (COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS)vkeys;
}

LRESULT CALLBACK WebViewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WebViewMessage* msg = nullptr;
	switch (message)
	{
	case WM_MOUSEACTIVATE:
	{
		auto it = s_hwndToWebView.find(hWnd);
		if (it != s_hwndToWebView.end() && it->second->IsCompositionMode())
			return MA_NOACTIVATE;
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	case WM_ACTIVATE:
	{
		auto it = s_hwndToWebView.find(hWnd);
		if (it != s_hwndToWebView.end() && it->second->IsCompositionMode())
			return 0;
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	case WM_SETFOCUS:
	{
		auto it = s_hwndToWebView.find(hWnd);
		if (it != s_hwndToWebView.end() && it->second->IsCompositionMode())
			return 0;
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	case WM_WEBVIEW_MESSAGE:
		msg = (WebViewMessage*)lParam;
		if (msg->m_cmd == "Open")
			msg->m_webview->Open(msg->m_url);
		else if (msg->m_cmd == "Show")
			msg->m_visible ? msg->m_webview->Show() : msg->m_webview->Hide();
		else if (msg->m_cmd == "SetPosition")
			msg->m_webview->SetPosition(msg->m_x, msg->m_y, msg->m_width, msg->m_height);
		else if (msg->m_cmd == "SendWebMessage")
			msg->m_webview->PostWebMessage(msg->m_msg);
		else if (msg->m_cmd == "Debug")
			msg->m_webview->Debug(msg->m_debug);
		break;
	default:
	{
		auto it = s_hwndToWebView.find(hWnd);
		if (it != s_hwndToWebView.end() && it->second->IsCompositionMode())
		{
			COREWEBVIEW2_MOUSE_EVENT_KIND eventKind = (COREWEBVIEW2_MOUSE_EVENT_KIND)0;
			UINT32 mouseData = 0;
			bool isMouseMsg = true;
			switch (message)
			{
			case WM_MOUSEMOVE:     eventKind = COREWEBVIEW2_MOUSE_EVENT_KIND_MOVE; break;
			case WM_LBUTTONDOWN:   eventKind = COREWEBVIEW2_MOUSE_EVENT_KIND_LEFT_BUTTON_DOWN; break;
			case WM_LBUTTONUP:     eventKind = COREWEBVIEW2_MOUSE_EVENT_KIND_LEFT_BUTTON_UP; break;
			case WM_LBUTTONDBLCLK: eventKind = COREWEBVIEW2_MOUSE_EVENT_KIND_LEFT_BUTTON_DOUBLE_CLICK; break;
			case WM_RBUTTONDOWN:   eventKind = COREWEBVIEW2_MOUSE_EVENT_KIND_RIGHT_BUTTON_DOWN; break;
			case WM_RBUTTONUP:     eventKind = COREWEBVIEW2_MOUSE_EVENT_KIND_RIGHT_BUTTON_UP; break;
			case WM_RBUTTONDBLCLK: eventKind = COREWEBVIEW2_MOUSE_EVENT_KIND_RIGHT_BUTTON_DOUBLE_CLICK; break;
			case WM_MBUTTONDOWN:   eventKind = COREWEBVIEW2_MOUSE_EVENT_KIND_MIDDLE_BUTTON_DOWN; break;
			case WM_MBUTTONUP:     eventKind = COREWEBVIEW2_MOUSE_EVENT_KIND_MIDDLE_BUTTON_UP; break;
			case WM_MBUTTONDBLCLK: eventKind = COREWEBVIEW2_MOUSE_EVENT_KIND_MIDDLE_BUTTON_DOUBLE_CLICK; break;
			case WM_MOUSEWHEEL:
				eventKind = COREWEBVIEW2_MOUSE_EVENT_KIND_WHEEL;
				mouseData = GET_WHEEL_DELTA_WPARAM(wParam);
				break;
			case WM_MOUSEHWHEEL:
				eventKind = COREWEBVIEW2_MOUSE_EVENT_KIND_HORIZONTAL_WHEEL;
				mouseData = GET_WHEEL_DELTA_WPARAM(wParam);
				break;
			case WM_MOUSELEAVE:    eventKind = COREWEBVIEW2_MOUSE_EVENT_KIND_LEAVE; break;
			default: isMouseMsg = false; break;
			}
			if (isMouseMsg)
			{
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				if (message == WM_MOUSEWHEEL || message == WM_MOUSEHWHEEL)
				{
					ScreenToClient(hWnd, &pt);
				}
				it->second->GetCompositionController()->SendMouseInput(eventKind, GetVirtualKeys(), mouseData, pt);
				return 0;
			}
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	}
	return 0;
}

bool WebView::Create(HINSTANCE hInstance, HWND hParentWnd)
{
	if (!IsSupported())
	{
		if (m_on_created_callback != nullptr)
			m_on_created_callback();
		return false;
	}

	if (m_hWnd || m_nWndState != WEBVIEW_STATE_UNINITIALIZED)
	{
		if (m_on_created_callback != nullptr)
			m_on_created_callback();
		return true;
	}
	m_nWndState = WEBVIEW_STATE_INITIALIZING;

	static const char* s_szWindowClass = "ParaCraftWebView";
	static const char* s_szTitle = "ParaCraftWebView";
	std::thread([this, hInstance, hParentWnd]() -> bool {
		CoInitialize(NULL);

		WNDCLASSEXA wcex = {};
		wcex.cbSize = sizeof(WNDCLASSEXA);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WebViewWndProc;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = m_bTransparent ? (HBRUSH)GetStockObject(NULL_BRUSH) : (HBRUSH)(COLOR_WINDOW + 1);
		std::string sClassName = std::string(s_szWindowClass) + GetID();
		wcex.lpszClassName = sClassName.c_str();
		wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

		RegisterClassExA(&wcex);

		DWORD dwStyle = 0;
		DWORD dwExStyle = 0;
		int createX = m_x, createY = m_y;
		if (hParentWnd && m_bTransparent)
		{
			dwStyle = WS_POPUP | WS_CLIPCHILDREN;
			dwExStyle = WS_EX_NOREDIRECTIONBITMAP | WS_EX_NOACTIVATE;
			m_hParentWnd = hParentWnd;
			POINT pt = { m_x, m_y };
			ClientToScreen(hParentWnd, &pt);
			createX = pt.x;
			createY = pt.y;
		}
		else if (hParentWnd)
		{
			dwStyle = WS_CHILD | WS_CLIPCHILDREN;
			SetWindowLongPtr(hParentWnd, GWL_STYLE, GetWindowLongPtr(hParentWnd, GWL_STYLE) | WS_CLIPCHILDREN);
		}
		else
		{
			dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
		}

		m_hWnd = CreateWindowExA(
			dwExStyle,
			wcex.lpszClassName,
			s_szTitle,
			dwStyle,
			createX, createY, (std::max)(1, m_width), (std::max)(1, m_height),
			hParentWnd,
			NULL,
			hInstance,
			NULL
		);

		if (!m_hWnd)
		{
			WriteLog("Error: ParaWebView CreateWindow Failed (err=%lu)\n", (unsigned long)GetLastError());
			if (m_on_created_callback != nullptr)
				m_on_created_callback();
			return false;
		}

		if (!CreateWebView(m_hWnd))
		{
			WriteLog("Error ParaWebView: failed to create WebView2 controller\n");
			if (m_on_created_callback != nullptr)
				m_on_created_callback();
			return false;
		}

		if (m_bTransparent && hParentWnd)
			SubclassParentWindow(hParentWnd, this);

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
	if (!IsInitialized()) return;
	WebViewMessage msg;
	msg.m_visible = true;
	msg.m_cmd = "Show";
	SendWebViewMessage(msg);
}

void WebView::SendHide()
{
	m_bShow = false;
	if (!IsInitialized()) return;
	WebViewMessage msg;
	msg.m_visible = false;
	msg.m_cmd = "Show";
	SendWebViewMessage(msg);
}

void WebView::SendDebug(bool debug)
{
	m_bDebug = debug;
	if (!IsInitialized()) return;
	WebViewMessage msg;
	msg.m_debug = debug;
	msg.m_cmd = "Debug";
	SendWebViewMessage(msg);
}

bool WebView::IsInitialized()
{
	return m_nWndState == WEBVIEW_STATE_INITIALIZED;
}

void WebView::Show()
{
	m_bShow = true;
	if (m_hWnd == NULL || !IsInitialized()) return;
	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);
	if (m_webview_controller)
		m_webview_controller->put_IsVisible(TRUE);
}

void WebView::Hide()
{
	m_bShow = false;
	if (m_hWnd == NULL || !IsInitialized()) return;
	ShowWindow(m_hWnd, SW_HIDE);
	UpdateWindow(m_hWnd);
	if (m_webview_controller)
		m_webview_controller->put_IsVisible(FALSE);
}

void WebView::Debug(bool enable)
{
	if (!m_webview) return;
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
	if (!m_hWnd) return;
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
		int posX = x, posY = y;
		if (m_bTransparent && m_hParentWnd)
		{
			POINT pt = { x, y };
			ClientToScreen(m_hParentWnd, &pt);
			posX = pt.x;
			posY = pt.y;
		}
		if (!SetWindowPos(m_hWnd, NULL, posX, posY, w, h, (m_bShow ? SWP_SHOWWINDOW : SWP_HIDEWINDOW) | SWP_NOACTIVATE))
		{
			WriteLog("Error: ParaWebView failed to set position!\n");
		}
	}
	UpdateWindow(m_hWnd);

	RECT bounds = {};
	GetClientRect(m_hWnd, &bounds);
	if (m_webview_controller != nullptr)
	{
		m_webview_controller->put_Bounds(bounds);
	}
	if (m_dcomp_device)
	{
		m_dcomp_device->Commit();
	}
}

void WebView::SetOnCreateCallback(std::function<void()> callback)
{
	m_on_created_callback = callback;
}

void WebView::Destroy()
{
	if (m_hWnd == NULL) return;

	if (m_hParentWnd)
	{
		UnsubclassParentWindow(m_hParentWnd, this);
	}

	s_hwndToWebView.erase(m_hWnd);
	DestroyWindow(m_hWnd);
	m_hWnd = NULL;
	m_hParentWnd = NULL;
	if (m_webview_controller)
	{
		m_webview_controller->Close();
		m_webview_controller = nullptr;
	}
	m_composition_controller = nullptr;
	m_dcomp_visual = nullptr;
	m_dcomp_target = nullptr;
	m_dcomp_device = nullptr;
	m_webview = nullptr;
	m_bShow = false;
	m_nWndState = WEBVIEW_STATE_DESTROYED;
}

void WebView::Open(const std::wstring& url)
{
	m_url = url;
	if (m_webview == nullptr || url.empty() || !IsInitialized())
		return;
	m_webview->Navigate(url.c_str());
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

bool WebView::IsShow()
{
	return m_bShow;
}

bool WebView::CreateWebView(HWND hWnd)
{
	if (m_bTransparent)
	{
		return CreateCompositionWebView(hWnd);
	}
	std::wstring user_data_folder = m_user_data_folder.empty() ? GetCacheDirectory() : m_user_data_folder;
	auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
	options->put_AdditionalBrowserArguments(L"--enable-features=AllowAutoplay --autoplay-policy=no-user-gesture-required");
	HRESULT ok = CreateCoreWebView2EnvironmentWithOptions(nullptr, user_data_folder.c_str(), options.Get(),
		Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([hWnd, this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
			if (FAILED(result) || !env)
			{
				WriteLog("error: ParaWebView failed to call CreateCoreWebView2EnvironmentWithOptions\n");
				if (m_on_created_callback != nullptr)
					m_on_created_callback();
				return result;
			}

			return env->CreateCoreWebView2Controller(hWnd,
				Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([hWnd, this](HRESULT, ICoreWebView2Controller* controller) -> HRESULT {
					if (controller == nullptr)
					{
						WriteLog("error: ParaWebView failed to CreateCoreWebView2Controller\n");
						if (m_on_created_callback != nullptr)
							m_on_created_callback();
						return E_FAIL;
					}
					m_webview_controller = controller;
					m_webview_controller->get_CoreWebView2(&m_webview);
					SetupWebViewAfterCreation(hWnd);
					return S_OK;
				}).Get());
		}).Get());

	return SUCCEEDED(ok);
}

bool WebView::CreateCompositionWebView(HWND hWnd)
{
	HRESULT hr = DCompositionCreateDevice(nullptr, IID_PPV_ARGS(&m_dcomp_device));
	if (FAILED(hr))
	{
		WriteLog("error: WebView DCompositionCreateDevice FAILED\n");
		if (m_on_created_callback != nullptr)
			m_on_created_callback();
		return false;
	}

	hr = m_dcomp_device->CreateTargetForHwnd(hWnd, TRUE, &m_dcomp_target);
	if (FAILED(hr))
	{
		WriteLog("error: WebView CreateTargetForHwnd FAILED\n");
		if (m_on_created_callback != nullptr)
			m_on_created_callback();
		return false;
	}

	hr = m_dcomp_device->CreateVisual(&m_dcomp_visual);
	if (FAILED(hr))
	{
		WriteLog("error: WebView CreateVisual FAILED\n");
		if (m_on_created_callback != nullptr)
			m_on_created_callback();
		return false;
	}

	m_dcomp_target->SetRoot(m_dcomp_visual.get());

	std::wstring user_data_folder = m_user_data_folder.empty() ? GetCacheDirectory() : m_user_data_folder;
	auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
	options->put_AdditionalBrowserArguments(L"--enable-features=AllowAutoplay --autoplay-policy=no-user-gesture-required");
	hr = CreateCoreWebView2EnvironmentWithOptions(nullptr, user_data_folder.c_str(), options.Get(),
		Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([hWnd, this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
			if (FAILED(result) || !env)
			{
				WriteLog("error: ParaWebView failed to call CreateCoreWebView2EnvironmentWithOptions\n");
				if (m_on_created_callback != nullptr)
					m_on_created_callback();
				return result;
			}

			wil::com_ptr<ICoreWebView2Environment3> env3;
			HRESULT hr = env->QueryInterface(IID_PPV_ARGS(&env3));
			if (FAILED(hr) || !env3)
			{
				return env->CreateCoreWebView2Controller(hWnd,
					Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([hWnd, this](HRESULT, ICoreWebView2Controller* controller) -> HRESULT {
						if (controller == nullptr)
						{
							if (m_on_created_callback != nullptr)
								m_on_created_callback();
							return E_FAIL;
						}
						m_webview_controller = controller;
						m_webview_controller->get_CoreWebView2(&m_webview);
						SetupWebViewAfterCreation(hWnd);
						return S_OK;
					}).Get());
			}

			return env3->CreateCoreWebView2CompositionController(hWnd,
				Callback<ICoreWebView2CreateCoreWebView2CompositionControllerCompletedHandler>([hWnd, this](HRESULT, ICoreWebView2CompositionController* compositionController) -> HRESULT {
					if (compositionController == nullptr)
					{
						if (m_on_created_callback != nullptr)
							m_on_created_callback();
						return E_FAIL;
					}

					m_composition_controller = compositionController;
					m_composition_controller->put_RootVisualTarget(m_dcomp_visual.get());
					m_composition_controller->QueryInterface(IID_PPV_ARGS(&m_webview_controller));
					m_webview_controller->get_CoreWebView2(&m_webview);

					wil::com_ptr<ICoreWebView2Controller2> controller2;
					if (SUCCEEDED(m_webview_controller->QueryInterface(IID_PPV_ARGS(&controller2))) && controller2)
					{
						COREWEBVIEW2_COLOR bgColor = { 0, 0, 0, 0 };
						controller2->put_DefaultBackgroundColor(bgColor);
					}

					m_dcomp_device->Commit();
					s_hwndToWebView[hWnd] = this;

					EventRegistrationToken cursorToken;
					m_composition_controller->add_CursorChanged(Callback<ICoreWebView2CursorChangedEventHandler>(
						[hWnd](ICoreWebView2CompositionController* sender, IUnknown*) -> HRESULT {
							HCURSOR cursor = nullptr;
							if (SUCCEEDED(sender->get_Cursor(&cursor)))
								SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR)cursor);
							return S_OK;
						}).Get(), &cursorToken);

					SetupWebViewAfterCreation(hWnd);
					return S_OK;
				}).Get());
		}).Get());

	return SUCCEEDED(hr);
}

void WebView::SetupWebViewAfterCreation(HWND hWnd)
{
	wil::com_ptr<ICoreWebView2Settings> settings;
	m_webview->get_Settings(&settings);
	settings->put_IsScriptEnabled(TRUE);
	settings->put_AreDefaultScriptDialogsEnabled(TRUE);
	settings->put_IsWebMessageEnabled(TRUE);
	wil::com_ptr<ICoreWebView2Settings3> settings3 = settings.try_query<ICoreWebView2Settings3>();
	if (settings3)
		settings3->put_AreBrowserAcceleratorKeysEnabled(IsDebug() ? TRUE : FALSE);
	settings->put_AreDevToolsEnabled(IsDebug() ? TRUE : FALSE);

	RECT bounds = {};
	GetClientRect(hWnd, &bounds);
	m_webview_controller->put_Bounds(bounds);

	EventRegistrationToken token;
	m_webview->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>(
		[this](ICoreWebView2*, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
			wil::unique_cotaskmem_string uri_mem;
			args->get_Uri(&uri_mem);
			std::wstring uri(uri_mem.get() ? uri_mem.get() : L"");
			if (uri.find(L"paracraft://") == 0)
			{
				args->put_Cancel(true);
				ParseProtoUrl(uri);
			}
			return S_OK;
		}).Get(), &token);

	m_webview->add_PermissionRequested(Callback<ICoreWebView2PermissionRequestedEventHandler>(
		[](ICoreWebView2*, ICoreWebView2PermissionRequestedEventArgs* args) -> HRESULT {
			COREWEBVIEW2_PERMISSION_KIND permissionType;
			args->get_PermissionKind(&permissionType);
			if (permissionType == COREWEBVIEW2_PERMISSION_KIND_CAMERA)
			{
				args->put_State(COREWEBVIEW2_PERMISSION_STATE_ALLOW);
			}
			return S_OK;
		}).Get(), &token);

	m_webview->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
		[this](ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
			wil::unique_cotaskmem_string message;
			if (SUCCEEDED(args->TryGetWebMessageAsString(&message)) && message.get())
			{
				if (m_on_message_callback != nullptr)
					m_on_message_callback(message.get());
				webview->PostWebMessageAsString(message.get());
			}
			return S_OK;
		}).Get(), &token);

	m_nWndState = WEBVIEW_STATE_INITIALIZED;

	InitUrlEnv();

	if (!m_url.empty())
		Open(m_url);

	if (IsShow())
		Show();
	else
		Hide();

	if (m_on_created_callback != nullptr)
		m_on_created_callback();
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
	m_webview->ExecuteScript(script_text.c_str(), Callback<ICoreWebView2ExecuteScriptCompletedHandler>([](HRESULT, LPCWSTR) -> HRESULT {
		return S_OK;
	}).Get());
}

void WebView::ParseProtoUrl(const std::wstring url)
{
	static std::wstring s_sendmsg_proto = L"paracraft://sendmsg";
	if (url.find(s_sendmsg_proto) == 0 && m_on_proto_send_msg_callback != nullptr && url.length() > s_sendmsg_proto.length())
	{
		m_on_proto_send_msg_callback(url.substr(s_sendmsg_proto.length() + 1));
	}
}

bool WebView::IsSupported()
{
	auto pv = ReadRegStr(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Microsoft\\EdgeUpdate\\Clients\\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}", "pv");
	if (!pv.empty() && pv != "0.0.0.0") return true;
	pv = ReadRegStr(HKEY_CURRENT_USER, "Software\\Microsoft\\EdgeUpdate\\Clients\\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}", "pv");
	if (!pv.empty() && pv != "0.0.0.0") return true;
	pv = ReadRegStr(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\EdgeUpdate\\Clients\\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}", "pv");
	if (!pv.empty() && pv != "0.0.0.0") return true;
	return false;
}

bool WebView::DownloadAndInstallWV2RT(std::function<void()> successed, std::function<void()> failed)
{
	HRESULT hr = URLDownloadToFileA(NULL, "https://msedge.sf.dl.delivery.mp.microsoft.com/filestreamingservice/files/fe757736-da6f-40aa-801b-184c7e959409/MicrosoftEdgeWebview2Setup.exe", ".\\MicrosoftEdgeWebview2Setup.exe", 0, 0);
	if (hr == S_OK)
	{
		SHELLEXECUTEINFOA shExInfo = {};
		shExInfo.cbSize = sizeof(shExInfo);
		shExInfo.fMask = SEE_MASK_NOASYNC;
		shExInfo.lpVerb = "runas";
		shExInfo.lpFile = "MicrosoftEdgeWebview2Setup.exe";
		shExInfo.lpParameters = " /silent /install";

		if (ShellExecuteExA(&shExInfo))
		{
			WriteLog("ParaWebView: WV2RT Install successful\n");
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
	wchar_t szExePath[1024] = { 0 };
	GetModuleFileNameW(nullptr, szExePath, static_cast<DWORD>(sizeof(szExePath) / sizeof(wchar_t)) - 1);
	std::wstring path = szExePath;
	size_t slashPos = path.find_last_of(L"\\/");
	std::wstring baseDir = (slashPos != std::wstring::npos) ? path.substr(0, slashPos + 1) : L"";
#ifdef _DEBUG
	return baseDir + L"WindowWebView2CacheDebug";
#else
	return baseDir + L"WindowWebView2Cache";
#endif
}
