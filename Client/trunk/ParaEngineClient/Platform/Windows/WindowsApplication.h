#pragma once

#include "ParaEngineAppBase.h"
#include "ITouchInputTranslator.h"
#include "IParaEngineApp.h"
#include <map>

// forward declare
namespace ParaEngine
{
	struct SpriteFontEntity;
	class CDirectKeyboard;
	class CGUIRoot;
	class CSceneObject;
	class CAISimulator;
	class CParaWorldAsset;
	class CAudioEngine;
	class CWinRawMsgQueue;
	class CViewportManager;

	class IRenderContext;
	class IRenderDevice;
	class WindowsRenderWindow;
	struct CWinRawMsg;
}

namespace ParaEngine
{

	class CWindowsApplication : public CParaEngineAppBase
	{


	protected:


		HRESULT Initialize3DEnvironment();
		HRESULT Reset3DEnvironment();
		void    Cleanup3DEnvironment();
		/** frame move and render
		* @param bForceRender: if true, it will force frame move and render the scene. if not, it will
		* internally use a frame rate controller that maintain the frame rate at 30 fps, no matter who often this function is called. */
		HRESULT Render3DEnvironment(bool bForceRender = false);
		virtual void UpdateStats();

		virtual bool UpdateViewPort();

		/** this function is called per frame, in most cases, it will render the 3d scene and frame move.
		* call this as often as one like internally it will use a timer to best fit the interval.
		*/
		virtual HRESULT DoWork();

		LRESULT HandleWindowMessage(ParaEngine::WindowsRenderWindow* sender, UINT uMsg, WPARAM wParam, LPARAM lParam);

	public:

		virtual void    Pause(bool bPause);
		virtual bool	IsPaused();

		/** force present the scene. */
		HRESULT PresentScene();

		void    EnablePassiveRendering(bool bEnable) { m_bPassiveRendering = bEnable; };
		bool	IsPassiveRenderingEnabled() { return m_bPassiveRendering; };

		/** disable 3D rendering, do not present the scene.
		* This is usually called before and after we show a standard win32 window during full screen mode, such as displaying a flash window
		* @param bEnable: true to enable.
		*/
		void Enable3DRendering(bool bEnable) { m_bEnable3DRendering = bEnable; };

		/** whether 3D rendering is enabled, do not present the scene.
		* This is usually called before and after we show a standard win32 window during full screen mode, such as displaying a flash window */
		bool Is3DRenderingEnabled() { return m_bEnable3DRendering; };

		/**
		* Set the frame rate timer interval
		* @param fTimeInterval:  value in seconds. such as 0.033f or 0.01667f
		* 	Passing a value <= 0 to render in idle time.
		* @param nFrameRateControl: 0 for real time, 1 for ideal frame rate at 30 FPS no matter whatever time interval is set.
		*/
		void SetRefreshTimer(float fTimeInterval, int nFrameRateControl = 0);

		/** get the refresh timer.
		*/
		float GetRefreshTimer() const;

		// Functions to create, run, pause, and clean up the application
		virtual HRESULT Create();
		virtual int     Run(HINSTANCE hInstance);

		ParaEngine::PEAppState GetAppState() { return m_nAppState; }
		void SetAppState(ParaEngine::PEAppState state) { m_nAppState = state; }
	protected:

		CD3DSettings      m_d3dSettings;

		// Internal variables for the state of the app
		bool              m_bWindowed;
		bool              m_bActive;
		bool              m_bDeviceLost;
		bool              m_bMinimized;
		bool              m_bMaximized;
		bool              m_bIgnoreSizeChange;
		bool              m_bDeviceObjectsInited;
		bool              m_bDeviceObjectsRestored;

		// Internal variables used for timing
		bool              m_bFrameMoving;
		bool              m_bSingleStep;


		/// passive rendering, it will not render the scene, but simulation and time remains the same. Default is false
		bool              m_bPassiveRendering;

		/// whether to render 3d scene and present to screen. 
		bool              m_bEnable3DRendering;

		// Main objects used for creating and rendering the 3D scene
		HWND              m_hWnd;              // The main app window
		HWND              m_hWndFocus;         // The D3D focus window (usually same as m_hWnd)
		HMENU             m_hMenu;             // App menu bar (stored here when fullscreen)

		IRenderContext*   m_pRenderContext;
		IRenderDevice*    m_pRenderDevice;

		DWORD             m_dwCreateFlags;     // Indicate sw or hw vertex processing
		RECT              m_rcWindowClient;    // Saved client area size for mode switches

											   // Variables for timing
		double            m_fTime;             // Current time in seconds
		double            m_fElapsedTime;      // Time elapsed since last frame
		FLOAT             m_fFPS;              // Instanteous frame rate
		TCHAR             m_strDeviceStats[90];// string to hold D3D device stats
		TCHAR             m_strFrameStats[90]; // string to hold frame stats
		float			  m_fRefreshTimerInterval; //  in seconds. 
		int				  m_nFrameRateControl;

		// Overridable variables for the app
		TCHAR*            m_strWindowTitle;    // Title for the app's window
		DWORD             m_dwCreationWidth;   // Width used to create window
		DWORD             m_dwCreationHeight;  // Height used to create window
		bool              m_bShowCursorWhenFullscreen; // Whether to show cursor when fullscreen
		bool              m_bClipCursorWhenFullscreen; // Whether to limit cursor pos when fullscreen
		bool              m_bStartFullscreen;  // Whether to start up the app in fullscreen mode
		bool              m_bCreateMultithreadDevice; // Whether to create a multithreaded device
		bool              m_bAllowDialogBoxMode; // If enabled the framework will try to enable GDI dialogs while in fullscreen

												 /** application state */
		PEAppState m_nAppState;


		WindowsRenderWindow* m_pRenderWindow;


	public:
		/** start the application immediately, it calls StartApp with the given command line.
		* @param lpCmdLine: command line arguments. we support the following argument at the moment
		*  - bootstappper="config/bootstrapper.xml"
		*/
		CWindowsApplication(const char* lpCmdLine);

		virtual ~CWindowsApplication();


		/** whether it is debug build. */
		bool IsDebugBuild();




		/** whether to use full screen mode, it does not immediately change the device, call UpdateScreenMode() to update the device. */
		virtual void SetFullScreenMode(bool bFullscreen);
		virtual bool IsFullScreenMode();

		/** The BringWindowToTop current window to the top of the Z order. This function only works if IsFullScreenMode() is false.
		*/
		virtual void BringWindowToTop();

		/** whether the user can close the window (sending WM_CLOSE message). Default to true.
		* When set to false, the scripting interface will receive WM_CLOSE message via system event. And can use ParaEngine.Exit() to quit the application after user confirmation, etc.
		*/
		virtual void SetAllowWindowClosing(bool bAllowClosing);
		virtual bool IsWindowClosingAllowed();



		/** change the full screen mode, it does not immediately change the device, call UpdateScreenMode() to update the device. */
		virtual void GetScreenResolution(Vector2* pOut);
		virtual void SetScreenResolution(const Vector2& vSize);

		virtual void GetResolution(float* pX, float* pY);
		virtual void SetResolution(float x, float y);

		/** anti-aliasing for both windowed and full screen mode. it does not immediately change the device, call UpdateScreenMode() to update the device.*/
		virtual int GetMultiSampleType();
		virtual void SetMultiSampleType(int nType);

		/** anti-aliasing for both windowed and full screen mode. it does not immediately change the device, call UpdateScreenMode() to update the device.*/
		virtual int GetMultiSampleQuality();
		virtual void SetMultiSampleQuality(int nType);

		/** call this function to update changes of FullScreen Mode and Screen Resolution. */
		virtual bool UpdateScreenMode();

		// ParaEngine pipeline routines
		/** switch to either windowed mode or full screen mode. */
		virtual bool SetWindowedMode(bool bWindowed);

		/** return true if it is currently under windowed mode. */
		virtual bool IsWindowedMode();
		/** set the window title when at windowed mode */

		virtual void SetWindowText(const char* pChar);
		/** get the window title when at windowed mode */
		virtual const char* GetWindowText();

		/** get the render engine stats to output.
		* @param output: the output buffer.
		* @param dwFields: current it is 0, which just collect graphics card settings.
		* 1 is os information.
		*/
		virtual void GetStats(string& output, DWORD dwFields = 0);

		/**
		* Write a string to the registry. e.g. WriteRegStr("HKLM", "Software\My Company\My Software", "string Value", "string Name");
		* @param root_key: must be
		HKCR or HKEY_CLASSES_ROOT
		HKLM or HKEY_LOCAL_MACHINE
		HKCU or HKEY_CURRENT_USER
		HKU or HKEY_USERS
		*/
		bool WriteRegStr(const string& root_key, const string& sSubKey, const string& name, const string& value);
		/** Read string from the registry.
		* Valid values for root_key are listed under WriteRegStr. NULL will be returned if the string is not present.
		* If the value is present, but is of type REG_DWORD, it will be read and converted to a string.
		*/
		const char* ReadRegStr(const string& root_key, const string& sSubKey, const string& name);

		/**
		* Write a DWORD to the registry. see WriteRegStr() for more info
		*/
		bool WriteRegDWORD(const string& root_key, const string& sSubKey, const string& name, DWORD value);

		/** Read DWORD from the registry.
		* Valid values for root_key are listed under WriteRegStr. NULL will be returned if the DWORD is not present or type is a string.
		*/
		DWORD ReadRegDWORD(const string& root_key, const string& sSubKey, const string& name);

		/** get the current mouse cursor position.
		* @param pX: out
		* @param pY: out
		* @param bInBackbuffer: if true, it will scale the output according to the ratio of back buffer and current window size.
		*/
		virtual void GetCursorPosition(int* pX, int * pY, bool bInBackbuffer = true);

		/** translate a position from game coordination system to client window position.
		* @param inout_x: in and out
		* @param inout_y: in and out
		* @param bInBackbuffer: if true, it will scale the output according to the ratio of back buffer and current window size.
		*/
		virtual void GameToClient(int& inout_x, int & inout_y, bool bInBackbuffer = true);

		/** translate a position from client window position to game coordination system.
		* @param inout_x: in and out
		* @param inout_y: in and out
		* @param bInBackbuffer: if true, it will scale the output according to the ratio of back buffer and current window size.
		*/
		virtual void ClientToGame(int& inout_x, int & inout_y, bool bInBackbuffer = true);

		/** write the current setting to config file. Such as graphics mode and whether full screen, etc.
		* config file at ./config.txt will be automatically loaded when the game engine starts.
		* @param sFileName: if this is "", it will be the default config file at ./config.txt
		*/
		virtual void WriteConfigFile(const char* sFileName);

		/** get whether ParaEngine is loaded from config/config.new.txt. if this is true, we need to ask the user to save to config/config.txt. This is usually done at start up. */
		virtual bool HasNewConfig();

		/** set whether ParaEngine is loaded from config/config.new.txt. if this is true, we need to ask the user to save to config/config.txt. This is usually done at start up. */
		virtual void SetHasNewConfig(bool bHasNewConfig);

		/** switch to ignore windows size change. default to false.
		* if false, the user is allowed to adjust window size in windowed mode. */
		virtual void SetIgnoreWindowSizeChange(bool bIgnoreSizeChange);

		/** return true if it is currently under windowed mode. */
		virtual bool GetIgnoreWindowSizeChange();

		/** render the current frame and does not return until everything is presented to screen.
		* this function is usually used to draw the animated loading screen. */
		virtual bool ForceRender();
	public:
		/** this function should be called when the application is created. I.e. the windows HWND is valid.
		*/
		HRESULT OnCreateWindow();


		/** init application */
		HRESULT Init(HWND* pHWND);

		/**
		* This function should be called only once when the application start, one can initialize game objects here.
		* @param pHWND:a pointer to the handle of the current application window.
		*/
		HRESULT OneTimeSceneInit();

		/**
		* This callback function will be called immediately after the Direct3D device has been
		* created, which will happen during application initialization and windowed/full screen
		* toggles. This is the best location to create D3DPOOL_MANAGED resources since these
		* resources need to be reloaded whenever the device is destroyed. Resources created
		* here should be released in the OnDestroyDevice callback.
		*/
		virtual HRESULT InitDeviceObjects();

		/**
		* This callback function will be called immediately after the Direct3D device has been
		* reset, which will happen after a lost device scenario. This is the best location to
		* create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever
		* the device is lost. Resources created here should be released in the OnLostDevice
		* callback.
		*/
		virtual HRESULT RestoreDeviceObjects();


		/**
		* This callback function will be called at the end of every frame to perform all the
		* rendering calls for the scene, and it will also be called if the window needs to be
		* repainted. After this function has returned, application should call
		* IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
		* @param fTime: Current time elapsed.
		*/
		virtual HRESULT Render();

		/** Output text information to console, for debugging only.*/
		void GenerateD3DDebugString();

		/**
		* This callback function will be called once at the beginning of every frame. This is the
		* best location for your application to handle updates to the scene, but is not
		* intended to contain actual rendering calls, which should instead be placed in the
		* OnFrameRender callback.
		* @param fTime: Current time elapsed.
		*/
		virtual HRESULT FrameMove();
		virtual HRESULT FrameMove(double fTime);

		/**
		* This callback function will be called immediately after the Direct3D device has
		* entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
		* in the OnResetDevice callback should be released here, which generally includes all
		* D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for
		* information about lost devices.
		*/
		virtual HRESULT InvalidateDeviceObjects();
		/**
		* This callback function will be called immediately after the Direct3D device has
		* been destroyed, which generally happens as a result of application termination or
		* windowed/full screen toggles. Resources created in the OnCreateDevice callback
		* should be released here, which generally includes all D3DPOOL_MANAGED resources.
		*/
		virtual HRESULT DeleteDeviceObjects();

		/**
		* This function should be called only once when the application end, one can destroy game objects here.
		*/
		virtual HRESULT FinalCleanup();

	public:
		/** process game input.*/
		void HandleUserInput();

		virtual void SetMainWindow(HWND hWnd, bool bIsExternalWindow = true);
		virtual HWND GetMainWindow();

		/** call this function to create the ParaEngine main rendering device based on the current window.
		* If no window is specified using SetMainWindow(), it will create a default window using the default config settings.
		* @param sCommandLine: the command line parameter
		*/
		virtual HRESULT StartApp(const char* sCommandLine = 0);

		void InitSystemModules();

		void InitWin3DSettings();

		void InitLogger();

		void BootStrapAndLoadConfig();

		void InitApp(const char* sCommandLine);

		bool CheckClientLicense();

		void LoadAndApplySettings();

		virtual HRESULT StopApp();

		virtual LRESULT MsgProcWinThread(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool bCallDefProcedure = true);

		virtual LRESULT MsgProcWinThreadCustom(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual LRESULT SendMessageToApp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool PostWinThreadMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

		/** get a message from the application message queue and remove it from the queue. This function is mostly
		used internally by the main thread.
		* @param pMsg: the receiving message
		* @return true if one message is fetched. or false if there is no more messages in the queue.
		*/
		virtual bool GetMessageFromApp(CWinRawMsg* pMsg);

		/**
		* Before handling window messages, application should pass incoming windows
		* messages to the application through this callback function.
		*/
		virtual LRESULT MsgProcApp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		/** create instance */
		virtual HRESULT Create(HINSTANCE hInstance);
		virtual float GetFPS();
		void UpdateFrameStats(double fTime);

		/** change the cursor to indicate a movie recording state*/
		//void UpdateRecordingState();

		/** get the module handle, it may be exe or the dll handle, depending on how the main host app is built. */
		virtual HINSTANCE GetModuleHandle();


		virtual float GetRefreshTimer();

		virtual void GetWindowCreationSize(int * pWidth, int * pHeight);


		/** this function is called whenever the application is disabled or enabled. usually called when receiving the WM_ACTIVATEAPP message.
		* [main thread only]
		*/
		virtual void ActivateApp(bool bActivate);

		/** whether the application is active or not. */
		virtual bool IsAppActive();

		/** Get the current ParaEngine app usage.
		* [main thread only]
		* @return see PE_USAGE
		*/
		virtual DWORD GetCoreUsage();

		/** Set the current ParaEngine app usage.
		* [main thread only]
		* @param dwUsage: bitwise of PE_USAGE
		*/
		virtual void SetCoreUsage(DWORD dwUsage);

		/** set the minimum UI resolution size. if the backbuffer is smaller than this, we will use automatically use UI scaling
		* for example, if minimum width is 1024, and backbuffer it 800, then m_fUIScalingX will be automatically set to 1024/800.
		* calling this function will cause OnSize() and UpdateBackbufferSize() to be called. Actually it calls SetUIScale()
		* [main thread only]
		* @param nWidth: the new width.
		* @param nHeight: the new height.
		* @param bAutoUIScaling: default to true. whether we will automatically recalculate the UI scaling accordingly with regard to current backbuffer size.
		*/
		virtual void SetMinUIResolution(int nWidth, int nHeight, bool bAutoUIScaling = true);

		/** Send the exit message, so that the game engine will prepare to exit in the next frame.
		* this is the recommended way of exiting application.
		* this is mainly used for writing test cases. Where a return value of 0 means success, any other value means failure.
		*/
		virtual void Exit(int nReturnCode = 0);

		/** this returns true if the main window or one of its child window has key focus.
		* @param hWnd: the current window handle from which we will compare. If 0, we will use GetFocus() to get it.
		*/
		virtual bool HasFocus(HWND hWnd = 0);

		virtual bool IsTouchInputting() { return m_isTouching; }
		virtual bool IsSlateMode();

		// deprecated:
		virtual int32 GetTouchPointX() { return m_touchPointX; }
		virtual int32 GetTouchPointY() { return m_touchPointY; }


		/** server mode has no user interface */
		virtual bool IsServerMode();;
	public:
		/// default to false. if true, we will lower frame rate when the window is not focused.
		void SetAutoLowerFrameRateWhenNotFocused(bool bEnabled);
		bool GetAutoLowerFrameRateWhenNotFocused();

		/// if true, sound volume will be set to 0, when not focused.
		void SetToggleSoundWhenNotFocused(bool bEnabled);
		bool GetToggleSoundWhenNotFocused();

		/** return true if app has focus. Usually we will lower render frame move when app has lost focus in web browser mode. */
		bool AppHasFocus();

		/** managing multiple 3d views */
		CViewportManager* GetViewportManager();

	private:
		bool UpdateScreenDevice();

		/** get touch event sCode from windows message */
		const char* GetTouchEventSCodeFromMessage(const char * event_type, HWND hWnd, WPARAM wParam, LPARAM lParam);

		/** set the app window to a rect. it will ensure that rect left, top is bigger than 0 in windowed mode. */
		void SetAppWndRect(const RECT& rect);
	protected:
		HINSTANCE             m_hInstance;                // app instance

														  /** 2d gui root */
		ref_ptr<CGUIRoot> m_pGUIRoot;
		/** 3d scene root object */
		ref_ptr<CSceneObject>		  m_pRootScene;
		/** viewport */
		ref_ptr<CViewportManager>	  m_pViewportManager;
		/** asset manager */
		ref_ptr<CParaWorldAsset>	  m_pParaWorldAsset;

		CDirectKeyboard*	  m_pKeyboard;
		float                 m_fAspectRatio;             // Aspect ratio used by the FPS camera
		CAudioEngine*        m_pAudioEngine;                   // audio engine class

		bool                  m_bDrawReflection : 1;          // reflection-drawing option
		bool                  m_bDisplayText : 1;             // whether to display text
		bool                  m_bDisplayHelp : 1;             // whether to display help text
		bool	m_bServerMode : 1;
		// whether we are loaded from config/config.new.txt.
		bool m_bHasNewConfig : 1;
		bool m_bAllowWindowClosing : 1;
		// default to false. if true, we will lower frame rate when the window is not focused.
		bool m_bAutoLowerFrameRateWhenNotFocused : 1;
		// if true, sound volume will be set to 0, when not focused.
		bool m_bToggleSoundWhenNotFocused : 1;

		/** initial game effect setting to be loaded from the config file. default value it 0*/
		int m_nInitialGameEffectSet;


		/** 1 window mode desired, 0 full screen mode desired, -1 if unknown. */
		int					m_nWindowedDesired;
		bool				m_bIsKeyEvent;
		bool				m_bUpdateScreenDevice; // whether to toggle screen in the next frame

												   // for keeping command line arguments.
		std::string m_cmd;

		/** The thread safe message queue target for receiving messages from window thread. They are processed in the main game thread though. */
		CWinRawMsgQueue*	m_pWinRawMsgQueue;

		/** the thread id of the main window thread. It is used when doing the PostWinThreadMessage(). */
		DWORD m_dwWinThreadID;

		/** the Core usage.  @see PE_USAGE 	*/
		DWORD m_dwCoreUsage;

		/** whether app has focus. */
		bool m_bAppHasFocus;

		/** for debugging display */
		std::string m_sTitleString;

		/** set foreground window */
		HWND m_hwndTopLevelWnd;

		bool m_isSlateMode;
		// deprecated:
		int32 m_touchPointX;
		int32 m_touchPointY;

		ITouchInputTranslator* m_pTouchInput;

		ITouchInputTranslator* LoadTouchInputPlug();

	};

}