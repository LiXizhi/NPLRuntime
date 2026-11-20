#include "ParaEngine.h"
#include "ParaEngineSettings.h"
#include "util/os_calls.h"
#include "util/StringHelper.h"
#include "ObjectAutoReleasePool.h"
#include "AttributesManager.h"
#include "PluginManager.h"
#include "MeshPhysicsObject.h"
#include "MeshObject.h"
#include "MissileObject.h"
#include "BipedObject.h"
#include "BMaxModel/BMaxObject.h"
#include "SkyMesh.h"
#include "BlockPieceParticle.h"
#include "ContainerObject.h"
#include "RenderTarget.h"
#include "WeatherEffect.h"
#include "OverlayObject.h"
#include "LightObject.h"
#include "NPLRuntime.h"
#include "EventsCenter.h"
#include "BootStrapper.h"
#include "NPL/NPLHelper.h"
#include "AISimulator.h"
#include "AsyncLoader.h"
#include "FileManager.h"
#include "Archive.h"
#include "NPLPackageConfig.h"
#include "IO/ResourceEmbedded.h"
#include "2dengine/GUIRoot.h"
#include "2dengine/GUIMouseVirtual.h"
#include "FrameRateController.h"
#include "FileLogger.h"
#include "InfoCenter/ICDBManager.h"
#include "InfoCenter/ICConfigManager.h"
#include "SceneObject.h"
#include "ParaWorldAsset.h"
#include "terrain/Terrain.h"
#include "ViewportManager.h"
#include "MoviePlatform.h"
#include "Framework/Common/Time/ParaTimer.h"
#include "EnvironmentSim.h"
#include "AutoCamera.h"
#include "BlockEngine/BlockWorldClient.h"
#include "BlockEngine/BlockWorldManager.h"
#include "ObjectManager.h"
#include "2dengine/GUIHighlight.h"
#include "3dengine/AudioEngine2.h"
#include "WindowsApplication.h"
#include "RenderWindowDelegate.h"
#include "DynamicAttributeField.h"
#include "ParaEngineInfo.h"
#include "OSWindows.h"
#include "GuiConsole.h"
#include "RHIBackendRegistry.h"

#if defined(PARA_RHI_HAS_VULKAN) && __has_include("RenderContextVulkan.h")
#include "RenderContextVulkan.h"
#define PARA_WIN_HAS_VULKAN_CTX 1
#endif
#if defined(PARA_RHI_HAS_D3D12) && __has_include("RenderContextD3D12.h")
#include "RenderContextD3D12.h"
#define PARA_WIN_HAS_D3D12_CTX 1
#endif
#ifdef PARA_RHI_HAS_D3D11
#include "RenderContextD3D11.h"
#endif
#ifdef PARA_RHI_HAS_OPENGL
#include "Render/context/wgl/RenderContextWGL.h"
#endif
#if defined(PARA_RHI_HAS_WEBGPU) && __has_include("RenderContextWebGPU.h")
#include "RenderContextWebGPU.h"
#define PARA_WIN_HAS_WEBGPU_CTX 1
#endif

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <objbase.h>
#include <objidl.h>
#include <gdiplus.h>
#include "resource.h"

HINSTANCE g_hAppInstance = nullptr;

ParaEngine::IParaEngineApp* CreateParaEngineApp()
{
	return new ParaEngine::CWindowsApplication(g_hAppInstance);
}

namespace ParaEngine
{
	namespace
	{
		HKEY ResolveRootHKey(const std::string& rootKey)
		{
			if (rootKey == "HKCR" || rootKey == "HKEY_CLASSES_ROOT")
				return HKEY_CLASSES_ROOT;
			if (rootKey == "HKLM" || rootKey == "HKEY_LOCAL_MACHINE")
				return HKEY_LOCAL_MACHINE;
			if (rootKey == "HKCU" || rootKey == "HKEY_CURRENT_USER")
				return HKEY_CURRENT_USER;
			if (rootKey == "HKU" || rootKey == "HKEY_USERS")
				return HKEY_USERS;
			return HKEY_CURRENT_USER;
		}

		std::wstring NormalizeRegSubKeyW(const std::string& subKeyUtf8)
		{
			std::string normalized = subKeyUtf8;
			for (char& ch : normalized)
			{
				if (ch == '/')
					ch = '\\';
			}
			while (!normalized.empty() && normalized.front() == '\\')
				normalized.erase(normalized.begin());
			return StringHelper::MultiByteToWideChar(normalized.c_str(), CP_UTF8);
		}

		HKEY OpenOrCreateRegKey(const std::string& rootKey, const std::string& subKey, REGSAM samDesired, bool bCreate)
		{
			HKEY hRoot = ResolveRootHKey(rootKey);
			std::wstring wSubKey = NormalizeRegSubKeyW(subKey);
			HKEY hKey = nullptr;
			LONG lRet = ERROR_SUCCESS;
			if (bCreate)
			{
				DWORD dwDisposition = 0;
				lRet = ::RegCreateKeyExW(
					hRoot,
					wSubKey.c_str(),
					0,
					nullptr,
					REG_OPTION_NON_VOLATILE,
					samDesired,
					nullptr,
					&hKey,
					&dwDisposition);
			}
			else
			{
				lRet = ::RegOpenKeyExW(hRoot, wSubKey.c_str(), 0, samDesired, &hKey);
			}

			if (lRet != ERROR_SUCCESS)
			{
				return nullptr;
			}
			return hKey;
		}

		void BringHwndToForegroundForce(HWND hWnd)
		{
			if (!hWnd || !::IsWindow(hWnd))
				return;
			HWND hFore = ::GetForegroundWindow();
			if (hFore == hWnd)
				return;
			DWORD dwForeThread = hFore ? ::GetWindowThreadProcessId(hFore, nullptr) : 0;
			DWORD dwTargetThread = ::GetWindowThreadProcessId(hWnd, nullptr);
			if (dwForeThread != 0 && dwTargetThread != 0 && dwForeThread != dwTargetThread)
			{
				::AttachThreadInput(dwForeThread, dwTargetThread, TRUE);
				::SetForegroundWindow(hWnd);
				::BringWindowToTop(hWnd);
				::AttachThreadInput(dwForeThread, dwTargetThread, FALSE);
			}
			else
			{
				::SetForegroundWindow(hWnd);
				::BringWindowToTop(hWnd);
			}
		}
	} // anonymous namespace

	CWindowsApplication::CWindowsApplication(HINSTANCE hInstance)
		: m_hInst(hInstance)
		, m_hExternalWnd(nullptr)
		, m_bIsExternalWindow(false)
		, m_dwWinThreadID(::GetCurrentThreadId())
		, m_dwCoreUsage(PE_USAGE_STANDALONE | PE_USAGE_OWN_D3D | PE_USAGE_OWN_WINDOW)
		, m_nCreationWidth(1020)
		, m_nCreationHeight(680)
		, m_nMultiSampleType(0)
		, m_nMultiSampleQuality(0)
		, m_hMenu(nullptr)
		, m_bComInitialized(false)
		, m_gdiplusToken(0)
		, m_bUpdateScreenDevice(false)
		, m_bSizeChanged(false)
		, m_bModeChanged(false)
		, m_bAllowWindowClosing(true)
		, m_bHasNewConfig(false)
		, m_bIgnoreSizeChange(false)
		, m_bPassiveRendering(false)
		, m_bAutoLowerFrameRateWhenNotFocused(false)
		, m_bToggleSoundWhenNotFocused(false)
		, m_fSavedRefreshInterval(0.0f)
		, m_savedWindowStyle(0)
		, m_bInBorderlessFullscreen(false)
	{
		memset(&m_wpPrev, 0, sizeof(m_wpPrev));
		m_wpPrev.length = sizeof(m_wpPrev);

		HRESULT hrCom = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
		if (SUCCEEDED(hrCom))
		{
			m_bComInitialized = true;
		}

		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken = 0;
		if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr) == Gdiplus::Ok)
		{
			m_gdiplusToken = static_cast<uintptr_t>(gdiplusToken);
		}
	}

	CWindowsApplication::~CWindowsApplication()
	{
		if (m_hMenu)
		{
			::DestroyMenu(m_hMenu);
			m_hMenu = nullptr;
		}
		if (m_gdiplusToken != 0)
		{
			Gdiplus::GdiplusShutdown(static_cast<ULONG_PTR>(m_gdiplusToken));
			m_gdiplusToken = 0;
		}
		if (m_bComInitialized)
		{
			::CoUninitialize();
			m_bComInitialized = false;
		}
	}

	void CWindowsApplication::GameToClient(int& inout_x, int& inout_y, bool bInBackbuffer /*= true*/)
	{
		HWND hWnd = CGlobals::GetAppHWND();
		auto pWinDelegate = static_cast<RenderWindowDelegate*>(m_pRenderWindow);

		if (pWinDelegate && pWinDelegate->IsRotateScreen())
		{
			// Inverse of counter-clockwise rotation (render_x = h - 1 - win_y, render_y = win_x)
			const int renderX = inout_x;
			const int renderY = inout_y;
			inout_x = renderY;
			inout_y = std::max(0, pWinDelegate->m_window_height - 1 - renderX);
		}

		if (bInBackbuffer && hWnd && ::IsWindow(hWnd))
		{
			RECT rcClient = {};
			if (::GetClientRect(hWnd, &rcClient))
			{
				const int clientW = rcClient.right - rcClient.left;
				const int clientH = rcClient.bottom - rcClient.top;
				int backW = static_cast<int>(m_cfg.screenWidth);
				int backH = static_cast<int>(m_cfg.screenHeight);
				if (pWinDelegate && pWinDelegate->IsRotateScreen())
				{
					std::swap(backW, backH);
				}
				if (clientW > 0 && clientH > 0 && backW > 0 && backH > 0
					&& (clientW != backW || clientH != backH))
				{
					inout_x = static_cast<int>(std::lround(static_cast<double>(inout_x) * static_cast<double>(clientW) / static_cast<double>(backW)));
					inout_y = static_cast<int>(std::lround(static_cast<double>(inout_y) * static_cast<double>(clientH) / static_cast<double>(backH)));
				}
			}
		}
	}

	void CWindowsApplication::ClientToGame(int& inout_x, int& inout_y, bool bInBackbuffer /*= true*/)
	{
		HWND hWnd = CGlobals::GetAppHWND();
		auto pWinDelegate = static_cast<RenderWindowDelegate*>(m_pRenderWindow);

		if (bInBackbuffer && hWnd && ::IsWindow(hWnd))
		{
			RECT rcClient = {};
			if (::GetClientRect(hWnd, &rcClient))
			{
				const int clientW = rcClient.right - rcClient.left;
				const int clientH = rcClient.bottom - rcClient.top;
				int backW = static_cast<int>(m_cfg.screenWidth);
				int backH = static_cast<int>(m_cfg.screenHeight);
				if (pWinDelegate && pWinDelegate->IsRotateScreen())
				{
					std::swap(backW, backH);
				}
				if (clientW > 0 && clientH > 0 && backW > 0 && backH > 0
					&& (clientW != backW || clientH != backH))
				{
					inout_x = static_cast<int>(std::lround(static_cast<double>(inout_x) * static_cast<double>(backW) / static_cast<double>(clientW)));
					inout_y = static_cast<int>(std::lround(static_cast<double>(inout_y) * static_cast<double>(backH) / static_cast<double>(clientH)));
				}
			}
		}

		if (pWinDelegate && pWinDelegate->IsRotateScreen())
		{
			pWinDelegate->WindowXYToRenderXY(inout_x, inout_y, inout_x, inout_y);
		}
	}

	void CWindowsApplication::GetCursorPosition(int* pX, int* pY, bool bInBackbuffer /*= true*/)
	{
		int x = 0;
		int y = 0;
		if (IsTouchInputting() && CGUIRoot::GetInstance() && CGUIRoot::GetInstance()->GetMouse())
		{
			CGUIRoot::GetInstance()->GetMouse()->GetDeviceCursorPos(x, y);
		}
		else
		{
			POINT ptCursor = { 0, 0 };
			if (::GetCursorPos(&ptCursor))
			{
				HWND hWnd = CGlobals::GetAppHWND();
				if (hWnd && ::IsWindow(hWnd))
				{
					::ScreenToClient(hWnd, &ptCursor);
				}
				x = static_cast<int>(ptCursor.x);
				y = static_cast<int>(ptCursor.y);
				ClientToGame(x, y, bInBackbuffer);
			}
		}
		if (pX)
			*pX = x;
		if (pY)
			*pY = y;
	}

	void CWindowsApplication::SetRefreshTimer(float fTimeInterval, int nFrameRateControl /*= 0*/)
	{
		CParaEngineAppBase::SetRefreshTimer(fTimeInterval, nFrameRateControl);
	}

	bool CWindowsApplication::AppHasFocus()
	{
		if (!IsAppActive())
			return false;
		HWND hMain = GetMainWindow();
		if (!hMain || !::IsWindow(hMain))
			return m_bActive;
		HWND hFocus = ::GetFocus();
		for (int depth = 0; depth < 4 && hFocus != nullptr; ++depth)
		{
			if (hFocus == hMain)
				return true;
			hFocus = ::GetParent(hFocus);
		}
		HWND hFore = ::GetForegroundWindow();
		if (hFore == hMain || (hFore && ::IsChild(hFore, hMain)))
			return true;
		auto pWinDelegate = static_cast<RenderWindowDelegate*>(m_pRenderWindow);
		return pWinDelegate ? pWinDelegate->HasFocus() : m_bActive;
	}

	void CWindowsApplication::ActivateApp(bool bActivate)
	{
		const bool wasActive = m_bActive;
		CParaEngineAppBase::ActivateApp(bActivate);

		if (m_pGUIRoot.get())
		{
			if (m_pGUIRoot->IsActive() != bActivate)
			{
				if (bActivate)
					m_pGUIRoot->ActivateRoot();
				else
					m_pGUIRoot->InactivateRoot();
			}
		}

		if (wasActive != bActivate && m_bToggleSoundWhenNotFocused && m_bAudioEngineInitialized)
		{
			CAudioEngine2::GetInstance()->OnSwitch(bActivate);
		}

		if (wasActive != bActivate && m_bAutoLowerFrameRateWhenNotFocused)
		{
			const float fLowTimer = 1.0f / 20.0f;
			const float fCurrent = (GetRefreshTimer() <= 0.0f) ? IDEAL_FRAME_RATE : GetRefreshTimer();
			if (!bActivate)
			{
				if (fCurrent < fLowTimer)
				{
					m_fSavedRefreshInterval = GetRefreshTimer();
					SetRefreshTimer(fLowTimer, m_nFrameRateControl);
				}
			}
			else if (m_fSavedRefreshInterval != 0.0f)
			{
				SetRefreshTimer(m_fSavedRefreshInterval, m_nFrameRateControl);
				m_fSavedRefreshInterval = 0.0f;
			}
		}
	}

	bool CWindowsApplication::IsSlateMode()
	{
		return (::GetSystemMetrics(SM_CONVERTIBLESLATEMODE) == 0);
	}

	void CWindowsApplication::GetStats(string& output, DWORD dwFields)
	{
		if (dwFields == 0)
		{
			std::ostringstream oss;
			if (m_pRenderDevice)
			{
				const RHIDeviceCaps& caps = m_pRenderDevice->GetCaps();
				oss << "Backend: " << (caps.backendName ? caps.backendName : "Unknown") << "\n";
				oss << "Adapter: " << (caps.adapterName ? caps.adapterName : "Unknown") << "\n";
				oss << "FeatureLevel: 0x" << std::hex << caps.featureLevel << std::dec
					<< " (SM " << caps.GetShaderModelMajor() << ")\n";
				oss << "MaxSimultaneousRTs: " << caps.maxSimultaneousRTs
					<< ", MaxTextureSize: " << caps.maxTextureSize << "\n";
			}
			else
			{
				oss << "Backend: Null\n";
			}
			oss << "Resolution: " << m_cfg.screenWidth << "x" << m_cfg.screenHeight
				<< (m_cfg.isWindowed ? " (Windowed)" : " (Fullscreen)") << "\n";
			oss << "FPS: " << GetFPS();
			output = oss.str();
		}
		else
		{
			output = COSInfo::GetSystemInfoString((dwFields & 2) != 0);
		}
	}

	bool CWindowsApplication::WriteRegStr(const string& root_key, const string& sSubKey, const string& name, const string& value)
	{
		HKEY hKey = OpenOrCreateRegKey(root_key, sSubKey, KEY_SET_VALUE, true);
		if (!hKey)
		{
			OUTPUT_LOG("warning: WriteRegStr failed to open/create %s\\%s\n", root_key.c_str(), sSubKey.c_str());
			return false;
		}

		std::wstring wName = StringHelper::MultiByteToWideChar(name.c_str(), CP_UTF8);
		std::wstring wValue = StringHelper::MultiByteToWideChar(value.c_str(), CP_UTF8);
		const DWORD cbData = static_cast<DWORD>((wValue.size() + 1) * sizeof(wchar_t));
		LONG lRet = ::RegSetValueExW(
			hKey,
			wName.empty() ? nullptr : wName.c_str(),
			0,
			REG_SZ,
			reinterpret_cast<const BYTE*>(wValue.c_str()),
			cbData);
		::RegCloseKey(hKey);

		if (lRet != ERROR_SUCCESS)
		{
			OUTPUT_LOG("warning: WriteRegStr failed to set %s\\%s:%s (err=%ld)\n", root_key.c_str(), sSubKey.c_str(), name.c_str(), lRet);
			return false;
		}
		return true;
	}

	const char* CWindowsApplication::ReadRegStr(const string& root_key, const string& sSubKey, const string& name)
	{
		thread_local static std::string s_regStrResult;
		s_regStrResult.clear();

		HKEY hKey = OpenOrCreateRegKey(root_key, sSubKey, KEY_QUERY_VALUE, false);
		if (!hKey)
		{
			return nullptr;
		}

		std::wstring wName = StringHelper::MultiByteToWideChar(name.c_str(), CP_UTF8);
		DWORD dwType = 0;
		DWORD cbData = 0;
		LONG lRet = ::RegQueryValueExW(
			hKey,
			wName.empty() ? nullptr : wName.c_str(),
			nullptr,
			&dwType,
			nullptr,
			&cbData);
		if (lRet != ERROR_SUCCESS)
		{
			::RegCloseKey(hKey);
			// Legacy ParaEngine ReadRegStr returns "" when the key itself exists
			// and the unnamed default value ("") is queried but unset.
			if (name.empty() && lRet == ERROR_FILE_NOT_FOUND)
				return s_regStrResult.c_str();
			return nullptr;
		}

		if ((dwType == REG_SZ || dwType == REG_EXPAND_SZ) && cbData > 0)
		{
			std::vector<wchar_t> wBuf((cbData / sizeof(wchar_t)) + 2, L'\0');
			DWORD cbRead = cbData;
			lRet = ::RegQueryValueExW(
				hKey,
				wName.empty() ? nullptr : wName.c_str(),
				nullptr,
				&dwType,
				reinterpret_cast<LPBYTE>(wBuf.data()),
				&cbRead);
			::RegCloseKey(hKey);
			if (lRet != ERROR_SUCCESS)
				return nullptr;
			s_regStrResult = StringHelper::WideCharToMultiByte(wBuf.data(), CP_UTF8);
			return s_regStrResult.c_str();
		}
		else if (dwType == REG_DWORD && cbData >= sizeof(DWORD))
		{
			DWORD dwValue = 0;
			DWORD cbRead = sizeof(DWORD);
			lRet = ::RegQueryValueExW(
				hKey,
				wName.empty() ? nullptr : wName.c_str(),
				nullptr,
				&dwType,
				reinterpret_cast<LPBYTE>(&dwValue),
				&cbRead);
			::RegCloseKey(hKey);
			if (lRet != ERROR_SUCCESS)
				return nullptr;
			s_regStrResult = std::to_string(dwValue);
			return s_regStrResult.c_str();
		}

		::RegCloseKey(hKey);
		return s_regStrResult.c_str();
	}

	bool CWindowsApplication::WriteRegDWORD(const string& root_key, const string& sSubKey, const string& name, DWORD value)
	{
		HKEY hKey = OpenOrCreateRegKey(root_key, sSubKey, KEY_SET_VALUE, true);
		if (!hKey)
		{
			OUTPUT_LOG("warning: WriteRegDWORD failed to open/create %s\\%s\n", root_key.c_str(), sSubKey.c_str());
			return false;
		}

		std::wstring wName = StringHelper::MultiByteToWideChar(name.c_str(), CP_UTF8);
		LONG lRet = ::RegSetValueExW(
			hKey,
			wName.empty() ? nullptr : wName.c_str(),
			0,
			REG_DWORD,
			reinterpret_cast<const BYTE*>(&value),
			sizeof(DWORD));
		::RegCloseKey(hKey);

		if (lRet != ERROR_SUCCESS)
		{
			OUTPUT_LOG("warning: WriteRegDWORD failed to set %s\\%s:%s (err=%ld)\n", root_key.c_str(), sSubKey.c_str(), name.c_str(), lRet);
			return false;
		}
		return true;
	}

	DWORD CWindowsApplication::ReadRegDWORD(const string& root_key, const string& sSubKey, const string& name)
	{
		HKEY hKey = OpenOrCreateRegKey(root_key, sSubKey, KEY_QUERY_VALUE, false);
		if (!hKey)
		{
			return 0;
		}

		std::wstring wName = StringHelper::MultiByteToWideChar(name.c_str(), CP_UTF8);
		DWORD dwType = 0;
		DWORD dwValue = 0;
		DWORD cbData = sizeof(DWORD);
		LONG lRet = ::RegQueryValueExW(
			hKey,
			wName.empty() ? nullptr : wName.c_str(),
			nullptr,
			&dwType,
			reinterpret_cast<LPBYTE>(&dwValue),
			&cbData);
		::RegCloseKey(hKey);

		if (lRet != ERROR_SUCCESS || dwType != REG_DWORD)
		{
			return 0;
		}
		return dwValue;
	}

	void CWindowsApplication::SetAutoLowerFrameRateWhenNotFocused(bool bEnabled)
	{
		m_bAutoLowerFrameRateWhenNotFocused = bEnabled;
	}

	bool CWindowsApplication::GetAutoLowerFrameRateWhenNotFocused()
	{
		return m_bAutoLowerFrameRateWhenNotFocused;
	}

	void CWindowsApplication::SetToggleSoundWhenNotFocused(bool bEnabled)
	{
		m_bToggleSoundWhenNotFocused = bEnabled;
	}

	bool CWindowsApplication::GetToggleSoundWhenNotFocused()
	{
		return m_bToggleSoundWhenNotFocused;
	}

	void CWindowsApplication::SetMainWindow(HWND hWnd, bool bIsExternalWindow /*= true*/)
	{
		m_hExternalWnd = hWnd;
		m_bIsExternalWindow = bIsExternalWindow;
		if (bIsExternalWindow && hWnd)
		{
			m_dwCoreUsage &= ~PE_USAGE_OWN_WINDOW;
		}
		else
		{
			m_dwCoreUsage |= PE_USAGE_OWN_WINDOW;
		}
	}

	HWND CWindowsApplication::GetMainWindow()
	{
		if (m_pRenderWindow)
		{
			HWND hWnd = reinterpret_cast<HWND>(m_pRenderWindow->GetNativeHandle());
			if (hWnd)
				return hWnd;
		}
		if (m_cfg.renderWindow)
		{
			HWND hWnd = reinterpret_cast<HWND>(m_cfg.renderWindow->GetNativeHandle());
			if (hWnd)
				return hWnd;
		}
		return m_hExternalWnd;
	}

	HINSTANCE CWindowsApplication::GetModuleHandle()
	{
		return m_hInst ? m_hInst : ::GetModuleHandleW(nullptr);
	}

	DWORD CWindowsApplication::GetCoreUsage()
	{
		return m_dwCoreUsage;
	}

	void CWindowsApplication::SetCoreUsage(DWORD dwUsage)
	{
		m_dwCoreUsage = (m_dwCoreUsage & 0xfffffff0u) | dwUsage;
	}

	bool CWindowsApplication::PostWinThreadMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		HWND hWnd = GetMainWindow();
		if (hWnd && ::IsWindow(hWnd))
		{
			return ::PostMessageW(hWnd, uMsg, wParam, lParam) != FALSE;
		}
		if (m_dwWinThreadID != 0)
		{
			return ::PostThreadMessageW(m_dwWinThreadID, uMsg, wParam, lParam) != FALSE;
		}
		return false;
	}

	void CWindowsApplication::EnablePassiveRendering(bool bEnable)
	{
		m_bPassiveRendering = bEnable;
		setRenderEnabled(!bEnable);
	}

	bool CWindowsApplication::IsPassiveRenderingEnabled()
	{
		return m_bPassiveRendering;
	}

	void CWindowsApplication::SetAllowWindowClosing(bool bAllowClosing)
	{
		m_bAllowWindowClosing = bAllowClosing;
	}

	bool CWindowsApplication::IsWindowClosingAllowed()
	{
		return m_bAllowWindowClosing;
	}

	bool CWindowsApplication::HasNewConfig()
	{
		return m_bHasNewConfig;
	}

	void CWindowsApplication::SetHasNewConfig(bool bHasNewConfig)
	{
		m_bHasNewConfig = bHasNewConfig;
	}

	void CWindowsApplication::GetWindowCreationSize(int* pWidth, int* pHeight)
	{
		if (pWidth)
			*pWidth = m_nCreationWidth;
		if (pHeight)
			*pHeight = m_nCreationHeight;
	}

	void CWindowsApplication::SetMinUIResolution(int nWidth, int nHeight, bool bAutoUIScaling /*= true*/)
	{
		if (m_pGUIRoot.get())
			m_pGUIRoot->SetMinimumScreenSize(nWidth, nHeight, bAutoUIScaling);
	}

	void CWindowsApplication::SetMaxUIResolution(int nWidth, int nHeight, bool bAutoUIScaling /*= true*/)
	{
		if (m_pGUIRoot.get())
			m_pGUIRoot->SetMaximumScreenSize(nWidth, nHeight, bAutoUIScaling);
	}

	void CWindowsApplication::SetIgnoreWindowSizeChange(bool bIgnoreSizeChange)
	{
		m_bIgnoreSizeChange = bIgnoreSizeChange;
	}

	bool CWindowsApplication::GetIgnoreWindowSizeChange()
	{
		return m_bIgnoreSizeChange;
	}

	void CWindowsApplication::SetMultiSampleType(int nType)
	{
		if (m_nMultiSampleType != nType)
		{
			m_nMultiSampleType = nType;
			m_cfg.msaaSamples = static_cast<uint32_t>((std::max)(0, nType));
			m_bSizeChanged = true;
		}
	}

	int CWindowsApplication::GetMultiSampleType()
	{
		return m_nMultiSampleType;
	}

	void CWindowsApplication::SetMultiSampleQuality(int nType)
	{
		if (m_nMultiSampleQuality != nType)
		{
			m_nMultiSampleQuality = nType;
			m_bSizeChanged = true;
		}
	}

	int CWindowsApplication::GetMultiSampleQuality()
	{
		return m_nMultiSampleQuality;
	}

	void CWindowsApplication::ShowMenu(bool bShow)
	{
		HWND hWnd = CGlobals::GetAppHWND();
		if (!hWnd || !::IsWindow(hWnd))
			return;

		if (bShow)
		{
			if (!m_hMenu)
			{
				m_hMenu = ::LoadMenuW(GetModuleHandle(), MAKEINTRESOURCEW(IDR_MENU));
			}
			if (m_hMenu)
			{
				::SetMenu(hWnd, m_hMenu);
			}
		}
		else
		{
			::SetMenu(hWnd, nullptr);
		}
	}

	bool CWindowsApplication::InitApp(IRenderWindow* pWindow, const char* sCommandLine)
	{
		m_dwWinThreadID = ::GetCurrentThreadId();
		if (!m_hInst)
		{
			m_hInst = g_hAppInstance ? g_hAppInstance : ::GetModuleHandleW(nullptr);
		}

#ifdef PARA_WIN_HAS_D3D12_CTX
		RHIBackendRegistry::GetInstance().RegisterBackend(ERHIBackend::D3D12, "D3D12", CreateRenderContextD3D12, 100);
#endif
#ifdef PARA_WIN_HAS_VULKAN_CTX
		RHIBackendRegistry::GetInstance().RegisterBackend(ERHIBackend::Vulkan, "Vulkan", CreateRenderContextVulkan, 95);
#endif
#ifdef PARA_RHI_HAS_D3D11
		RHIBackendRegistry::GetInstance().RegisterBackend(ERHIBackend::D3D11, "D3D11", CreateRenderContextD3D11, 90);
#endif
#ifdef PARA_WIN_HAS_WEBGPU_CTX
		RHIBackendRegistry::GetInstance().RegisterBackend(ERHIBackend::WebGPU, "WebGPU", CreateRenderContextWebGPU, 60);
#endif
#ifdef PARA_RHI_HAS_OPENGL
		RHIBackendRegistry::GetInstance().RegisterBackend(ERHIBackend::OpenGL, "OpenGL", CreateRenderContextWGL, 50);
#endif

		// Locate working directory first so ParaEngine.sig, VerifyCommandLine, and config/config.txt resolve relative paths properly
		FindParaEngineDirectory();
		std::string verifiedCmd;
		VerifyCommandLine(sCommandLine, verifiedCmd);
		SetAppCommandLine(verifiedCmd.c_str());

		BootStrapAndLoadConfig();
		LoadAndApplySettings();

		m_nCreationWidth = static_cast<int>(m_cfg.screenWidth);
		m_nCreationHeight = static_cast<int>(m_cfg.screenHeight);

		ERHIBackend preferred = ERHIBackend::Null;
		if (!Is3DRenderingEnabled())
		{
			preferred = ERHIBackend::Null;
			setRenderEnabled(false);
		}
		else
		{
			preferred = RHIBackendRegistry::GetInstance().ResolvePreferredBackend(verifiedCmd.c_str());
		}
		RHIBackendRegistry::GetInstance().SetPreferredBackend(preferred);
		m_cfg.preferredBackend = preferred;

		HWND hParentWnd = nullptr;
		const char* sParentWnd = GetAppCommandLineByParam("parentwindow", nullptr);
		if (sParentWnd && sParentWnd[0] != '\0')
		{
			uintptr_t rawVal = static_cast<uintptr_t>(std::strtoull(sParentWnd, nullptr, 0));
			if (rawVal != 0 && ::IsWindow(reinterpret_cast<HWND>(rawVal)))
			{
				hParentWnd = reinterpret_cast<HWND>(rawVal);
				m_dwCoreUsage = (m_dwCoreUsage & ~0xfu) | PE_USAGE_WEB_BROWSER;
			}
		}

		auto pWinDelegate = static_cast<RenderWindowDelegate*>(pWindow);
		m_pRenderWindow = pWindow;
		m_cfg.renderWindow = pWinDelegate;

		if (pWinDelegate && preferred != ERHIBackend::Null && Is3DRenderingEnabled())
		{
			if (m_bIsExternalWindow && m_hExternalWnd && ::IsWindow(m_hExternalWnd))
			{
				pWinDelegate->AttachExternalWindow(m_hExternalWnd);
				if (pWinDelegate->GetWidth() > 0 && pWinDelegate->GetHeight() > 0)
				{
					m_cfg.screenWidth = pWinDelegate->GetWidth();
					m_cfg.screenHeight = pWinDelegate->GetHeight();
				}
			}
			else if (!pWinDelegate->GetHandle())
			{
				pWinDelegate->Create(m_hInst, m_cfg.screenWidth, m_cfg.screenHeight, hParentWnd);
			}

			HWND hCreated = pWinDelegate->GetHandle();
			if (hCreated)
			{
				if (!m_cfg.isWindowed && !pWinDelegate->IsChildWindow())
				{
					ApplyFullscreenWindowStyle(hCreated, true);
				}
				else if (!pWinDelegate->IsChildWindow())
				{
					CDynamicAttributeField* pMaxField = ParaEngineSettings::GetSingleton().GetDynamicField("IsWindowMaximized");
					if (pMaxField && static_cast<bool>(*pMaxField))
					{
						::ShowWindow(hCreated, SW_SHOWMAXIMIZED);
					}
				}
			}
		}

		const char* sConsole = GetAppCommandLineByParam("console", nullptr);
		if (sConsole && strcmp(sConsole, "true") == 0)
		{
			RedirectIOToConsole();
		}

		return CParaEngineAppBase::InitApp(pWindow, verifiedCmd.c_str());
	}

	void CWindowsApplication::Exit(int nReturnCode /*= 0*/)
	{
		CParaEngineAppBase::Exit(nReturnCode);
		auto pWinDelegate = static_cast<RenderWindowDelegate*>(m_pRenderWindow);
		if (pWinDelegate && pWinDelegate->GetHandle())
		{
			pWinDelegate->ForceQuit();
		}
		MainLoopStop();
	}

	int CWindowsApplication::Run(HINSTANCE hInstance)
	{
		(void)hInstance;
		NextLoop(1, &CWindowsApplication::handle_mainloop_timer, this);
		MainLoopRun();
		FinalCleanup();

		return GetReturnCode();
	}

	void CWindowsApplication::handle_mainloop_timer(const boost::system::error_code& err)
	{
		if (err)
			return;

		auto pWindow = static_cast<RenderWindowDelegate*>(m_pRenderWindow);
		const bool bHasNativeWindow = (pWindow != nullptr && pWindow->GetHandle() != nullptr);

		if (GetAppState() == PEAppState_Exiting || HasClosingRequest() || (bHasNativeWindow && pWindow->ShouldClose()))
		{
			MainLoopStop();
			return;
		}

		if (bHasNativeWindow)
		{
			pWindow->PollEvents();
		}

		if (GetAppState() == PEAppState_Exiting || HasClosingRequest() || (bHasNativeWindow && pWindow->ShouldClose()))
		{
			MainLoopStop();
			return;
		}

		if (bHasNativeWindow && Is3DRenderingEnabled())
		{
			UpdateScreenDevice();
		}

		this->DoWork();

		if (GetAppState() == PEAppState_Exiting || HasClosingRequest() || (bHasNativeWindow && pWindow->ShouldClose()))
		{
			MainLoopStop();
			return;
		}

		int nextWaitMs = 10;
		if (!IsAppActive() && m_bAutoLowerFrameRateWhenNotFocused)
		{
			nextWaitMs = 50;
		}
		else if (m_profileUnlimitedFPS)
		{
			nextWaitMs = 0;
		}
		else if (GetRefreshTimer() <= 0.0f)
		{
			nextWaitMs = 1;
		}
		else
		{
			double fNextInterval = 0.01;
			CalculateRenderTime(&fNextInterval);
			nextWaitMs = std::clamp(static_cast<int>(fNextInterval * 1000.0), 1, 100);
		}
		NextLoop(nextWaitMs, &CWindowsApplication::handle_mainloop_timer, this);
	}

	void CWindowsApplication::WriteConfigFile(const char* FileName)
	{
		std::string sFileName;
		if (FileName == nullptr || FileName[0] == '\0')
			sFileName = "config/config.txt";
		else
			sFileName = FileName;

		{
			DWORD dwAttrs = ::GetFileAttributesA(sFileName.c_str());
			if (dwAttrs != INVALID_FILE_ATTRIBUTES && (dwAttrs & FILE_ATTRIBUTE_READONLY))
			{
				::SetFileAttributesA(sFileName.c_str(), dwAttrs & (~FILE_ATTRIBUTE_READONLY));
			}
		}

		CParaFile file;
		if (!file.CreateNewFile(sFileName.c_str()))
		{
			OUTPUT_LOG("failed creating file %s\r\n", sFileName.c_str());
		}
		file.WriteFormated("-- ParaEngine startup settings.\n");
		file.WriteFormated("-- Auto generated by ParaEngine %s\n", ParaEngineInfo::CParaEngineInfo::GetVersion().c_str());

		file.WriteFormated("-- MultiSampleType: hardware fullscreen MultiSample AntiAliasing(AA): 0 to disable;2 to enable;4 is better\n");
		file.WriteFormated("-- MultiSampleQuality: 0 default quality, 1 for higher quality. the higher value, the more quality\n");
		file.WriteFormated("-- ScriptEditor: default is editor/notepad.exe, UltraEdit is another good choice if one has it.\n");
		file.WriteFormated("-- GameEffectSet: initial game effect set, 0 is recommended. possible value is [1024, 0,1,2,-1,-2]. 1024 means FF, otherwise the larger the better.\n");
		file.WriteFormated("-- language: possible values are enUS or zhCN\n");

		ParaEngineSettings& settings = ParaEngineSettings::GetSingleton();
		CVariable value;

		value = !m_cfg.isWindowed;
		settings.SetDynamicField("StartFullscreen", value);

		value = IsWindowMaximized();
		settings.SetDynamicField("IsWindowMaximized", value);

		value = m_nMultiSampleType;
		settings.SetDynamicField("MultiSampleType", value);

		value = settings.GetScriptEditor();
		settings.SetDynamicField("ScriptEditor", value);

		value = settings.GetGameEffectSet();
		settings.SetDynamicField("GameEffectSet", value);

		value = settings.GetMouseInverse();
		settings.SetDynamicField("InverseMouse", value);

		value = settings.GetLocale();
		settings.SetDynamicField("language", value);

		value = settings.GetTextureLOD();
		settings.SetDynamicField("TextureLOD", value);

		string field_text;
		settings.SaveDynamicFieldsToString(field_text);
		file.WriteString(field_text);

		OUTPUT_LOG("ParaEngine config file is saved to %s\r\n", sFileName.c_str());
	}

	void CWindowsApplication::GetScreenResolution(Vector2* pOut)
	{
		if (pOut)
		{
			GetResolution(&(pOut->x), &(pOut->y));
		}
	}

	void CWindowsApplication::SetScreenResolution(const Vector2& vSize)
	{
		SetResolution(vSize.x, vSize.y);
		UpdateScreenMode();
	}

	void CWindowsApplication::GetResolution(float* pX, float* pY)
	{
		if (pX)
			*pX = static_cast<float>(m_cfg.screenWidth);
		if (pY)
			*pY = static_cast<float>(m_cfg.screenHeight);
	}

	void CWindowsApplication::SetResolution(float x, float y)
	{
		uint32_t _x = static_cast<uint32_t>(std::max(1.0f, x));
		uint32_t _y = static_cast<uint32_t>(std::max(1.0f, y));

		if (m_cfg.screenWidth != _x || m_cfg.screenHeight != _y)
		{
			m_bSizeChanged = true;
			m_cfg.screenWidth = _x;
			m_cfg.screenHeight = _y;
			if (m_cfg.isWindowed)
			{
				m_nCreationWidth = static_cast<int>(_x);
				m_nCreationHeight = static_cast<int>(_y);
			}

			ParaEngineSettings& settings = ParaEngineSettings::GetSingleton();
			CVariable value;
			value = static_cast<int>(_x);
			settings.SetDynamicField("ScreenWidth", value);
			value = static_cast<int>(_y);
			settings.SetDynamicField("ScreenHeight", value);
		}
	}

	void CWindowsApplication::LoadAndApplySettings()
	{
		ParaEngineSettings& settings = ParaEngineSettings::GetSingleton();

		CDynamicAttributeField* pField = nullptr;
		if ((pField = settings.GetDynamicField("StartFullscreen")))
			m_cfg.isWindowed = !static_cast<bool>(*pField);
		else
			m_cfg.isWindowed = true;

		if ((pField = settings.GetDynamicField("ScreenWidth")))
			m_cfg.screenWidth = static_cast<int>(*pField);
		else
			m_cfg.screenWidth = 1020;

		if ((pField = settings.GetDynamicField("ScreenHeight")))
			m_cfg.screenHeight = static_cast<int>(*pField);
		else
			m_cfg.screenHeight = 680;

		if ((pField = settings.GetDynamicField("MultiSampleType")))
			SetMultiSampleType(static_cast<int>(*pField));

		if ((pField = settings.GetDynamicField("ScriptEditor")))
			settings.SetScriptEditor((const std::string&)(*pField));

		if ((pField = settings.GetDynamicField("InverseMouse")))
			settings.SetMouseInverse(static_cast<bool>(*pField));

		if ((pField = settings.GetDynamicField("language")))
			settings.SetLocale((const char*)(*pField));

		if ((pField = settings.GetDynamicField("TextureLOD")))
			settings.SetTextureLOD(static_cast<int>(*pField));

		const char* sIsFullScreen = GetAppCommandLineByParam("fullscreen", nullptr);
		if (sIsFullScreen)
			m_cfg.isWindowed = (strcmp("true", sIsFullScreen) != 0);

		const char* sWidth = GetAppCommandLineByParam("width", nullptr);
		if (sWidth && std::atoi(sWidth) > 0)
			m_cfg.screenWidth = static_cast<uint32_t>(std::atoi(sWidth));

		const char* sHeight = GetAppCommandLineByParam("height", nullptr);
		if (sHeight && std::atoi(sHeight) > 0)
			m_cfg.screenHeight = static_cast<uint32_t>(std::atoi(sHeight));
	}

	bool CWindowsApplication::UpdateScreenMode()
	{
		m_bUpdateScreenDevice = true;
		return true;
	}

	void CWindowsApplication::SetFullScreenMode(bool bFullscreen)
	{
		SetWindowedMode(!bFullscreen);
	}

	bool CWindowsApplication::IsFullScreenMode()
	{
		return !m_cfg.isWindowed;
	}

	bool CWindowsApplication::SetWindowedMode(bool bWindowed)
	{
		if (IsWindowedMode() == bWindowed)
			return true;

		m_cfg.isWindowed = bWindowed;
		m_bModeChanged = true;
		return UpdateScreenMode();
	}

	bool CWindowsApplication::IsWindowedMode()
	{
		return m_cfg.isWindowed;
	}

	void CWindowsApplication::SetWindowMaximized(bool isMaximized)
	{
		HWND hWnd = CGlobals::GetAppHWND();
		if (hWnd && ::IsWindow(hWnd))
		{
			::ShowWindow(hWnd, isMaximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
		}
	}

	bool CWindowsApplication::IsWindowMaximized()
	{
		HWND hWnd = CGlobals::GetAppHWND();
		return (hWnd && ::IsWindow(hWnd)) ? (::IsZoomed(hWnd) != FALSE) : false;
	}

	void CWindowsApplication::GetVisibleSize(Vector2* pOut)
	{
		if (!pOut)
			return;
		HWND hWnd = CGlobals::GetAppHWND();
		if (hWnd && ::IsWindow(hWnd))
		{
			RECT rc = {};
			if (::GetClientRect(hWnd, &rc))
			{
				int w = rc.right - rc.left;
				int h = rc.bottom - rc.top;
				if (w > 0 && h > 0)
				{
					if (IsRotateScreen())
					{
						std::swap(w, h);
					}
					*pOut = Vector2(static_cast<float>(w), static_cast<float>(h));
					return;
				}
			}
		}
		*pOut = Vector2(static_cast<float>(m_cfg.screenWidth), static_cast<float>(m_cfg.screenHeight));
	}

	void CWindowsApplication::BringWindowToTop()
	{
		if (!IsFullScreenMode())
		{
			HWND hWnd = CGlobals::GetAppHWND();
			if (!hWnd || !::IsWindow(hWnd))
				return;

			if ((GetCoreUsage() & PE_USAGE_WEB_BROWSER) != 0)
			{
				HWND hTop = ::GetAncestor(hWnd, GA_ROOT);
				BringHwndToForegroundForce(hTop ? hTop : hWnd);
			}
			else
			{
				BringHwndToForegroundForce(hWnd);
			}
		}
	}

	void CWindowsApplication::SetWindowText(const char* pChar)
	{
		if (!pChar)
			return;
		std::wstring s = StringHelper::MultiByteToWideChar(pChar, CP_UTF8);
		HWND hWnd = CGlobals::GetAppHWND();
		if (hWnd && ::IsWindow(hWnd))
			::SetWindowTextW(hWnd, s.c_str());
	}

	const char* CWindowsApplication::GetWindowText()
	{
		thread_local static std::string s_title;
		s_title.clear();
		HWND hWnd = CGlobals::GetAppHWND();
		if (hWnd && ::IsWindow(hWnd))
		{
			WCHAR wTitle[512] = { 0 };
			::GetWindowTextW(hWnd, wTitle, 511);
			s_title = StringHelper::WideCharToMultiByte(wTitle, CP_UTF8);
		}
		return s_title.c_str();
	}

	void CWindowsApplication::ApplyFullscreenWindowStyle(HWND hWnd, bool bFullscreen)
	{
		if (!hWnd || !::IsWindow(hWnd))
			return;

		auto pWinDelegate = static_cast<RenderWindowDelegate*>(m_pRenderWindow);
		if (pWinDelegate && pWinDelegate->IsChildWindow())
			return;

		const bool prevIgnore = m_bIgnoreSizeChange;
		m_bIgnoreSizeChange = true;

		if (bFullscreen && !m_bInBorderlessFullscreen)
		{
			m_savedWindowStyle = ::GetWindowLongPtrW(hWnd, GWL_STYLE);
			m_wpPrev.length = sizeof(m_wpPrev);
			::GetWindowPlacement(hWnd, &m_wpPrev);

			MONITORINFO mi = {};
			mi.cbSize = sizeof(mi);
			HMONITOR hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
			if (::GetMonitorInfoW(hMonitor, &mi))
			{
				const int monW = mi.rcMonitor.right - mi.rcMonitor.left;
				const int monH = mi.rcMonitor.bottom - mi.rcMonitor.top;
				::SetWindowLongPtrW(hWnd, GWL_STYLE, (m_savedWindowStyle & ~(WS_OVERLAPPEDWINDOW)) | WS_POPUP | WS_VISIBLE);
				::SetWindowPos(
					hWnd,
					HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					monW,
					monH,
					SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
				m_cfg.screenWidth = static_cast<uint32_t>(std::max(1, monW));
				m_cfg.screenHeight = static_cast<uint32_t>(std::max(1, monH));
				if (pWinDelegate)
				{
					pWinDelegate->SetWindowedFlag(false);
					pWinDelegate->SetWindowSize(monW, monH);
				}
				m_bInBorderlessFullscreen = true;
			}
		}
		else if (!bFullscreen && m_bInBorderlessFullscreen)
		{
			LONG_PTR restoreStyle = (m_savedWindowStyle != 0) ? m_savedWindowStyle : (WS_OVERLAPPEDWINDOW | WS_VISIBLE);
			::SetWindowLongPtrW(hWnd, GWL_STYLE, restoreStyle);
			if (m_wpPrev.length == sizeof(m_wpPrev) && m_wpPrev.rcNormalPosition.right > m_wpPrev.rcNormalPosition.left)
			{
				::SetWindowPlacement(hWnd, &m_wpPrev);
			}
			::SetWindowPos(
				hWnd,
				nullptr,
				0,
				0,
				0,
				0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
			if (pWinDelegate)
			{
				pWinDelegate->SetWindowedFlag(true);
				int targetW = (m_nCreationWidth > 0) ? m_nCreationWidth : 1020;
				int targetH = (m_nCreationHeight > 0) ? m_nCreationHeight : 680;
				m_cfg.screenWidth = static_cast<uint32_t>(targetW);
				m_cfg.screenHeight = static_cast<uint32_t>(targetH);
				pWinDelegate->SetSize(targetW, targetH);
			}
			m_bInBorderlessFullscreen = false;
		}

		m_bIgnoreSizeChange = prevIgnore;
	}

	bool CWindowsApplication::UpdateViewPort()
	{
		if (m_pRenderDevice)
		{
			auto rc = m_pRenderDevice->GetViewport();
			if (rc.Width != m_cfg.screenWidth || rc.Height != m_cfg.screenHeight)
			{
				rc.Width = m_cfg.screenWidth;
				rc.Height = m_cfg.screenHeight;
				return m_pRenderDevice->SetViewport(rc);
			}
		}
		return false;
	}

	bool CWindowsApplication::UpdateScreenDevice()
	{
		if (m_bUpdateScreenDevice && m_pRenderWindow)
		{
			OUTPUT_LOG("update screen device to (%u, %u) windowed: %s\n", m_cfg.screenWidth, m_cfg.screenHeight, m_cfg.isWindowed ? "true" : "false");
			m_bUpdateScreenDevice = false;

			auto pWinDelegate = static_cast<RenderWindowDelegate*>(m_pRenderWindow);
			HWND hWnd = pWinDelegate->GetHandle();
			bool bReset = false;

			if (m_bModeChanged && hWnd)
			{
				ApplyFullscreenWindowStyle(hWnd, !m_cfg.isWindowed);
				m_bModeChanged = false;
				bReset = true;
			}

			if (m_bSizeChanged)
			{
				if (m_cfg.isWindowed && !pWinDelegate->IsChildWindow())
				{
					int width = static_cast<int>(m_cfg.screenWidth);
					int height = static_cast<int>(m_cfg.screenHeight);
					if (IsRotateScreen())
					{
						std::swap(width, height);
					}
					const bool prevIgnore = m_bIgnoreSizeChange;
					m_bIgnoreSizeChange = true;
					pWinDelegate->SetSize(width, height);
					m_bIgnoreSizeChange = prevIgnore;
				}
				bReset = true;
				m_bSizeChanged = false;
			}

			if (bReset)
			{
				if (!m_pRenderContext || !m_pRenderDevice)
					return false;

				InvalidateDeviceObjects();
				m_pRenderContext->ResetDevice(m_pRenderDevice, m_cfg);
				auto hr = RestoreDeviceObjects();
				if (FAILED(hr))
				{
					OUTPUT_LOG("reset render device failed because RestoreDeviceObjects failed: %d\n", hr);
					InvalidateDeviceObjects();
					return false;
				}
				if (m_pGUIRoot.get())
				{
					m_pGUIRoot->SetUIScale(1.f, 1.f, true, true);
				}
				return UpdateViewPort();
			}
		}
		return true;
	}

	void CWindowsApplication::FixWindowSize(bool fixed)
	{
		auto pWinDelegate = static_cast<RenderWindowDelegate*>(m_pRenderWindow);
		if (IsWindowedMode() && pWinDelegate)
		{
			HWND hwnd = pWinDelegate->GetHandle();
			if (!hwnd || pWinDelegate->IsChildWindow())
				return;

			LONG_PTR dwWindowStyle = ::GetWindowLongPtrW(hwnd, GWL_STYLE);
			if (fixed)
			{
				dwWindowStyle &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
			}
			else
			{
				dwWindowStyle |= (WS_THICKFRAME | WS_MAXIMIZEBOX);
			}
			::SetWindowLongPtrW(hwnd, GWL_STYLE, dwWindowStyle);
			::SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}

	void CWindowsApplication::ShowWindow(bool bShow)
	{
		if (!IsFullScreenMode())
		{
			if ((GetCoreUsage() & PE_USAGE_WEB_BROWSER) != 0)
			{
				OUTPUT_LOG("ShowWindow doesn't know how to respond in web browser mode!\n");
			}
			else
			{
				HWND hWnd = CGlobals::GetAppHWND();
				if (hWnd && ::IsWindow(hWnd))
				{
					::ShowWindow(hWnd, bShow ? SW_SHOW : SW_HIDE);
				}
			}
		}
	}

	void CWindowsApplication::SetLandscapeMode(std::string landscapeMode)
	{
		m_sLandscapeMode = landscapeMode;
		auto pWinDelegate = static_cast<RenderWindowDelegate*>(m_pRenderWindow);
		if (!pWinDelegate)
			return;
		if (landscapeMode == "on")
		{
			pWinDelegate->SetScreenOrientation(RenderWindowDelegate::s_screen_orientation_landscape);
		}
		else if (landscapeMode == "off")
		{
			pWinDelegate->SetScreenOrientation(RenderWindowDelegate::s_screen_orientation_portrait);
		}
		else
		{
			pWinDelegate->SetScreenOrientation(RenderWindowDelegate::s_screen_orientation_auto);
		}
	}

	std::string CWindowsApplication::GetLandscapeMode()
	{
		auto pWinDelegate = static_cast<RenderWindowDelegate*>(m_pRenderWindow);
		if (!pWinDelegate)
			return m_sLandscapeMode.empty() ? "auto" : m_sLandscapeMode;
		auto screen_orientation = pWinDelegate->GetScreenOrientation();
		if (screen_orientation == RenderWindowDelegate::s_screen_orientation_landscape)
		{
			return "on";
		}
		else if (screen_orientation == RenderWindowDelegate::s_screen_orientation_portrait)
		{
			return "off";
		}
		else
		{
			return "auto";
		}
	}

	bool CWindowsApplication::IsRotateScreen()
	{
		auto pWinDelegate = static_cast<RenderWindowDelegate*>(m_pRenderWindow);
		return pWinDelegate ? pWinDelegate->IsRotateScreen() : false;
	}
} // end namespace
