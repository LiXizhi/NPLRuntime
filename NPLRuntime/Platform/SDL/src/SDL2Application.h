#pragma once
#include "ParaEngineAppBase.h"
#include "Core/MainLoopBase.h"
#include "Render/IRenderContext.h"
#include "RenderWindowDelegate.h"

namespace ParaEngine
{
	class CSDL2Application : public CParaEngineAppBase, public MainLoopBase
	{
	public:
		CSDL2Application();
		~CSDL2Application();
		virtual void GameToClient(int& inout_x, int& inout_y, bool bInBackbuffer = true) override;


		virtual void ClientToGame(int& inout_x, int& inout_y, bool bInBackbuffer = true) override;


		virtual void SetRefreshTimer(float fTimeInterval, int nFrameRateControl = 0) override;


		virtual bool AppHasFocus() override;


		virtual void GetStats(string& output, DWORD dwFields) override;


		virtual bool WriteRegStr(const string& root_key, const string& sSubKey, const string& name, const string& value) override;


		virtual void SetAutoLowerFrameRateWhenNotFocused(bool bEnabled) override;


		virtual const char* ReadRegStr(const string& root_key, const string& sSubKey, const string& name) override;


		virtual bool WriteRegDWORD(const string& root_key, const string& sSubKey, const string& name, DWORD value) override;


		virtual DWORD ReadRegDWORD(const string& root_key, const string& sSubKey, const string& name) override;


		virtual bool GetAutoLowerFrameRateWhenNotFocused() override;


		virtual void SetToggleSoundWhenNotFocused(bool bEnabled) override;


		virtual bool GetToggleSoundWhenNotFocused() override;

		virtual int Run();

		virtual bool InitApp(IRenderWindow* pWindow, const char* sCommandLine /* = nullptr */) override;
	
		virtual void StopApp() override { CParaEngineAppBase::StopApp(); m_exit = true; }
		
		virtual void setIMEKeyboardState(bool bOpen, bool bMoveView = false, int ctrlBottom = -1, const string& editParams="") override;

		/** write the current setting to config file. Such as graphics mode and whether full screen, etc.
		* config file at ./config.txt will be automatically loaded when the game engine starts.
		* @param sFileName: if this is "", it will be the default config file at ./config.txt
		*/
		virtual void WriteConfigFile(const char* sFileName) override;

		/** change the full screen mode, it does not immediately change the device, call UpdateScreenMode() to update the device. */
		virtual void GetScreenResolution(Vector2* pOut) override;

		virtual void SetScreenResolution(const Vector2& vSize) override;

		/** change the full screen mode, it does not immediately change the device, call UpdateScreenMode() to update the device. */
		virtual void GetResolution(float* pX, float* pY) override;
		virtual void SetResolution(float x, float y)  override;

		/** call this function to update changes of FullScreen Mode and Screen Resolution. */
		virtual bool UpdateScreenMode() override;

		// ParaEngine pipeline routines
		/** switch to either windowed mode or full screen mode. */
		virtual bool SetWindowedMode(bool bWindowed) override;

		/** return true if it is currently under windowed mode. */
		virtual bool IsWindowedMode();
		/** set the window title when at windowed mode */
		virtual void SetWindowText(const char* pChar) override;
		/** get the window title when at windowed mode */
		virtual const char* GetWindowText() override;
		virtual void FixWindowSize(bool fixed) override;

		/** Show or hide current window. This function only works if IsFullScreenMode() is false.
		 */
		virtual void ShowWindow(bool bShow) override;

		virtual void SetTouchInputting(bool bTouchInputting) override {
			CParaEngineAppBase::SetTouchInputting(bTouchInputting);
		}

		virtual void SetLandscapeMode(std::string landscapeMode);
		virtual std::string GetLandscapeMode();
		virtual bool IsRotateScreen();

		bool IsInputing();
	protected:
		void LoadAndApplySettings();
		bool UpdateScreenDevice();
		bool UpdateViewPort();
#ifndef EMSCRIPTEN_SINGLE_THREAD
		void handle_mainloop_timer(const boost::system::error_code& err);
#endif
		RenderConfiguration m_cfg;

		bool m_bUpdateScreenDevice;
		bool m_bSizeChanged;
		RenderWindowDelegate m_renderWindow;
		bool m_exit;
		std::string m_title;
	};
}
