#pragma once
//-----------------------------------------------------------------------------
// Copyright (C) 2007 - 2010 ParaEngine Corporation, All Rights Reserved.
// Author: LiXizhi
// Date:	2010.2
// Description:	API for ParaEngine App windows management interface 
//-----------------------------------------------------------------------------

#include <string>
#include <stdint.h>
#include "BaseInterface.h"
#include "IAttributeFields.h"

#if defined(__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)))
	#define PE_DEPRECATED_ATTRIBUTE __attribute__((deprecated))
#elif _MSC_VER >= 1400 //vs 2005 or higher
	#define PE_DEPRECATED_ATTRIBUTE __declspec(deprecated) 
#else
	#define PE_DEPRECATED_ATTRIBUTE
#endif 


struct IDirect3DDevice9;
struct IDirect3DSwapChain9;

namespace NPL{
	class INPLRuntime;
}

namespace ParaEngine
{
	struct CWinRawMsg;
	class CRefCounted;
	class Vector2;
	class IRenderWindow;
	class IAttributeFields;

	/** ParaEngine application state */
	enum PEAppState
	{
		PEAppState_None = 0,
		PEAppState_Started,
		PEAppState_Device_Created,
		PEAppState_Device_Error,
		PEAppState_Ready,
		PEAppState_SwitchedOut,
		PEAppState_Stopped,
		PEAppState_Exiting,
		PEAppState_Exited,
		PEAppState_Unknown,
	};

	/** how ParaEngine core is used. the first 4 bits are mutually exclusive. 0xfffffff0.*/
	enum PE_USAGE
	{
		// standalone application
		PE_USAGE_STANDALONE = 1,
		// Used in web browser
		PE_USAGE_WEB_BROWSER = 2,
		// as a system service
		PE_USAGE_SERVICE = 4,
		// owns the d3d device, otherwise it is using external device. 
		PE_USAGE_OWN_D3D = 16, 
		// owns the main window, otherwise it renders to an external window. 
		PE_USAGE_OWN_WINDOW = 32,
	};

#ifndef WIN32
#define WM_USER 0x0400
#endif
/** @def this is file path we will find to locate the paraengine root directory. */
#define PARAENGINE_SIG_FILE	"ParaEngine.sig"
/** @def first paraengine user message */
#define PE_WM_FIRST			WM_USER+2301
/** @def Show/hide cursor for current window from any threads. One can use PostThreadMessage() from any threads.*/
#define PE_WM_SHOWCURSOR			WM_USER+2301
/** @def This allows any thread to post a user message to terminate the application window */
#define PE_WM_QUIT					WM_USER+2302
/** @def set capture for current window from any threads*/
#define PE_WM_SETCAPTURE			WM_USER+2303
/** @def release capture for current window from any threads*/
#define PE_WM_RELEASECAPTURE		WM_USER+2304
/** @def call set focus in the current thread to the hWnd passed. wParam is the hWND */
#define PE_WM_SETFOCUS		WM_USER+2305
/** @def call _ImmSetOpenStatus() in the current thread to the hWnd passed. wParam is the hWND, lParam is 0 if false.  */
#define PE_IME_SETOPENSTATUS		WM_USER+2306
/** @def call CGUIIME::OnFocusIn/out() function. If lParam==1, it means focus in, otherwise focus out.  */
#define PE_IME_SETFOCUS		WM_USER+2307
/** @def application switch in/out message. If lParam==1, it means switch in, otherwise switch out.  */
#define PE_APP_SWITCH		WM_USER+2308
/** @def application switch in/out message. wParam,lParam is the two error codes passed to CD3DWindowUtil::DisplayErrorMsg. lParam can be MSG_NONE, MSGERR_APPMUSTEXIT, MSGWARN_SWITCHEDTOREF*/
#define PE_APP_SHOW_ERROR_MSG	WM_USER+2309
/** @def last ParaEngine user message */
#define PE_WM_LAST			WM_USER+2399

/** @def a slow timer that periodically check if there are some tasks assigned from other threads. */
#define PE_TIMERID_HEARTBEAT		5602

	/**
	*  a table of virtual functions which are used by plug-ins to access the game engine 
	*/
	class IParaEngineApp : public BaseInterface
	{
	public:
		virtual IRenderWindow* GetRenderWindow() = 0;

		virtual bool InitApp(IRenderWindow* pWindow, const char* sCommandLine = nullptr) = 0;

		/** This is the first function that should be called when acquiring the IParaEngineApp interface. 
		* call this function to start the application. Rendering window and devices are not created, one need to call Create() instead. 
		* @param sCommandLine: the command line parameter
		*/
		virtual bool StartApp() = 0;

		/** This is the last function that should be called. It is usually called just before process exit. 
		*/
		virtual void StopApp() = 0;

		/** set the hWnd on to which we will render and process window messages.
		* this function should be called prior to Create().
		* @note: the rendering device size will use the client area of the input window
		* @param hWnd: the Window on to which we will render.
		* @param bIsExternalWindow: this is always true, unless for the default window used by ParaEngine when no window is created by the user.
		*/
		PE_DEPRECATED_ATTRIBUTE virtual void SetMainWindow(HWND hWnd, bool bIsExternalWindow = true) = 0;
		PE_DEPRECATED_ATTRIBUTE virtual HWND GetMainWindow() = 0;

		/** only call this function if one does not want to manage game loop externally. */
		virtual int Run(HINSTANCE hInstance) = 0;

		/** this function is called per frame, in most cases, it will render the 3d scene and frame move.
		* call this as often as one like internally it will use a timer to best fit the interval.
		*/
		virtual HRESULT DoWork() = 0;

		/** create from an existing d3d device. This is an advanced function to replaced the default render device.
		* and caller is responsible for managing device life time. The external caller must call InitDeviceObjects(), RestoreDeviceObjects(), InvalidateDeviceObjects(), DeleteDeviceObjects() at proper time
		*/
		PE_DEPRECATED_ATTRIBUTE virtual HRESULT CreateFromD3D9Device(IDirect3DDevice9* pD3dDevice, IDirect3DSwapChain9* apSwapChain) = 0;

		/**
		* This callback function will be called immediately after the Direct3D device has been
		* created, which will happen during application initialization and windowed/full screen
		* toggles. This is the best location to create D3DPOOL_MANAGED resources since these
		* resources need to be reloaded whenever the device is destroyed. Resources created
		* here should be released in the OnDestroyDevice callback.
		*/
		virtual HRESULT InitDeviceObjects() = 0;

		/**
		* This callback function will be called immediately after the Direct3D device has been
		* reset, which will happen after a lost device scenario. This is the best location to
		* create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever
		* the device is lost. Resources created here should be released in the OnLostDevice
		* callback.
		*/
		virtual HRESULT RestoreDeviceObjects() = 0;


		/**
		* This callback function will be called immediately after the Direct3D device has
		* entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
		* in the OnResetDevice callback should be released here, which generally includes all
		* D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for
		* information about lost devices.
		*/
		virtual HRESULT InvalidateDeviceObjects() = 0;

		/**
		* This callback function will be called immediately after the Direct3D device has
		* been destroyed, which generally happens as a result of application termination or
		* windowed/full screen toggles. Resources created in the OnCreateDevice callback
		* should be released here, which generally includes all D3DPOOL_MANAGED resources.
		*/
		virtual HRESULT DeleteDeviceObjects() = 0;


		/** create d3d render device based on the current main window.
		* Use this function to create a new render device automatically.
		*/
		PE_DEPRECATED_ATTRIBUTE virtual HRESULT Create(HINSTANCE hInstance = 0) = 0;

		/** init the application. no need to be called unless in a service where no rendering devices are created. */
		PE_DEPRECATED_ATTRIBUTE virtual HRESULT Init(HWND* pHWND = 0) = 0;

		/** Frame move and render a frame during idle time (no messages are waiting). Call this function during CPU idle time.
		* internally it uses a timer to control frame rates, so it is safe to call this as often as one like.
		* @param bForceRender: if true, it will force frame move and render the scene. if not, it will
		* internally use a frame rate controller that maintain the frame rate at 30 fps, no matter who often this function is called.
		*/
		PE_DEPRECATED_ATTRIBUTE virtual HRESULT Render3DEnvironment(bool bForceRender = false) = 0;

		/** the window message processor. One needs send all messages belonging to the main window to this function, after calling Create().
		* @note: the main rendering thread can be a different thread than the window proc thread.
		* @param bCallDefProcedure: whether we will call the ::DefWindowProdure().
		* @return: 0 if message was not processed. 1 if message is processed. -1 if message is processed by can be passed on to other processor.
		*/
		PE_DEPRECATED_ATTRIBUTE virtual LRESULT MsgProcWinThread(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool bCallDefProcedure = true) = 0;

		/*** process win thread messages without a hWnd. Such messages are usually invoked by PostWinThreadMessage() from other threads. */
		PE_DEPRECATED_ATTRIBUTE virtual LRESULT MsgProcWinThreadCustom(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

		/** Send a RAW win32 message the application to be processed in the next main thread update interval.
		* This function can be called from any thread. It is also used by the windows procedure thread to dispatch messages to the main processing thread.
		*/
		PE_DEPRECATED_ATTRIBUTE virtual LRESULT SendMessageToApp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

		/** post a raw win32 message from any thread to the thread on which hWnd is created. */
		PE_DEPRECATED_ATTRIBUTE virtual bool PostWinThreadMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

		/** get a message from the application message queue and remove it from the queue. This function is mostly
		used internally by the main thread.
		* @param pMsg: the receiving message
		* @return true if one message is fetched. or false if there is no more messages in the queue.
		*/
		PE_DEPRECATED_ATTRIBUTE virtual bool GetMessageFromApp(CWinRawMsg* pMsg) = 0;

		/**
		* handle a message in the main application thread.
		*/
		PE_DEPRECATED_ATTRIBUTE virtual LRESULT MsgProcApp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;


		/**  passive rendering, it will not render the scene, but simulation and time remains the same. Default is false*/
		virtual void    EnablePassiveRendering( bool bEnable ) = 0;

		/**  passive rendering, it will not render the scene, but simulation and time remains the same. Default is false*/
		virtual bool	IsPassiveRenderingEnabled( ) = 0;

		/** disable 3D rendering, do not present the scene. 
		* This is usually called before and after we show a standard win32 window during full screen mode, such as displaying a flash window 
		* @param bEnable: true to enable.
		*/
		virtual void Enable3DRendering(bool bEnable) = 0;

		/** whether 3D rendering is enabled, do not present the scene. 
		* This is usually called before and after we show a standard win32 window during full screen mode, such as displaying a flash window */
		virtual bool Is3DRenderingEnabled() = 0;


		/** whether to use full screen mode, it does not immediately change the device, call UpdateScreenMode() to update the device. */
		virtual void SetFullScreenMode(bool bFullscreen) = 0;
		virtual bool IsFullScreenMode() = 0;

		/** The BringWindowToTop current window to the top of the Z order. This function only works if IsFullScreenMode() is false.
		*/
		virtual void BringWindowToTop() = 0;

		/** whether the user can close the window (sending WM_CLOSE message). Default to true. 
		* When set to false, the scripting interface will receive WM_CLOSE message via system event. And can use ParaEngine.Exit() to quit the application after user confirmation, etc. 
		*/
		virtual void SetAllowWindowClosing(bool bAllowClosing) = 0;
		virtual bool IsWindowClosingAllowed() = 0;

		/** turn on/off menu */
		PE_DEPRECATED_ATTRIBUTE virtual void ShowMenu(bool bShow) = 0;


		/** change the full screen mode, it does not immediately change the device, call UpdateScreenMode() to update the device. */
		virtual void GetResolution(float* pX, float* pY) = 0;
		virtual void SetResolution(float x, float y) = 0;

		/** anti-aliasing for both windowed and full screen mode. it does not immediately change the device, call UpdateScreenMode() to update the device.*/
		virtual int GetMultiSampleType() = 0;
		virtual void SetMultiSampleType(int nType) = 0;

		/** anti-aliasing for both windowed and full screen mode. it does not immediately change the device, call UpdateScreenMode() to update the device.*/
		PE_DEPRECATED_ATTRIBUTE virtual int GetMultiSampleQuality() = 0;
		PE_DEPRECATED_ATTRIBUTE virtual void SetMultiSampleQuality(int nType) = 0;

		/** call this function to update changes of FullScreen Mode and Screen Resolution. */
		virtual bool UpdateScreenMode() = 0;

		// ParaEngine pipeline routines
		/** switch to either windowed mode or full screen mode. */
		virtual bool SetWindowedMode(bool bWindowed) = 0;
		/** return true if it is currently under windowed mode. */
		virtual bool IsWindowedMode() = 0;
		/** set the window title when at windowed mode */
		virtual void SetWindowText(const char* pChar) = 0;
		/** get the window title when at windowed mode */
		virtual const char* GetWindowText() = 0;

		/** get the current mouse cursor position.
		* @param pX: out
		* @param pY: out
		* @param bInBackbuffer: if true, it will scale the output according to the ratio of back buffer and current window size.
		*/
		PE_DEPRECATED_ATTRIBUTE virtual void GetCursorPosition(int* pX, int * pY, bool bInBackbuffer = true) = 0;

		/** translate a position from game coordination system to client window position. 
		* @param inout_x: in and out
		* @param inout_y: in and out
		* @param bInBackbuffer: if true, it will scale the output according to the ratio of back buffer and current window size. 
		*/
		virtual void GameToClient(int& inout_x,int & inout_y, bool bInBackbuffer = true) = 0;

		/** translate a position from client window position to game coordination system. 
		* @param inout_x: in and out
		* @param inout_y: in and out
		* @param bInBackbuffer: if true, it will scale the output according to the ratio of back buffer and current window size. 
		*/
		virtual void ClientToGame(int& inout_x,int & inout_y, bool bInBackbuffer = true) = 0;

		/** write the current setting to config file. Such as graphics mode and whether full screen, etc. 
		* config file at ./config.txt will be automatically loaded when the game engine starts. 
		* @param sFileName: if this is "", it will be the default config file at ./config.txt
		*/
		virtual void WriteConfigFile(const char* sFileName) = 0;

		/** get whether ParaEngine is loaded from config/config.new.txt. if this is true, we need to ask the user to save to config/config.txt. This is usually done at start up. */
		virtual bool HasNewConfig() = 0;

		/** set whether ParaEngine is loaded from config/config.new.txt. if this is true, we need to ask the user to save to config/config.txt. This is usually done at start up. */
		virtual void SetHasNewConfig(bool bHasNewConfig) = 0;

		/** switch to ignore windows size change. default to false. 
		* if false, the user is allowed to adjust window size in windowed mode. */
		virtual void SetIgnoreWindowSizeChange(bool bIgnoreSizeChange) = 0;

		/** return true if it is currently under windowed mode. */
		virtual bool GetIgnoreWindowSizeChange() = 0;

		/** get the module handle, it may be exe or the dll handle, depending on how the main host app is built. */
		virtual HINSTANCE GetModuleHandle() = 0;

		/**
		* Set the frame rate timer interval
		* @param fTimeInterval:  value in seconds. such as 0.033f or 0.01667f
		* 	Passing a value <= 0 to render in idle time. 
		* @param nFrameRateControl: 0 for real time, 1 for ideal frame rate at 30 FPS no matter whatever time interval is set. 
		*/
		virtual void SetRefreshTimer(float fTimeInterval, int nFrameRateControl=0) = 0;
		
		/** get the refresh timer. 
		*/
		virtual float GetRefreshTimer() = 0;

		/** get the window creation size in default application config. */
		virtual void GetWindowCreationSize(int * pWidth, int * pHeight) = 0;

		/** get application state */
		virtual PEAppState GetAppState() = 0;

		/** set application state */
		virtual void SetAppState(ParaEngine::PEAppState state) = 0;

		/** this function is called whenever the application is disabled or enabled. usually called when receiving the WM_ACTIVATEAPP message. 
		* [main thread only]
		*/
		virtual void ActivateApp(bool bActivate) = 0;

		/** whether the application is active or not. */
		virtual bool IsAppActive() = 0;

		/** Get the current ParaEngine app usage. 
		* [main thread only]
		* @return see PE_USAGE
		*/
		virtual DWORD GetCoreUsage() = 0;

		/** Set the current ParaEngine app usage. 
		* [main thread only]
		* @param dwUsage: bitwise of PE_USAGE
		*/
		virtual void SetCoreUsage(DWORD dwUsage) = 0;

		/** set the minimum UI resolution size. if the backbuffer is smaller than this, we will use automatically use UI scaling 
		* for example, if minimum width is 1024, and backbuffer it 800, then m_fUIScalingX will be automatically set to 1024/800. 
		* calling this function will cause OnSize() and UpdateBackbufferSize() to be called. Actually it calls SetUIScale()
		* [main thread only]
		* @param nWidth: the new width. 
		* @param nHeight: the new height. 
		* @param bAutoUIScaling: default to true. whether we will automatically recalculate the UI scaling accordingly with regard to current backbuffer size. 
		*/
		virtual void SetMinUIResolution(int nWidth, int nHeight, bool bAutoUIScaling = true) = 0;

		/** Send the exit message, so that the game engine will prepare to exit in the next frame. 
		* this is the recommended way of exiting application. 
		* this is mainly used for writing test cases. Where a return value of 0 means success, any other value means failure. 
		*/
		virtual void Exit(int nReturnCode = 0) = 0;

		/** Get the exit code that will be used when the standalone executable exit. 
		* this is mainly used for writing test cases. Where a return value of 0 means success, any other value means failure. 
		*/
		virtual void SetReturnCode(int nReturnCode) = 0;
		virtual int GetReturnCode() = 0;

		/** get the NPL runtime system associate with the application. NPL provides communication framework across different language systems. */
		virtual NPL::INPLRuntime* GetNPLRuntime() = 0;

		/** whether the last mouse input is from touch or mouse. by default it is mouse mode. */
		virtual bool IsTouchInputting() = 0;

		virtual bool IsSlateMode() = 0;

		/** obsoleted function: */
		PE_DEPRECATED_ATTRIBUTE virtual int32 GetTouchPointX() = 0;
		PE_DEPRECATED_ATTRIBUTE virtual int32 GetTouchPointY() = 0;

		/** append text to log file. */
		virtual void WriteToLog(const char* sFormat, ...) = 0;

		/** write app log to file with time and code location. */
		virtual void AppLog(const char* sMessage) { WriteToLog(sMessage); };


		/** whether the last mouse input is from touch or mouse. by default it is mouse mode. */
		virtual void SetTouchInputting(bool bTouchInputting) {};


		/** show a system message box to the user. mostly about fatal error.  */
		virtual void SystemMessageBox(const std::string& msg) {};

		/** set string specifying the command line for the application, excluding the program name.
		* calling this function multiple times with different command line is permitted. The latter settings will merge and override the previous ones.
		* @param pCommandLine: such as   key="value" key2="value2"
		*/
		virtual void SetAppCommandLine(const char* pCommandLine) {};

		/**
		* return a specified parameter value in the command line of the application. If the parameter does not exist, the return value is NULL.
		* @param pParam: key to get
		* @param defaultValue: if the key does not exist, this value will be added and returned. This can be NULL.
		*/
		virtual const char* GetAppCommandLineByParam(const char* pParam, const char* defaultValue){ return NULL; };

		/** get string specifying the command line for the application, excluding the program name. */
		virtual const char* GetAppCommandLine() { return NULL; };


		/** change the full screen mode, it does not immediately change the device, call UpdateScreenMode() to update the device. */
		virtual void GetScreenResolution(Vector2* pOut){ };

		virtual void SetScreenResolution(const Vector2& vSize){};

		virtual float GetFPS() { return 30.f; };

		/**
		* we will automatically release singleton object when app stops, in the verse order when object is added to the pool. 
		*/
		virtual CRefCounted* AddToSingletonReleasePool(CRefCounted* pObject) { return pObject; };

		/** server mode has no user interface */
		virtual bool IsServerMode() { return true; };
		

		/** whether there is closing request in the message queue. In windows, the user clicks the close button.
		* For long running task in the main thread, it is recommended to periodically check this value to end the task prematurely if necessary.
		*/
		virtual bool HasClosingRequest() { return false; };
		virtual void SetHasClosingRequest(bool val) {};

		/** load NPL package from a disk folder.
		* it will first search the dev folder, then the current folder, and then the executable folder and all of its parent folders.
		* Once the folder is found, it is added to the global search path.
		* @param sFilePath: for example, "npl_packages/main/" is always loaded on start up.
		* @param pOutMainFile: output of the actual folder name or a main loader file path in the main loader.
		*/
		virtual bool LoadNPLPackage(const char* sFilePath, std::string * pOutMainFile = NULL) { return false; };

		/** render the current frame and does not return until everything is presented to screen.
		* this function is usually used to draw the animated loading screen. */
		virtual bool ForceRender() { return false; };

		/** get the NPL bin directory (main executable directory). this one ends with "/" */
		virtual const char* GetModuleDir() { return NULL; };

		/* whether the window size is fixed. */
		PE_DEPRECATED_ATTRIBUTE virtual void FixWindowSize(bool fixed) = 0;

		/** get the attribute object of the main ParaEngine interface */
		virtual IAttributeFields* GetAttributeObject() { return NULL; }

		/** Show or hide current window. This function only works if IsFullScreenMode() is false.
		 */
		virtual void ShowWindow(bool bShow) = 0;


		/** set the maximum UI resolution size. if the backbuffer is larger than this, we will use automatically use UI scaling
		 * for example, if maximum width is 1024, and backbuffer it 1600, then m_fUIScalingX will be automatically set to 1024/1600.
		 * calling this function will cause OnSize() and UpdateBackbufferSize() to be called. Actually it calls SetUIScale()
		 * [main thread only]
		 * @param nWidth: the new width.
		 * @param nHeight: the new height.
		 * @param bAutoUIScaling: default to true. whether we will automatically recalculate the UI scaling accordingly with regard to current backbuffer size.
		 */
		virtual void SetMaxUIResolution(int nWidth, int nHeight, bool bAutoUIScaling = true) = 0;

		virtual bool AppHasFocus() = 0;

		virtual bool FrameMove(double fTime) = 0;

		virtual bool FinalCleanup() = 0;



		virtual void GetStats(string& output, DWORD dwFields) = 0;

		virtual bool WriteRegStr(const string& root_key, const string& sSubKey, const string& name, const string& value) = 0;

		virtual void SetAutoLowerFrameRateWhenNotFocused(bool bEnabled) = 0;

		virtual const char* ReadRegStr(const string& root_key, const string& sSubKey, const string& name) = 0;


		virtual bool WriteRegDWORD(const string& root_key, const string& sSubKey, const string& name, DWORD value) = 0;


		virtual DWORD ReadRegDWORD(const string& root_key, const string& sSubKey, const string& name) = 0;


		virtual bool GetAutoLowerFrameRateWhenNotFocused() = 0;

		virtual void SetToggleSoundWhenNotFocused(bool bEnabled) = 0;

		virtual bool GetToggleSoundWhenNotFocused() = 0;

		virtual CViewportManager* GetViewportManager() = 0;

		virtual void setIMEKeyboardState(bool bOpen, bool bMoveView = false, int ctrlBottom = -1, const string& editParams="") = 0;

		virtual void SetWindowMaximized(bool isMaximized) {};
		virtual bool IsWindowMaximized() { return false; };
		virtual void GetVisibleSize(Vector2* pOut) {};

		virtual void SetLandscapeMode(std::string landscapeMode) {}
		virtual std::string GetLandscapeMode() { return "auto"; }
		virtual bool IsRotateScreen() { return false; }
	};

}
