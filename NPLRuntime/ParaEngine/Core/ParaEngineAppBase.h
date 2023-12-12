#pragma once
#include "IParaEngineApp.h"
#include "CommandLineParams.h"
#include "ParaVector2.h"
#include "FrameRateController.h"

namespace ParaEngine
{
	class ParaTimer;
	class CObjectAutoReleasePool;

	/* base implementation of ParaEngine APP, shared by both mobile and pc version.  */
	class CParaEngineAppBase : public IParaEngineApp, public CCommandLineParams, public IObject
	{

	public:
		CParaEngineAppBase();
		CParaEngineAppBase(const char* sCmd);
		virtual ~CParaEngineAppBase();
		static IParaEngineApp* GetInstance();
		virtual IRenderWindow* GetRenderWindow() override;
		virtual bool InitApp(IRenderWindow* pWindow, const char* sCommandLine /* = nullptr */) override;
		virtual bool StartApp() override;
		virtual void StopApp() override;

		virtual void Enable3DRendering(bool bEnable) override;
		virtual bool Is3DRenderingEnabled()override;
		/** whether the last mouse input is from touch or mouse. by default it is mouse mode. */
		virtual bool IsTouchInputting() override;
		virtual bool IsSlateMode() override;

		virtual HRESULT DoWork() override;

		virtual bool FrameMove(double fTime) override;
		virtual void OnPause(); // device lost
		virtual void OnResume();
		virtual void OnRendererRecreated(IRenderWindow* renderWindow);
		virtual void OnRendererDestroyed();

		virtual HRESULT InitDeviceObjects() override;
		virtual HRESULT RestoreDeviceObjects() override;
		virtual HRESULT InvalidateDeviceObjects() override;
		virtual HRESULT DeleteDeviceObjects() override;

		/** whether the application is active or not. */
		virtual bool IsAppActive() override;

		virtual void GameToClient(int& inout_x, int & inout_y, bool bInBackbuffer = true) override;
		virtual void ClientToGame(int& inout_x, int & inout_y, bool bInBackbuffer = true) override;
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

		virtual void onCmdLine(const std::string& cmd);

		virtual void setIMEKeyboardState(bool bOpen, bool bMoveView = false, int ctrlBottom = -1, const string& editParams="") override;
		
		virtual void SetLandscapeMode(std::string landscapeMode);
		virtual std::string GetLandscapeMode();

		/** return true if this is a render tick, otherwise false.
		* @param pNextInterval: main_loop timer interval.
		* @return frameDelta. if this is bigger than 0, we will render a frame.
		*/
		int CalculateRenderTime(double* pNextInterval);

		double GetAppTime();
		double GetElapsedTime();
	protected:
		virtual void SetTouchInputting(bool bTouchInputting) override;
		void AutoSetLocale();
		/** call this function at the end of the frame. */
		virtual void OnFrameEnded();
		virtual void WriteToLog(const char* sFormat, ...) override;
		virtual void AppLog(const char* sFormat) override;
		/** set the current working directory. This function is called in the constructor to ensure that all IO directs to the right dir. */
		virtual bool FindParaEngineDirectory(const char* sHint = NULL);
		void BootStrapAndLoadConfig();
		void InitSystemModules();
		void InitRenderEnvironment();
		void ResetRenderEnvironment();

		virtual float GetFPS() override;
		virtual void UpdateFrameStats(double fTime);
		
		void Render();
		void HandleUserInput();
		/** this function is called whenever the application is disabled or enabled. usually called when receiving the WM_ACTIVATEAPP message.
		* [main thread only]
		*/
		virtual void ActivateApp(bool bActivate) override;

		virtual LifetimeType	LifetimeControl() override { return wantsRelease;  };
		virtual BaseInterface*	AcquireInterface() override { addref();	return(BaseInterface*)this; };
		virtual void			ReleaseInterface() override { if (delref()) { DeleteInterface(); } };
		virtual void DeleteInterface() override  { delete this; }
        
        void setRenderEnabled(bool b) { m_bRender = b; }
        bool getRenderEnabled () const {return m_bRender;};

	protected:
		static IParaEngineApp* g_pCurrentApp;
		bool m_bEnable3DRendering;
		bool m_isTouching;
		bool m_hasClosingRequest;
		/** the application exit code or return code. 0 means success. otherwise means a failure. */
		int m_nReturnCode;
		/** a pool of registered singleton object. */
		CObjectAutoReleasePool* m_pSingletonReleasePool;
		/** packages/ directory path */
		std::string m_sPackagesDir;
		/** bin/ module path */
		std::string m_sModuleDir;
		/** initial working directory*/
		std::string m_sInitialWorkingDir;
		std::string m_sLandscapeMode;
		/** application state */
		PEAppState m_nAppState;
		ref_ptr<CGUIRoot> m_pGUIRoot;
		/** 3d scene root object */
		ref_ptr<CSceneObject>		  m_pRootScene;
		/** viewport */
		ref_ptr<CViewportManager>	  m_pViewportManager;
		/** asset manager */
		ref_ptr<CParaWorldAsset>	  m_pParaWorldAsset;
		ParaTimer* m_Timer;
		IRenderContext* m_pRenderContext;
		IRenderDevice* m_pRenderDevice;
		IRenderWindow* m_pRenderWindow;
		CFrameRateController m_doWorkFRC;
		float			  m_fRefreshTimerInterval; //  in seconds. 
		int				  m_nFrameRateControl;
		double            m_fTime;             // Current time in seconds
		double            m_fElapsedTime;      // Time elapsed since last frame
		float m_fFPS;
		bool m_bActive;
        bool m_bRender;
		bool m_bAudioEngineInitialized;

#pragma region OLD_CODE
	public:
		/** switch to ignore windows size change. default to false.
		* if false, the user is allowed to adjust window size in windowed mode. */
		virtual void SetIgnoreWindowSizeChange(bool bIgnoreSizeChange) override {};

		/** return true if it is currently under windowed mode. */
		virtual bool GetIgnoreWindowSizeChange() override { return false; };

		/** get the module handle, it may be exe or the dll handle, depending on how the main host app is built. */
		HINSTANCE GetModuleHandle() override { return 0; };

		/**
		* Set the frame rate timer interval
		* @param fTimeInterval:  value in seconds. such as 0.033f or 0.01667f
		* 	Passing a value <= 0 to render in idle time.
		* @param nFrameRateControl: 0 for real time, 1 for ideal frame rate at 30 FPS no matter whatever time interval is set.
		*/
		virtual void SetRefreshTimer(float fTimeInterval, int nFrameRateControl = 0) override;

		/** get the refresh timer.
		*/
		virtual float GetRefreshTimer() override;

		/** Get the current ParaEngine app usage.
		* [main thread only]
		* @return see PE_USAGE
		*/
		virtual DWORD GetCoreUsage() override;

		/** Set the current ParaEngine app usage.
		* [main thread only]
		* @param dwUsage: bitwise of PE_USAGE
		*/
		virtual void SetCoreUsage(DWORD dwUsage) override  { };

		/** Get the exit code that will be used when the standalone executable exit.
		* this is mainly used for writing test cases. Where a return value of 0 means success, any other value means failure.
		*/
		virtual void SetReturnCode(int nReturnCode) override;
		virtual int GetReturnCode() override;

		/** show a system message box to the user. mostly about fatal error.  */
		virtual void SystemMessageBox(const std::string& msg) override;

		/** set string specifying the command line for the application, excluding the program name.
		* calling this function multiple times with different command line is permitted. The latter settings will merge and override the previous ones.
		* @param pCommandLine: such as   key="value" key2="value2"
		*/
		virtual void SetAppCommandLine(const char* pCommandLine) override;

		/** get string specifying the command line for the application, excluding the program name. */
		virtual const char* GetAppCommandLine() override;

		/**
		* return a specified parameter value in the command line of the application. If the parameter does not exist, the return value is NULL.
		* @param pParam: key to get
		* @param defaultValue: if the key does not exist, this value will be added and returned. This can be NULL.
		*/
		virtual const char* GetAppCommandLineByParam(const char* pParam, const char* defaultValue) override;
	public:
		/** Send the exit message, so that the game engine will prepare to exit in the next frame.
		* this is the recommended way of exiting application.
		* this is mainly used for writing test cases. Where a return value of 0 means success, any other value means failure.
		*/
		virtual void Exit(int nReturnCode = 0) override;

		virtual ParaEngine::PEAppState GetAppState() override;
		virtual void SetAppState(ParaEngine::PEAppState state) override;

		/**
		* This function should be called only once when the application end, one can destroy game objects here.
		*/
		virtual bool FinalCleanup() override;

		/** only call this function if one does not want to manage game loop externally. */
		virtual int Run(HINSTANCE hInstance) override { return 0; };

		/** create d3d render device based on the current main window.
		* Use this function to create a new render device automatically.
		*/
		PE_DEPRECATED_ATTRIBUTE virtual HRESULT Create(HINSTANCE hInstance = 0)  override { return S_OK; };

		/** Frame move and render a frame during idle time (no messages are waiting). Call this function during CPU idle time.
		* internally it uses a timer to control frame rates, so it is safe to call this as often as one like.
		* @param bForceRender: if true, it will force frame move and render the scene. if not, it will
		* internally use a frame rate controller that maintain the frame rate at 30 fps, no matter who often this function is called.
		*/
		PE_DEPRECATED_ATTRIBUTE virtual HRESULT Render3DEnvironment(bool bForceRender = false) override  { return S_OK; }

		/** the window message processor. One needs send all messages belonging to the main window to this function, after calling Create().
		* @note: the main rendering thread can be a different thread than the window proc thread.
		* @param bCallDefProcedure: whether we will call the ::DefWindowProdure().
		* @return: 0 if message was not processed. 1 if message is processed. -1 if message is processed by can be passed on to other processor.
		*/
		PE_DEPRECATED_ATTRIBUTE virtual LRESULT MsgProcWinThread(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool bCallDefProcedure = true) override { return 0; };

		/*** process win thread messages without a hWnd. Such messages are usually invoked by PostWinThreadMessage() from other threads. */
		PE_DEPRECATED_ATTRIBUTE virtual LRESULT MsgProcWinThreadCustom(UINT uMsg, WPARAM wParam, LPARAM lParam)  override  { return 0; };

		/** Send a RAW win32 message the application to be processed in the next main thread update interval.
		* This function can be called from any thread. It is also used by the windows procedure thread to dispatch messages to the main processing thread.
		*/
		PE_DEPRECATED_ATTRIBUTE virtual LRESULT SendMessageToApp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)  override  { return 0; };

		/** post a raw win32 message from any thread to the thread on which hWnd is created. */
		PE_DEPRECATED_ATTRIBUTE virtual bool PostWinThreadMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)  override  { return false; };

		/** get a message from the application message queue and remove it from the queue. This function is mostly
		used internally by the main thread.
		* @param pMsg: the receiving message
		* @return true if one message is fetched. or false if there is no more messages in the queue.
		*/
		PE_DEPRECATED_ATTRIBUTE virtual bool GetMessageFromApp(CWinRawMsg* pMsg)  override  { return false; };

		/**
		* handle a message in the main application thread.
		*/
		PE_DEPRECATED_ATTRIBUTE virtual LRESULT MsgProcApp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override { return 0; };

		/**  passive rendering, it will not render the scene, but simulation and time remains the same. Default is false*/
		virtual void    EnablePassiveRendering(bool bEnable) override { };

		/**  passive rendering, it will not render the scene, but simulation and time remains the same. Default is false*/
		virtual bool	IsPassiveRenderingEnabled() override { return false; };

		/** The BringWindowToTop current window to the top of the Z order. This function only works if IsFullScreenMode() is false.
		*/
		virtual void BringWindowToTop() override { };

		/** Show or hide current window. This function only works if IsFullScreenMode() is false.
		 */
		virtual void ShowWindow(bool bShow) override { };

		/** whether the user can close the window (sending WM_CLOSE message). Default to true.
		* When set to false, the scripting interface will receive WM_CLOSE message via system event. And can use ParaEngine.Exit() to quit the application after user confirmation, etc.
		*/
		virtual void SetAllowWindowClosing(bool bAllowClosing) override { };
		virtual bool IsWindowClosingAllowed() override { return false; };

		/** whether to use full screen mode, it does not immediately change the device, call UpdateScreenMode() to update the device. */
		virtual void SetFullScreenMode(bool bFullscreen) override {};
		virtual bool IsFullScreenMode() override { return false; };

		/** change the full screen mode, it does not immediately change the device, call UpdateScreenMode() to update the device. */
		virtual void GetScreenResolution(Vector2* pOut) override { *pOut = Vector2::ZERO; };
		virtual void SetScreenResolution(const Vector2& vSize) override {};

		/** anti-aliasing for both windowed and full screen mode. it does not immediately change the device, call UpdateScreenMode() to update the device.*/
		virtual int GetMultiSampleType() override { return 0; };
		virtual void SetMultiSampleType(int nType) override {};

		/** anti-aliasing for both windowed and full screen mode. it does not immediately change the device, call UpdateScreenMode() to update the device.*/
		PE_DEPRECATED_ATTRIBUTE virtual int GetMultiSampleQuality() override { return 0; };
		PE_DEPRECATED_ATTRIBUTE virtual void SetMultiSampleQuality(int nType) override  {};

		/** call this function to update changes of FullScreen Mode and Screen Resolution. */
		virtual bool UpdateScreenMode() override { return false; };

		// ParaEngine pipeline routines
		/** switch to either windowed mode or full screen mode. */
		virtual bool SetWindowedMode(bool bWindowed) override  { return false; };
		/** return true if it is currently under windowed mode. */
		virtual bool IsWindowedMode() override { return true; };
		/** set the window title when at windowed mode */
		virtual void SetWindowText(const char* pChar) override {};
		/** get the window title when at windowed mode */
		virtual const char* GetWindowText() override { return ""; };
		/* whether the window size is fixed. */
		virtual void FixWindowSize(bool fixed) override {};


		/** write the current setting to config file. Such as graphics mode and whether full screen, etc.
		* config file at ./config.txt will be automatically loaded when the game engine starts.
		* @param sFileName: if this is "", it will be the default config file at ./config.txt
		*/
		virtual void WriteConfigFile(const char* sFileName) override  {};

		/** get whether ParaEngine is loaded from config/config.new.txt. if this is true, we need to ask the user to save to config/config.txt. This is usually done at start up. */
		virtual bool HasNewConfig() override  { return false; };

		/** set whether ParaEngine is loaded from config/config.new.txt. if this is true, we need to ask the user to save to config/config.txt. This is usually done at start up. */
		virtual void SetHasNewConfig(bool bHasNewConfig) override  {};


		/** get the window creation size in default application config. */
		virtual void GetWindowCreationSize(int * pWidth, int * pHeight) override {};

		/** set the minimum UI resolution size. if the backbuffer is smaller than this, we will use automatically use UI scaling
		* for example, if minimum width is 1024, and backbuffer it 800, then m_fUIScalingX will be automatically set to 1024/800.
		* calling this function will cause OnSize() and UpdateBackbufferSize() to be called. Actually it calls SetUIScale()
		* [main thread only]
		* @param nWidth: the new width.
		* @param nHeight: the new height.
		* @param bAutoUIScaling: default to true. whether we will automatically recalculate the UI scaling accordingly with regard to current backbuffer size.
		*/
		virtual void SetMinUIResolution(int nWidth, int nHeight, bool bAutoUIScaling = true) override { };

		/** set the maximum UI resolution size. if the backbuffer is larger than this, we will use automatically use UI scaling
		 * for example, if maximum width is 1024, and backbuffer it 1600, then m_fUIScalingX will be automatically set to 1024/1600.
		 * calling this function will cause OnSize() and UpdateBackbufferSize() to be called. Actually it calls SetUIScale()
		 * [main thread only]
		 * @param nWidth: the new width.
		 * @param nHeight: the new height.
		 * @param bAutoUIScaling: default to true. whether we will automatically recalculate the UI scaling accordingly with regard to current backbuffer size.
		 */
		virtual void SetMaxUIResolution(int nWidth, int nHeight, bool bAutoUIScaling = true) override { };

		/** change the full screen mode, it does not immediately change the device, call UpdateScreenMode() to update the device. */
		virtual void GetResolution(float* pX, float* pY) override {};
		virtual void SetResolution(float x, float y)  override {};

		/** get the NPL runtime system associate with the application. NPL provides communication framework across different language systems. */
		virtual NPL::INPLRuntime* GetNPLRuntime();

		/**
		* we will automatically release singleton object when app stops, in the verse order when object is added to the pool.
		*/
		virtual CRefCounted* AddToSingletonReleasePool(CRefCounted* pObject) override;

		/** whether there is closing request in the message queue. In windows, the user clicks the close button.
		* For long running task in the main thread, it is recommended to periodically check this value to end the task prematurely if necessary.
		*/
		virtual bool HasClosingRequest() override;
		virtual void SetHasClosingRequest(bool val) override;

		/** load NPL package from a disk folder.
		* it will first search the dev folder, then the current folder, and then the executable folder and all of its parent folders.
		* Once the folder is found, it is added to the global search path.
		* @param sFilePath: for example, "npl_packages/main/" is always loaded on start up.
		* @param pOutMainFile: output of the actual folder name or a main loader file path in the main loader. 
		*/
		virtual bool LoadNPLPackage(const char* sFilePath, std::string * pOutMainFile = NULL) override;

		/** check if there is bootstrapper file specified at command line, if not it will use NPL code wiki admin app. 
		*/
		virtual bool FindBootStrapper();

		/** parse common command line parameters */
		virtual bool InitCommandLineParams();


		/** render the current frame and does not return until everything is presented to screen.
		* this function is usually used to draw the animated loading screen. */
		virtual bool ForceRender() override;

		/** get the NPL bin directory (main executable directory). this one ends with "/" */
		virtual const char* GetModuleDir() override;

		/** set the hWnd on to which we will render and process window messages.
		* this function should be called prior to Create().
		* @note: the rendering device size will use the client area of the input window
		* @param hWnd: the Window on to which we will render.
		* @param bIsExternalWindow: this is always true, unless for the default window used by ParaEngine when no window is created by the user.
		*/
		PE_DEPRECATED_ATTRIBUTE virtual void SetMainWindow(HWND hWnd, bool bIsExternalWindow = true) override {};
		PE_DEPRECATED_ATTRIBUTE virtual HWND GetMainWindow() override { return nullptr;  };

		/** create from an existing d3d device. This is an advanced function to replaced the default render device.
		* and caller is responsible for managing device life time. The external caller must call InitDeviceObjects(), RestoreDeviceObjects(), InvalidateDeviceObjects(), DeleteDeviceObjects() at proper time
		*/
		PE_DEPRECATED_ATTRIBUTE virtual HRESULT CreateFromD3D9Device(IDirect3DDevice9* pD3dDevice, IDirect3DSwapChain9* apSwapChain) override { return S_OK;  };


		/** init the application. no need to be called unless in a service where no rendering devices are created. */
		PE_DEPRECATED_ATTRIBUTE virtual HRESULT Init(HWND* pHWND = 0)  override { return S_OK; };

		/** turn on/off menu */
		PE_DEPRECATED_ATTRIBUTE virtual void ShowMenu(bool bShow) override {};

		/** get the current mouse cursor position.
		* @param pX: out
		* @param pY: out
		* @param bInBackbuffer: if true, it will scale the output according to the ratio of back buffer and current window size.
		*/
		PE_DEPRECATED_ATTRIBUTE virtual void GetCursorPosition(int* pX, int * pY, bool bInBackbuffer = true) override {};

		/** obsoleted function: */
		PE_DEPRECATED_ATTRIBUTE virtual int32 GetTouchPointX() override { return 0;  };
		PE_DEPRECATED_ATTRIBUTE virtual int32 GetTouchPointY() override { return 0; };

		/** get the attribute object of the main ParaEngine interface */
		virtual IAttributeFields* GetAttributeObject() override;


	public:
		/** managing multiple 3d views */
		CViewportManager * GetViewportManager() override;

		virtual void VerifyCommandLine(const char* sCommandLine, std::string &strCmd);

	protected:
		/** shared init called in constructor.  */
		void DestroySingletons();

		/** we will load all packages that matches the following pattern in the order given by their name,
		* such that "main_001.pkg" is always loaded before "main_002.pkg" 
		* we will skip packages *_32bits.pkg when in 64bits; and skip *_64bits.pkg in 32bits; packages without above suffix will be loaded in both version. 
		*/
		void LoadPackages();
		void LoadPackagesInFolder(const std::string& sPkgFolder);
		
		/** register a given class. */
		void RegisterObjectClass(IAttributeFields* pObject);

		/** register any custom classes */
		void RegisterObjectClasses();
	};
#pragma endregion OLD_CODE

}
