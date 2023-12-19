#pragma once


#ifdef PLATFORM_MAC
    #include "platform/mac/ParaEngineApp.h"
#else


#include "ParaEngineAppBase.h"
#include "common/d3dapp.h"
#include "ITouchInputTranslator.h"

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
	struct CWinRawMsg;
}

/**
@mainpage ParaEngine Reference

*
* @section intro_sec Introduction
*
[The following text is taken from my bachelor thesis proposal]\n
In recent years, game engine related technology has drawn increasing academic attention from
a wide range of research areas. People come to realize that game engine may naturally evolve
in to the most widely used virtual reality platform in the future. The research framework
proposed in this paper is to exploit this possibility using established computer technologies
as well as newly designed ones. Current game engine framework already include solutions for a
large number of platform issues, such as real-time 3D visualization, physics simulation, event
and script system, path-finding, high-level decision making, networking, etc. However, the
computing paradigm behind it is usually constrained to a single platform, where a predefined
network topology must be explicitly constructed for cross-platform communications. My major
research goal is to redesign the computing paradigm to suit the need of highly dynamic networked
virtual environment, where intelligent entities are situated and communicate with each other
as well as human avatars. A complete distributed game engine framework will be proposed and
implemented with supporting game demos. Two areas of interest will be specialized with in-depth
study. One is the script programming environment and runtime for distributed game world logics;
the other is autonomous character animations in virtual game worlds.

The idea of distributed computer game engine can be pictured by drawing an analog with the current
World Wide Web. I.e. we compare web pages to 3D game worlds; hyperlinks and services in web pages
to active objects in 3D game worlds; and web browsers and client/server side runtime environments
to computer game engines. However, in distributed game world, interactions among entities will be
more intensive and extensive, such as several characters exchanging messages at real time; game world
logic will be more distributed, with each node being a potential server, and also more dynamic,
with different nodes forming temporary or long lasting relationships.
* \n
* @section copyright Copyright
*
I will possibly release it under GNU license when the game engine framework is stable.
* \n
* @section developer Developer
*
- Li, Xizhi: Developer of ParaEngine.
*/

/**
* The main game engine implementations.
* The ParaEngine namespace contains the main ParaEngine source code.
* It includes scene objects, scene management, asset and file management,
* 2D GUI, AI modules, ParaX file support, frame rate management, etc.
*/
namespace ParaEngine
{
	/**
	* This class demonstrate how to initialize, destroy and drive the game loop
	* of ParaEngine through the C++ programming interface. Users can derive their
	* main Windows application from this class.
	* Note: paraengine is designed to be manipulated through the NPL scripting interface
	* Currently,its C++ programming interface is not designed to be used from outside the core code.
	* i.e. Users' ability to program through C++ API is restricted by the amount of
	* source code unvailed to them.
	*
	* this class can be regarded as sample code for writing your own ParaEngine games
	* logics. This class is not engine specific, but it contains basic steps to establish
	* a running environment of any paraEngine created games. For example: SceneObject,
	* Environment and AI simulator are created here. Message handling, Timing and I/O are
	* also processed partly here.
	*
	* @see ParaWorld::CMyD3DApplication
	* Note for debugging: please see the macro comments
	*/
	class CParaEngineApp : public CD3DApplication, public CParaEngineAppBase
	{
	public:
		/** start the application immediately, it calls StartApp with the given command line.
		* @param lpCmdLine: command line arguments. we support the following argument at the moment
		*  - bootstappper="config/bootstrapper.xml"
		*/
		CParaEngineApp(const char* lpCmdLine);
		/** One needs to manually start the application.
		*/
		CParaEngineApp();
		virtual ~CParaEngineApp();

		/** only call this function if one does not want to manage game loop externally. */
		virtual int Run(HINSTANCE hInstance);

		/** whether it is debug build. */
		bool IsDebugBuild();

		/** this function is called per frame, in most cases, it will render the 3d scene and frame move.
		* call this as often as one like internally it will use a timer to best fit the interval.
		*/
		virtual HRESULT DoWork();

		/**  passive rendering, it will not render the scene, but simulation and time remains the same. Default is false*/
		virtual void    EnablePassiveRendering( bool bEnable );
		/**  passive rendering, it will not render the scene, but simulation and time remains the same. Default is false*/
		virtual bool	IsPassiveRenderingEnabled( );

		/** disable 3D rendering, do not present the scene.
		* This is usually called before and after we show a standard win32 window during full screen mode, such as displaying a flash window
		* @param bEnable: true to enable.
		*/
		virtual void Enable3DRendering(bool bEnable);

		/** whether 3D rendering is enabled, do not present the scene.
		* This is usually called before and after we show a standard win32 window during full screen mode, such as displaying a flash window */
		virtual bool Is3DRenderingEnabled();


		/** whether to use full screen mode, it does not immediately change the device, call UpdateScreenMode() to update the device. */
		virtual void SetFullScreenMode(bool bFullscreen);
		virtual bool IsFullScreenMode();

		/** The BringWindowToTop current window to the top of the Z order. This function only works if IsFullScreenMode() is false.
		*/
		virtual void BringWindowToTop();

		/** Show or hide current window. This function only works if IsFullScreenMode() is false.
		 */
		virtual void ShowWindow(bool bShow);

		/** whether the user can close the window (sending WM_CLOSE message). Default to true.
		* When set to false, the scripting interface will receive WM_CLOSE message via system event. And can use ParaEngine.Exit() to quit the application after user confirmation, etc.
		*/
		virtual void SetAllowWindowClosing(bool bAllowClosing);
		virtual bool IsWindowClosingAllowed();

		/** turn on/off menu */
		virtual void ShowMenu(bool bShow);

		virtual void SetWindowMaximized(bool isMaximized);
		virtual bool IsWindowMaximized();

		virtual void GetVisibleSize(Vector2* pOut);

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
		virtual void FixWindowSize(bool fixed);

		/** get the render engine stats to output.
		* @param output: the output buffer.
		* @param dwFields: current it is 0, which just collect graphics card settings.
		* 1 is os information.
		*/
		virtual void GetStats(string& output, DWORD dwFields=0);

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
		virtual void GetCursorPosition(int* pX,int * pY, bool bInBackbuffer = true);

		/** translate a position from game coordination system to client window position.
		* @param inout_x: in and out
		* @param inout_y: in and out
		* @param bInBackbuffer: if true, it will scale the output according to the ratio of back buffer and current window size.
		*/
		virtual void GameToClient(int& inout_x,int & inout_y, bool bInBackbuffer = true);

		/** translate a position from client window position to game coordination system.
		* @param inout_x: in and out
		* @param inout_y: in and out
		* @param bInBackbuffer: if true, it will scale the output according to the ratio of back buffer and current window size.
		*/
		virtual void ClientToGame(int& inout_x,int & inout_y, bool bInBackbuffer = true);

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

		/**
		* Called during device initialization, this code checks the device for some
		* minimum set of capabilities, and rejects those that don't pass by returning false.
		*/
		HRESULT ConfirmDevice( LPDIRECT3D9 pD3d, D3DCAPS9*, DWORD, D3DFORMAT, D3DFORMAT );

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

		virtual void SetMainWindow(HWND hWnd, bool bIsExternalWindow=true);
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

		virtual LRESULT MsgProcWinThread( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool bCallDefProcedure = true);

		virtual LRESULT MsgProcWinThreadCustom( UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual LRESULT SendMessageToApp( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
		virtual LRESULT MsgProcApp( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		/** create instance */
		virtual HRESULT Create( HINSTANCE hInstance = 0 );

		virtual HRESULT CreateFromD3D9Device(IDirect3DDevice9* pD3dDevice, IDirect3DSwapChain9* apSwapChain);

		virtual HRESULT Render3DEnvironment(bool bForceRender = false);
		virtual float GetFPS();
		void UpdateFrameStats(double fTime);

		/** change the cursor to indicate a movie recording state*/
		//void UpdateRecordingState();

		/** get the module handle, it may be exe or the dll handle, depending on how the main host app is built. */
		virtual HINSTANCE GetModuleHandle();

		virtual void SetRefreshTimer(float fTimeInterval, int nFrameRateControl = 0);

		virtual float GetRefreshTimer();

		virtual void GetWindowCreationSize(int * pWidth, int * pHeight);

		virtual PEAppState GetAppState();

		/** set application state */
		virtual void SetAppState(ParaEngine::PEAppState state);

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

		/** set the maximum UI resolution size. if the backbuffer is larger than this, we will use automatically use UI scaling 
		 * for example, if maximum width is 1024, and backbuffer it 1600, then m_fUIScalingX will be automatically set to 1024/1600. 
		 * calling this function will cause OnSize() and UpdateBackbufferSize() to be called. Actually it calls SetUIScale()
		 * [main thread only]
		 * @param nWidth: the new width. 
		 * @param nHeight: the new height. 
		 * @param bAutoUIScaling: default to true. whether we will automatically recalculate the UI scaling accordingly with regard to current backbuffer size. 
		 */
		virtual void SetMaxUIResolution(int nWidth, int nHeight, bool bAutoUIScaling = true);

		/** Send the exit message, so that the game engine will prepare to exit in the next frame.
		* this is the recommended way of exiting application.
		* this is mainly used for writing test cases. Where a return value of 0 means success, any other value means failure.
		*/
		virtual void Exit(int nReturnCode = 0);

		/** this returns true if the main window or one of its child window has key focus.
		* @param hWnd: the current window handle from which we will compare. If 0, we will use GetFocus() to get it.
		*/
		virtual bool HasFocus(HWND hWnd = 0);

		virtual bool IsTouchInputting(){ return m_isTouching; }
		virtual bool IsSlateMode();

		// deprecated:
		virtual int32 GetTouchPointX(){ return m_touchPointX; }
		virtual int32 GetTouchPointY(){ return m_touchPointY; }


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

		bool                  m_bDrawReflection:1;          // reflection-drawing option
		bool                  m_bDisplayText:1;             // whether to display text
		bool                  m_bDisplayHelp:1;             // whether to display help text
		bool	m_bServerMode:1;
		// whether we are loaded from config/config.new.txt.
		bool m_bHasNewConfig:1;
		bool m_bAllowWindowClosing:1;
		// default to false. if true, we will lower frame rate when the window is not focused.
		bool m_bAutoLowerFrameRateWhenNotFocused:1;
		// if true, sound volume will be set to 0, when not focused.
		bool m_bToggleSoundWhenNotFocused:1;

		/** initial game effect setting to be loaded from the config file. default value it 0*/
		int m_nInitialGameEffectSet;

		bool m_bInitialIsWindowMaximized;
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

		float m_fFPS;

		ITouchInputTranslator* m_pTouchInput;

		ITouchInputTranslator* LoadTouchInputPlug();

	public:
		static const int s_screen_orientation_auto      = 0; // 保持真实窗口方向
		static const int s_screen_orientation_landscape = 1; // 横屏 渲染宽大于等于渲染高
		static const int s_screen_orientation_portrait  = 2; // 竖屏 渲染宽小于等于渲染高

		virtual void SetLandscapeMode(std::string landscapeMode);
		virtual std::string GetLandscapeMode();
		virtual bool IsRotateScreen();
    	HRESULT HandlePossibleSizeChange(bool bUpdateSizeOnly = false);

		int m_screen_orientation;  // 屏幕方向
		bool m_screen_rotated;     // 屏幕是否旋转
	};

}

#endif
