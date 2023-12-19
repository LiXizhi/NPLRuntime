#pragma once
//-----------------------------------------------------------------------------
// File: D3DApp.h
//
// Desc: Base application class based on the Direct3D samples framework library.
//
//-----------------------------------------------------------------------------
#include "IParaEngineApp.h"

/** 
* A base class for creating D3D device based application. 
*/
class CD3DApplication
{
public:
	virtual         ~CD3DApplication()                         { }

protected:
	// Internal error handling function
    HRESULT DisplayErrorMsg( HRESULT hr, DWORD dwType );

    // Internal functions to manage and render the 3D scene
    static bool ConfirmDeviceHelper( D3DCAPS9* pCaps, VertexProcessingType vertexProcessingType, D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat );
    bool    FindBestWindowedMode( bool bRequireHAL, bool bRequireREF );
    bool    FindBestFullscreenMode( bool bRequireHAL, bool bRequireREF );
    HRESULT LaunchReadme();
    HRESULT ChooseInitialD3DSettings();
    HRESULT Initialize3DEnvironment();
	/** this function is called whenever the main window size changes. It may need to adjust/strech the d3d devices and coordinate systems. 
	* @param bUpdateSizeOnly: if true, we will only update the d3d device. 
	*/
    HRESULT HandlePossibleSizeChange(bool bUpdateSizeOnly = false);
    HRESULT Reset3DEnvironment();
    HRESULT ToggleFullscreen();
    HRESULT ForceWindowed();
    HRESULT UserSelectNewDevice();
    void    Cleanup3DEnvironment();
	/** frame move and render
	* @param bForceRender: if true, it will force frame move and render the scene. if not, it will
	* internally use a frame rate controller that maintain the frame rate at 30 fps, no matter who often this function is called. */
    HRESULT Render3DEnvironment(bool bForceRender=false);
    virtual void BuildPresentParamsFromSettings();
    virtual HRESULT AdjustWindowForChange();
    virtual void UpdateStats();

    // Overridable functions for the 3D scene created by the app
    virtual HRESULT ConfirmDevice(D3DCAPS9*,DWORD,D3DFORMAT,D3DFORMAT) { return S_OK; }
    virtual HRESULT OneTimeSceneInit()                         { return S_OK; }
    virtual HRESULT InitDeviceObjects()                        { return S_OK; }
    virtual HRESULT RestoreDeviceObjects()                     { return S_OK; }
    virtual HRESULT FrameMove()                                { return S_OK; }
    virtual HRESULT Render()                                   { return S_OK; }
    virtual HRESULT InvalidateDeviceObjects()                  { return S_OK; }
    virtual HRESULT DeleteDeviceObjects()                      { return S_OK; }
    virtual HRESULT FinalCleanup()                             { return S_OK; }
	/** update view port by backbuffer size. */
	virtual bool UpdateViewPort();

	/** this function is called per frame, in most cases, it will render the 3d scene and frame move. 
	* call this as often as one like internally it will use a timer to best fit the interval. 
	*/
	virtual HRESULT DoWork();

	
public:
    virtual void    Pause( bool bPause );
	virtual bool	IsPaused();
    
	/** force present the scene. */
	HRESULT PresentScene();

	void    EnablePassiveRendering( bool bEnable ) { m_bPassiveRendering = bEnable;};
	bool	IsPassiveRenderingEnabled( ) { return m_bPassiveRendering; };

	/** disable 3D rendering, do not present the scene. 
	* This is usually called before and after we show a standard win32 window during full screen mode, such as displaying a flash window 
	* @param bEnable: true to enable.
	*/
	void Enable3DRendering(bool bEnable) {m_bEnable3DRendering = bEnable;};

	/** whether 3D rendering is enabled, do not present the scene. 
	* This is usually called before and after we show a standard win32 window during full screen mode, such as displaying a flash window */
	bool Is3DRenderingEnabled() {return m_bEnable3DRendering;};

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
	virtual HRESULT CreateFromD3D9Device(IDirect3DDevice9* pD3dDevice, IDirect3DSwapChain9* apSwapChain);

	virtual int     Run(HINSTANCE hInstance);

	ParaEngine::PEAppState GetAppState() {return m_nAppState;}
	void SetAppState(ParaEngine::PEAppState state) {m_nAppState = state;}

	/** whether the d3d device is managed externally */
	bool IsExternalD3DDevice(){return m_bIsExternalD3DDevice;}

protected:
	// Internal constructor
	CD3DApplication();

	CD3DEnumeration   m_d3dEnumeration;
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
	/** if this is true, the directX is not needed to run the application. this is useful for server mode application.
	* by default, it is false. 
	*/
	bool			  m_bDisableD3D;

	/// passive rendering, it will not render the scene, but simulation and time remains the same. Default is false
	bool              m_bPassiveRendering;

	/// whether to render 3d scene and present to screen. 
	bool              m_bEnable3DRendering;

	// Main objects used for creating and rendering the 3D scene
	D3DPRESENT_PARAMETERS m_d3dpp;         // Parameters for CreateDevice/Reset
	HWND              m_hWnd;              // The main app window
	HWND              m_hWndFocus;         // The D3D focus window (usually same as m_hWnd)
	HMENU             m_hMenu;             // App menu bar (stored here when fullscreen)
	LPDIRECT3D9       m_pD3D;              // The main D3D object
	LPDIRECT3DDEVICE9 m_pd3dDevice;        // The D3D rendering device
	IDirect3DSwapChain9* m_pd3dSwapChain;
	D3DCAPS9          m_d3dCaps;           // Caps for the device
	D3DSURFACE_DESC   m_d3dsdBackBuffer;   // Surface desc of the backbuffer
	DWORD             m_dwCreateFlags;     // Indicate sw or hw vertex processing
	DWORD             m_dwWindowStyle;     // Saved window style for mode switches
	RECT              m_rcWindowBounds;    // Saved window bounds for mode switches
	RECT              m_rcWindowClient;    // Saved client area size for mode switches
	int				  m_nClientWidth;
	int				  m_nClientHeight;

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

	/** whether the main rendering window is an externally provided window or not. This will be true if SetMainWindow() is called. */
	bool			  m_bIsExternalWindow;

	bool m_bIsExternalD3DDevice;
	/** application state */
	ParaEngine::PEAppState m_nAppState;
};
