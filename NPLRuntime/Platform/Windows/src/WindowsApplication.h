#pragma once
#include "ParaEngineAppBase.h"
#include "Core/MainLoopBase.h"
#include "Render/IRenderContext.h"

namespace ParaEngine
{
	class CWindowsApplication : public CParaEngineAppBase, public MainLoopBase
	{
	public:
		CWindowsApplication(HINSTANCE hInstance);
		virtual ~CWindowsApplication();

		virtual void GameToClient(int& inout_x, int& inout_y, bool bInBackbuffer = true) override;
		virtual void ClientToGame(int& inout_x, int& inout_y, bool bInBackbuffer = true) override;
		virtual void GetCursorPosition(int* pX, int* pY, bool bInBackbuffer = true) override;

		virtual void SetRefreshTimer(float fTimeInterval, int nFrameRateControl = 0) override;

		virtual bool AppHasFocus() override;
		virtual void ActivateApp(bool bActivate) override;
		virtual bool IsSlateMode() override;

		virtual void GetStats(string& output, DWORD dwFields) override;

		virtual bool WriteRegStr(const string& root_key, const string& sSubKey, const string& name, const string& value) override;
		virtual const char* ReadRegStr(const string& root_key, const string& sSubKey, const string& name) override;
		virtual bool WriteRegDWORD(const string& root_key, const string& sSubKey, const string& name, DWORD value) override;
		virtual DWORD ReadRegDWORD(const string& root_key, const string& sSubKey, const string& name) override;

		virtual void SetAutoLowerFrameRateWhenNotFocused(bool bEnabled) override;
		virtual bool GetAutoLowerFrameRateWhenNotFocused() override;
		virtual void SetToggleSoundWhenNotFocused(bool bEnabled) override;
		virtual bool GetToggleSoundWhenNotFocused() override;

		virtual int Run(HINSTANCE hInstance) override;
		virtual bool InitApp(IRenderWindow* pWindow, const char* sCommandLine = nullptr) override;
		virtual void Exit(int nReturnCode = 0) override;

		virtual void SetMainWindow(HWND hWnd, bool bIsExternalWindow = true) override;
		virtual HWND GetMainWindow() override;
		virtual HINSTANCE GetModuleHandle() override;
		virtual DWORD GetCoreUsage() override;
		virtual void SetCoreUsage(DWORD dwUsage) override;
		virtual bool PostWinThreadMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

		virtual void EnablePassiveRendering(bool bEnable) override;
		virtual bool IsPassiveRenderingEnabled() override;

		virtual void SetAllowWindowClosing(bool bAllowClosing) override;
		virtual bool IsWindowClosingAllowed() override;

		virtual bool HasNewConfig() override;
		virtual void SetHasNewConfig(bool bHasNewConfig) override;
		virtual void GetWindowCreationSize(int* pWidth, int* pHeight) override;
		virtual void SetMinUIResolution(int nWidth, int nHeight, bool bAutoUIScaling = true) override;
		virtual void SetMaxUIResolution(int nWidth, int nHeight, bool bAutoUIScaling = true) override;
		virtual void SetIgnoreWindowSizeChange(bool bIgnoreSizeChange) override;
		virtual bool GetIgnoreWindowSizeChange() override;

		virtual void SetMultiSampleType(int nType) override;
		virtual int GetMultiSampleType() override;

		virtual void WriteConfigFile(const char* sFileName) override;

		virtual void GetScreenResolution(Vector2* pOut) override;
		virtual void SetScreenResolution(const Vector2& vSize) override;
		virtual void GetResolution(float* pX, float* pY) override;
		virtual void SetResolution(float x, float y) override;

		virtual bool UpdateScreenMode() override;

		virtual void SetFullScreenMode(bool bFullscreen) override;
		virtual bool IsFullScreenMode() override;
		virtual bool SetWindowedMode(bool bWindowed) override;
		virtual bool IsWindowedMode() override;

		virtual void BringWindowToTop() override;
		virtual void SetWindowText(const char* pChar) override;
		virtual const char* GetWindowText() override;
		virtual void FixWindowSize(bool fixed) override;
		virtual void ShowWindow(bool bShow) override;

		virtual void SetWindowMaximized(bool isMaximized) override;
		virtual bool IsWindowMaximized() override;
		virtual void GetVisibleSize(Vector2* pOut) override;

		virtual void SetLandscapeMode(std::string landscapeMode) override;
		virtual std::string GetLandscapeMode() override;
		virtual bool IsRotateScreen() override;

	private:
		void LoadAndApplySettings();
		bool UpdateScreenDevice();
		bool UpdateViewPort();
		void ApplyFullscreenWindowStyle(HWND hWnd, bool bFullscreen);
		void handle_mainloop_timer(const boost::system::error_code& err);

		HINSTANCE m_hInst;
		HWND m_hExternalWnd;
		bool m_bIsExternalWindow;
		DWORD m_dwWinThreadID;
		DWORD m_dwCoreUsage;
		RenderConfiguration m_cfg;

		int m_nCreationWidth;
		int m_nCreationHeight;
		int m_nMultiSampleType;

		bool m_bUpdateScreenDevice;
		bool m_bSizeChanged;
		bool m_bModeChanged;
		bool m_bAllowWindowClosing;
		bool m_bHasNewConfig;
		bool m_bIgnoreSizeChange;
		bool m_bPassiveRendering;
		bool m_bAutoLowerFrameRateWhenNotFocused;
		bool m_bToggleSoundWhenNotFocused;
		float m_fSavedRefreshInterval;

		WINDOWPLACEMENT m_wpPrev;
		LONG_PTR m_savedWindowStyle;
		bool m_bInBorderlessFullscreen;
	};
}
