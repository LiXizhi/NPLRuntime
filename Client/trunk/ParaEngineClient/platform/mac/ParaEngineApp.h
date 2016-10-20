#pragma once
#include "IAttributeFields.h"
#include "ParaEngineAppBase.h"

// forward declare
namespace ParaEngine
{
	class CAISimulator;
	class CGUIRoot;
	class CViewportManager;
}

namespace cocos2d
{
	class EventCustom;
	class EventListenerCustom;
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
	* Note: ParaEngine is designed to be manipulated through the NPL scripting interface
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
	class CParaEngineApp : public CParaEngineAppBase
	{
	public:
		/**
		* @param lpCmdLine: command line arguments. we support the following argument at the moment
		*  - bootstappper="config/bootstrapper.xml"
		*/
		CParaEngineApp(const char*  lpCmdLine = NULL);
		virtual ~CParaEngineApp();

		/** This is the first function that should be called when acquiring the IParaEngineApp interface.
		* call this function to start the application. Rendering window and devices are not created, one need to call Create() instead.
		* @param sCommandLine: the command line parameter
		*/
		virtual HRESULT StartApp(const char* sCommandLine = 0);

		virtual void InitSystemModules();

		void BootStrapAndLoadConfig();

		/** This is the last function that should be called. It is usually called just before process exit.
		*/
		virtual HRESULT StopApp();

		/** set the hWnd on to which we will render and process window messages.
		* this function should be called prior to Create().
		* @note: the rendering device size will use the client area of the input window
		* @param hWnd: the Window on to which we will render.
		* @param bIsExternalWindow: this is always true, unless for the default window used by ParaEngine when no window is created by the user.
		*/
		virtual void SetMainWindow(HWND hWnd, bool bIsExternalWindow = true)  { };
		virtual HWND GetMainWindow()  { return 0; };

		/** only call this function if one does not want to manage game loop externally. */
		virtual int Run(HINSTANCE hInstance);;

		/** this function is called per frame, in most cases, it will render the 3d scene and frame move.
		* call this as often as one like internally it will use a timer to best fit the interval.
		*/
		virtual HRESULT DoWork()  { return 0; };

		/** create from an existing d3d device. This is an advanced function to replaced the default render device.
		* and caller is responsible for managing device life time. The external caller must call InitDeviceObjects(), RestoreDeviceObjects(), InvalidateDeviceObjects(), DeleteDeviceObjects() at proper time
		*/
		virtual HRESULT CreateFromD3D9Device(IDirect3DDevice9* pD3dDevice, IDirect3DSwapChain9* apSwapChain)  { return 0; };

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

		/** create d3d render device based on the current main window.
		* Use this function to create a new render device automatically.
		*/
		virtual HRESULT Create(HINSTANCE hInstance = 0)  { return 0; };

		/** Frame move and render a frame during idle time (no messages are waiting). Call this function during CPU idle time.
		* internally it uses a timer to control frame rates, so it is safe to call this as often as one like.
		* @param bForceRender: if true, it will force frame move and render the scene. if not, it will
		* internally use a frame rate controller that maintain the frame rate at 30 fps, no matter who often this function is called.
		*/
		virtual HRESULT Render3DEnvironment(bool bForceRender = false);;

		void UpdateStats(double fTime);

		/** the window message processor. One needs send all messages belonging to the main window to this function, after calling Create().
		* @note: the main rendering thread can be a different thread than the window proc thread.
		* @param bCallDefProcedure: whether we will call the ::DefWindowProdure().
		* @return: 0 if message was not processed. 1 if message is processed. -1 if message is processed by can be passed on to other processor.
		*/
		virtual LRESULT MsgProcWinThread(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool bCallDefProcedure = true)  { return 0; };

		/*** process win thread messages without a hWnd. Such messages are usually invoked by PostWinThreadMessage() from other threads. */
		virtual LRESULT MsgProcWinThreadCustom(UINT uMsg, WPARAM wParam, LPARAM lParam)  { return 0; };

		/** Send a RAW win32 message the application to be processed in the next main thread update interval.
		* This function can be called from any thread. It is also used by the windows procedure thread to dispatch messages to the main processing thread.
		*/
		virtual LRESULT SendMessageToApp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)  { return 0; };

		/** post a raw win32 message from any thread to the thread on which hWnd is created. */
		virtual bool PostWinThreadMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)  { return false; };

		/** get a message from the application message queue and remove it from the queue. This function is mostly
		used internally by the main thread.
		* @param pMsg: the receiving message
		* @return true if one message is fetched. or false if there is no more messages in the queue.
		*/
		virtual bool GetMessageFromApp(CWinRawMsg* pMsg)  { return false; };

		/**
		* handle a message in the main application thread.
		*/
		virtual LRESULT MsgProcApp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)  { return 0; };

		/**  passive rendering, it will not render the scene, but simulation and time remains the same. Default is false*/
		virtual void    EnablePassiveRendering(bool bEnable) { };

		/**  passive rendering, it will not render the scene, but simulation and time remains the same. Default is false*/
		virtual bool	IsPassiveRenderingEnabled()  { return false; };

		/** The BringWindowToTop current window to the top of the Z order. This function only works if IsFullScreenMode() is false.
		*/
		virtual void BringWindowToTop() { };

		/** whether the user can close the window (sending WM_CLOSE message). Default to true.
		* When set to false, the scripting interface will receive WM_CLOSE message via system event. And can use ParaEngine.Exit() to quit the application after user confirmation, etc.
		*/
		virtual void SetAllowWindowClosing(bool bAllowClosing) { };
		virtual bool IsWindowClosingAllowed()  { return false; };

		/** whether to use full screen mode, it does not immediately change the device, call UpdateScreenMode() to update the device. */
		virtual void SetFullScreenMode(bool bFullscreen){};
		virtual bool IsFullScreenMode(){return false;};

		/** turn on/off menu */
		virtual void ShowMenu(bool bShow){};

		/** change the full screen mode, it does not immediately change the device, call UpdateScreenMode() to update the device. */
		virtual Vector2 GetScreenResolution();
		virtual void SetScreenResolution(const Vector2& vSize);

		/** anti-aliasing for both windowed and full screen mode. it does not immediately change the device, call UpdateScreenMode() to update the device.*/
		virtual int GetMultiSampleType(){ return 0;};
		virtual void SetMultiSampleType(int nType){};

		/** anti-aliasing for both windowed and full screen mode. it does not immediately change the device, call UpdateScreenMode() to update the device.*/
		virtual int GetMultiSampleQuality(){return 0;};
		virtual void SetMultiSampleQuality(int nType){};

		/** call this function to update changes of FullScreen Mode and Screen Resolution. */
		virtual bool UpdateScreenMode(){return false;};

		// ParaEngine pipeline routines
		/** switch to either windowed mode or full screen mode. */
		virtual bool SetWindowedMode(bool bWindowed){return false;};
		/** return true if it is currently under windowed mode. */
		virtual bool IsWindowedMode(){return true;};
		/** set the window title when at windowed mode */
		virtual void SetWindowText(const char* pChar);
		/** get the window title when at windowed mode */
		virtual const char* GetWindowText();


		/** write the current setting to config file. Such as graphics mode and whether full screen, etc.
		* config file at ./config.txt will be automatically loaded when the game engine starts.
		* @param sFileName: if this is "", it will be the default config file at ./config.txt
		*/
		virtual void WriteConfigFile(const char* sFileName);

		/** get whether ParaEngine is loaded from config/config.new.txt. if this is true, we need to ask the user to save to config/config.txt. This is usually done at start up. */
		virtual bool HasNewConfig();

		/** set whether ParaEngine is loaded from config/config.new.txt. if this is true, we need to ask the user to save to config/config.txt. This is usually done at start up. */
		virtual void SetHasNewConfig(bool bHasNewConfig);


		/** get the window creation size in default application config. */
		virtual void GetWindowCreationSize(int * pWidth, int * pHeight);

		/** whether the application is active or not. */
		virtual bool IsAppActive();

		/** set the minimum UI resolution size. if the backbuffer is smaller than this, we will use automatically use UI scaling
		* for example, if minimum width is 1024, and backbuffer it 800, then m_fUIScalingX will be automatically set to 1024/800.
		* calling this function will cause OnSize() and UpdateBackbufferSize() to be called. Actually it calls SetUIScale()
		* [main thread only]
		* @param nWidth: the new width.
		* @param nHeight: the new height.
		* @param bAutoUIScaling: default to true. whether we will automatically recalculate the UI scaling accordingly with regard to current backbuffer size.
		*/
		virtual void SetMinUIResolution(int nWidth, int nHeight, bool bAutoUIScaling = true) { };

		/** change the full screen mode, it does not immediately change the device, call UpdateScreenMode() to update the device. */
		virtual void GetResolution(float* pX, float* pY);;
		virtual void SetResolution(float x, float y);;

		/** get the NPL runtime system associate with the application. NPL provides communication framework across different language systems. */
		virtual NPL::INPLRuntime* GetNPLRuntime();
		virtual bool IsSlateMode();
		/** server mode has no user interface */
		virtual bool IsServerMode() { return false; };

		float GetFPS() const;


		/** init application */
		HRESULT Init(HWND* pHWND);

		void InitAudioEngine();

		/**
		* This function should be called only once when the application start, one can initialize game objects here.
		* @param pHWND:a pointer to the handle of the current application window.
		*/
		HRESULT OneTimeSceneInit(HWND* pHWND);

		/**
		* This callback function will be called once at the beginning of every frame. This is the
		* best location for your application to handle updates to the scene, but is not
		* intended to contain actual rendering calls, which should instead be placed in the
		* OnFrameRender callback.
		* @param fTime: Current time elapsed.
		*/
		virtual HRESULT FrameMove(double fTime);

		/**
		* This function should be called only once when the application end, one can destroy game objects here.
		*/
		HRESULT FinalCleanup();

		/** process game input.*/
		void HandleUserInput();

		/** Send the exit message, so that the game engine will prepare to exit in the next frame.
		* this is the recommended way of exiting application.
		* this is mainly used for writing test cases. Where a return value of 0 means success, any other value means failure.
		*/
		virtual void Exit(int nReturnCode = 0);

		virtual ParaEngine::PEAppState GetAppState();
		virtual void SetAppState(ParaEngine::PEAppState state);

		CViewportManager* GetViewportManager();

#ifdef USE_OPENGL_RENDERER
		void listenRendererRecreated(cocos2d::EventCustom* event);
		cocos2d::EventListenerCustom* m_rendererRecreatedListener;
		// from cocos platform
		void applicationDidEnterBackground();
		void applicationWillEnterForeground();
#endif

	protected:

		// whether we are loaded from config/config.new.txt.
		bool m_bHasNewConfig;

		// whether start as server mode
		bool	m_bServerMode;

		ParaEngine::PEAppState m_nAppState;

		int m_nScreenWidth;
		int m_nScreenHeight;

		/** 2d gui root */
		ref_ptr<CGUIRoot> m_pGUIRoot;
		/** 3d scene root object */
		ref_ptr<CSceneObject>		  m_pRootScene;
		/** viewport */
		ref_ptr<CViewportManager>	  m_pViewportManager;
		/** asset manager */
		ref_ptr<CParaWorldAsset>	  m_pParaWorldAsset;

		double m_fTime;
		float m_fFPS;

		bool m_bIsAppActive;
	};

}
