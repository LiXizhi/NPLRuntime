
#include "ParaEngine.h"

#pragma region Headers

#include "resource.h"
#include "2dengine/GUIRoot.h"
#include "2dengine/GUIDirectInput.h"
#include "D3DWindowUtil.h"
#include "FrameRateController.h"
#include "MiscEntity.h"
#include "d3dapp.h"
#include "D3D9RenderContext.h"
#include "D3D9RenderDevice.h"
#include "WindowsRenderWindow.h"
#include <functional>


using namespace ParaEngine;


CFrameRateController g_doWorkFRC(CFrameRateController::FRC_CONSTANT_OR_BELOW);

CD3DApplication::CD3DApplication()
{

	SetAppState(PEAppState_None);

    m_pd3dDevice        = NULL;
	m_pd3dSwapChain     = NULL;
    m_hWnd              = NULL;
    m_hWndFocus         = NULL;
    m_hMenu             = NULL;
    m_bWindowed         = true;
    m_bActive           = false;
    m_bDeviceLost       = false;
    m_bMinimized        = false;
    m_bMaximized        = false;
    m_bIgnoreSizeChange = false;
    m_bDeviceObjectsInited = false;
    m_bDeviceObjectsRestored = false;
    m_dwCreateFlags     = 0;
	m_bPassiveRendering = false;
	m_bEnable3DRendering = true;
    m_bFrameMoving      = true;
    m_bSingleStep       = false;
    m_fTime             = 0.0f;
    m_fElapsedTime      = 0.0f;
    m_fFPS              = 0.0f;
    m_strDeviceStats[0] = _T('\0');
    m_strFrameStats[0]  = _T('\0');
	m_fRefreshTimerInterval = -1.f;
	m_nFrameRateControl = 0;

    m_strWindowTitle    = _T("D3D9 Application");
    m_dwCreationWidth   = 400;
    m_dwCreationHeight  = 300;
    m_bShowCursorWhenFullscreen = true;
    m_bStartFullscreen  = false;
    m_bCreateMultithreadDevice = true;
    m_bAllowDialogBoxMode = false;

	m_pRenderDevice = NULL;
	m_pRenderContext = nullptr;

	memset(&m_d3dSettings, 0 , sizeof(m_d3dSettings));
	
    Pause( true ); // Pause until we're ready to render

    // When m_bClipCursorWhenFullscreen is true, the cursor is limited to
    // the device window when the app goes fullscreen.  This prevents users
    // from accidentally clicking outside the app window on a multimon system.
    // This flag is turned off by default for debug builds, since it makes 
    // multimon debugging difficult.
#if defined(_DEBUG) || defined(DEBUG)
    m_bClipCursorWhenFullscreen = false;
#else
    m_bClipCursorWhenFullscreen = true;
#endif
}

#pragma endregion Headers

#pragma region DevicesAndEvents



//-----------------------------------------------------------------------------
// Name: Create()
// Desc: Here's what this function does:
//       - Checks to make sure app is still active (if fullscreen, etc)
//       - Checks to see if it is time to draw with DXUtil_Timer, if not, it just returns S_OK
//       - Calls FrameMove() to recalculate new positions
//       - Calls Render() to draw the new frame
//       - Updates some frame count statistics
//       - Calls m_pd3dDevice->Present() to display the rendered frame.
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::Create()
{
	HRESULT hr;

	if( m_hWnd == NULL)
	{
		OUTPUT_LOG("error: render window is not created when creating Create()\n");
		return E_FAIL;
	}

	SetAppState(PEAppState_Device_Created);
	

		m_pRenderContext = D3D9RenderContext::Create();


		if(m_pRenderContext == NULL )
			return DisplayErrorMsg( D3DAPPERR_NODIRECT3D, MSGERR_APPMUSTEXIT );
	
	
    // The focus window can be a specified to be a different window than the
    // device window.  If not, use the device window as the focus window.
    if( m_hWndFocus == NULL )
        m_hWndFocus = m_hWnd;

	OUTPUT_LOG("DEBUG: main thread wnd handle : %d\n", m_hWndFocus);

    // Initialize the application timer
    DXUtil_Timer( TIMER_START );

    // Initialize the app's custom scene stuff
    if( FAILED( hr = OneTimeSceneInit() ) )
    {
		delete m_pRenderContext;
		m_pRenderContext = nullptr;
        return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
    }


		// Initialize the 3D environment for the app
		if( FAILED( hr = Initialize3DEnvironment() ) )
		{
			delete m_pRenderContext;
			m_pRenderContext = nullptr;
			return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
		}
	

    // The app is ready to go
    Pause( false );

	SetAppState(PEAppState_Ready);
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Render3DEnvironment()
// Desc: Here's what this function does:
//       - Checks to make sure app is still active (if fullscreen, etc)
//       - Checks to see if it is time to draw with DXUtil_Timer, if not, it just returns S_OK
//       - Calls FrameMove() to recalculate new positions
//       - Calls Render() to draw the new frame
//       - Updates some frame count statistics
//       - Calls m_pd3dDevice->Present() to display the rendered frame.
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::Render3DEnvironment(bool bForceRender)
{
	HRESULT hr = S_OK;

	if( m_bDeviceLost )
	{
		// Test the cooperative level to see if it's okay to render
		if( FAILED( hr = m_pd3dDevice->TestCooperativeLevel() ) )
		{
			// If the device was lost, do not render until we get it back
			if( D3DERR_DEVICELOST == hr )
				return S_OK;

			// Check if the device needs to be reset.
			if( D3DERR_DEVICENOTRESET == hr )
			{

				
				OUTPUT_LOG("TestCooperativeLevel needs to reset device with D3DERR_DEVICENOTRESET\n");
				
				if( FAILED( hr = Reset3DEnvironment() ) )
				{
					// This fixed a strange issue where d3d->reset() returns D3DERR_DEVICELOST. Perhaps this is due to strange. 
					// I will keep reset for 5 seconds, until reset() succeed, otherwise we will exit application.  
					for (int i=0;i<5 && hr == D3DERR_DEVICELOST; ++i)
					{
						::Sleep(1000);
						if( SUCCEEDED( hr = Reset3DEnvironment() ) )
						{
							OUTPUT_LOG("TestCooperativeLevel successfully reset devices.\n");
							return hr;
						}
					}
					return DisplayErrorMsg( D3DAPPERR_RESETFAILED, MSGERR_APPMUSTEXIT );
				}
			}
			return hr;
		}
		m_bDeviceLost = false;
	}

	// Get the app's time, in seconds. Skip rendering if no time elapsed
	double fAppTime        = DXUtil_Timer( TIMER_GETAPPTIME );
	double fElapsedAppTime = DXUtil_Timer( TIMER_GETELAPSEDTIME );

	//////////////////////////////////////////////////////////////////////////
	//
	// frame rate control at 30 FPS
	//
	//////////////////////////////////////////////////////////////////////////

	static double fRenderTime = 0;
	static double fConstTime = 0;
	bool bUseIdealFrameRate = (m_nFrameRateControl == 1);

	fRenderTime += fElapsedAppTime;
	fConstTime += fElapsedAppTime;
	if(!bUseIdealFrameRate || (fConstTime >= IDEAL_FRAME_RATE) || bForceRender )
	{
		m_fElapsedTime = bUseIdealFrameRate ? fRenderTime : fElapsedAppTime;


		if(bForceRender)
		{
			fConstTime = 0;
		}
		else
		{
			// TRICKY: this helps to smooth the frame rate to IDEAL_FRAME_RATE, when force render is false
			fConstTime -= IDEAL_FRAME_RATE;
			if(fConstTime>=IDEAL_FRAME_RATE) // if 0.5 seconds is passed, we will catch up 
				fConstTime = 0;
		}
		fRenderTime = 0;

		if( ( 0.0f == fElapsedAppTime ) && m_bFrameMoving )
			return S_OK;

		if(bUseIdealFrameRate)
		{
			// only use ideal frame rate if interval is smaller than it.
			if(m_fRefreshTimerInterval<IDEAL_FRAME_RATE)
			{
				// TRICKY: this fixed a bug of inaccurate timing, we will assume a perfect timing here, but correct it if it differentiate too much from   
				// FrameMove (animate) the scene, frame move is called before Render(), since the frame move may contain ForceRender() calls
				// Store the time for the app
				if(m_fTime < (fAppTime-IDEAL_FRAME_RATE)) // if 0.5 seconds is passed, we will catch up 
					m_fTime = fAppTime;
				else
					m_fTime += IDEAL_FRAME_RATE;
			}
			else
			{
				// if we special refresh timer to be 0.03334 or bigger, we shall use absolute time. 
				m_fTime = fAppTime;
			}
		}
		else
		{
			m_fTime = fAppTime;
		}

		// OUTPUT_LOG("%f:%f, %f, %f\n", m_fTime, fAppTime, m_fElapsedTime, fConstTime);

		__try
		{
			// UpdateViewPort();

			// Frame move the scene
			if( SUCCEEDED( hr = FrameMove() ) )
			{
			}

			// Render the scene as normal

			if(m_bActive && !IsPassiveRenderingEnabled())
			{
				if(  Is3DRenderingEnabled() && !m_bMinimized && SUCCEEDED(hr) && SUCCEEDED( hr = Render() ) )
				{
					// only present if render returns true.
					hr = PresentScene();
				}
			}
			else
			{
				// passive mode: sleep until the next ideal frame move time and a little more. 0.1 seconds
				Sleep(100);
			}
		}
		__except(GenerateDump(GetExceptionInformation()))
		{
			exit(0);
		}
		// Show the frame on the primary surface.
	}
	return S_OK;
}

HRESULT CD3DApplication::PresentScene()
{
	// OUTPUT_LOG("---------\n");
	HRESULT hr;
	// only present if render returns true.
	PERF1("present");
	hr = m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	if( D3DERR_DEVICELOST == hr )
		m_bDeviceLost = true;
	return hr;
}


bool CD3DApplication::UpdateViewPort()
{
	if(CGlobals::GetRenderDevice())
	{
		D3DVIEWPORT9 CurrentViewport;
		CGlobals::GetRenderDevice()->GetViewport(&CurrentViewport);
		if(m_pRenderWindow->GetWidth() != CurrentViewport.Width && m_pRenderWindow->GetHeight() != CurrentViewport.Height)
		{
			CurrentViewport.Width = m_pRenderWindow->GetWidth();
			CurrentViewport.Height = m_pRenderWindow->GetHeight();
			CGlobals::GetRenderDevice()->SetViewport(&CurrentViewport);
		}
		return true;
	}
	return false;
}

HRESULT CD3DApplication::Initialize3DEnvironment()
{
    HRESULT hr = S_OK;


    m_bWindowed = m_d3dSettings.IsWindowed;



	RenderDeviceConfiguration cfg;
	cfg.isWindowed = !m_bStartFullscreen;
	cfg.renderWindow = m_pRenderWindow;

	m_pRenderDevice = m_pRenderContext->CreateDevice(cfg);
	if (!m_pRenderDevice)
	{
		OUTPUT_LOG("Error: Can not create render device. \n");
		return E_FAIL;
	}

	m_pd3dDevice = static_cast<CD3D9RenderDevice*>(m_pRenderDevice)->GetDirect3DDevice9();


	m_pRenderDevice = new CD3D9RenderDevice(m_pd3dDevice);
	CGlobals::SetRenderDevice(m_pRenderDevice);


	// Initialize the app's device-dependent objects
	hr = InitDeviceObjects();



	if (FAILED(hr))
	{
		DeleteDeviceObjects();
	}
	else
	{
		m_bDeviceObjectsInited = true;
		hr = RestoreDeviceObjects();
		if (FAILED(hr))
		{
			InvalidateDeviceObjects();
		}
		else
		{
			m_bDeviceObjectsRestored = true;
			return S_OK;
		}
	}

    return hr;
}





HRESULT CD3DApplication::Reset3DEnvironment()
{
    HRESULT hr = E_FAIL;

    // Release all vidmem objects
    if( m_bDeviceObjectsRestored )
    {
        m_bDeviceObjectsRestored = false;
        InvalidateDeviceObjects();
    }

	RenderDeviceConfiguration cfg;
	cfg.renderWindow = m_pRenderWindow;
	cfg.isWindowed = true;

	if (!m_pRenderContext->ResetDevice(m_pRenderDevice, cfg))
	{
		OUTPUT_LOG("reset d3d device failed because Reset function failed: %d\n", hr);
		InterpretError(hr, __FILE__, __LINE__);
		return hr;
	}


    // Initialize the app's device-dependent objects
    hr = RestoreDeviceObjects();
    if( FAILED(hr) )
    {
		OUTPUT_LOG("reset d3d device failed because Restor func failed: %d\n", hr);
        InvalidateDeviceObjects();
        return hr;
    }
    m_bDeviceObjectsRestored = true;

    // If the app is paused, trigger the rendering of the current frame
    if( false == m_bFrameMoving )
    {
        m_bSingleStep = true;
        DXUtil_Timer( TIMER_START );
        DXUtil_Timer( TIMER_STOP );
    }

    return S_OK;
}



#pragma endregion DevicesAndEvents

#pragma region Miscs




//-----------------------------------------------------------------------------
// Name: UpdateStats()
// Desc: 
//-----------------------------------------------------------------------------
void CD3DApplication::UpdateStats()
{
    // Keep track of the frame count
    static double fLastTime = 0.0f;
    static DWORD dwFrames  = 0;
    double fTime = DXUtil_Timer( TIMER_GETABSOLUTETIME );
    ++dwFrames;

    // Update the scene stats once per second
    if( fTime - fLastTime > 1.0f )
    {
        m_fFPS    = (float)(dwFrames / (fTime - fLastTime));
        fLastTime = fTime;
        dwFrames  = 0;

        const int cchMaxFrameStats = sizeof(m_strFrameStats) / sizeof(TCHAR);
        _sntprintf( m_strFrameStats, cchMaxFrameStats, _T("%.02f fps (%dx%d)"), m_fFPS,
			m_pRenderWindow->GetWidth(), m_pRenderWindow->GetHeight());
        m_strFrameStats[cchMaxFrameStats - 1] = TEXT('\0');
    }
}

void CD3DApplication::SetRefreshTimer(float fTimeInterval, int nFrameRateControl)
{
	m_fRefreshTimerInterval = fTimeInterval;
	m_nFrameRateControl = nFrameRateControl;
	g_doWorkFRC.m_fConstDeltaTime = (m_fRefreshTimerInterval<=0.f) ? IDEAL_FRAME_RATE : m_fRefreshTimerInterval;
}

float CD3DApplication::GetRefreshTimer() const
{
	return m_fRefreshTimerInterval;
}

//-----------------------------------------------------------------------------
// Name: Pause()
// Desc: Called in to toggle the pause state of the app.
//-----------------------------------------------------------------------------
void CD3DApplication::Pause( bool bPause )
{
	// OUTPUT_LOG("game is %s\n", bPause ? "paused" : "resumed");
	m_bActive = !bPause;
}

 bool CD3DApplication::IsPaused()
 {
	 return !m_bActive;
 }


//-----------------------------------------------------------------------------
// Name: Cleanup3DEnvironment()
// Desc: Cleanup scene objects
//-----------------------------------------------------------------------------
void CD3DApplication::Cleanup3DEnvironment()
{
    if( m_pd3dDevice != NULL )
    {
        if( m_bDeviceObjectsRestored )
        {
            m_bDeviceObjectsRestored = false;
            InvalidateDeviceObjects();
        }
        if( m_bDeviceObjectsInited )
        {
            m_bDeviceObjectsInited = false;
            DeleteDeviceObjects();
        }

		if (m_pd3dSwapChain) {
			if (m_pd3dSwapChain->Release() > 0) {
				DisplayErrorMsg( D3DAPPERR_NONZEROREFCOUNT, MSGERR_APPMUSTEXIT );
			}
		}
		long nRefCount = 0;
		if ((nRefCount=m_pd3dDevice->Release()) > 0) {
			OUTPUT_LOG("\n\nerror: reference count of d3ddevice is non-zero %d when exit\n\n", nRefCount);
		}
		
        m_pd3dDevice = NULL;
    }
}

//-----------------------------------------------------------------------------
// Name: DisplayErrorMsg()
// Desc: Displays error messages in a message box
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::DisplayErrorMsg( HRESULT hr, DWORD dwType )
{
    return CD3DWindowUtil::DisplayErrorMsg(hr, dwType);
}


int CD3DApplication::Run(HINSTANCE hInstance)
{
	// CD3DWindowDefault can be used as a sample to create application using paraengine lib. 
	//CD3DWindowDefault defaultWin;
	//defaultWin.SetAppInterface(CGlobals::GetApp());
	
	//return defaultWin.Run(hInstance);


	int nWidth, nHeight;
	CGlobals::GetApp()->GetWindowCreationSize(&nWidth, &nHeight);

	WCHAR* WindowClassName = L"ParaWorld";
	WCHAR* WindowTitle = L"ParaEngine Window";
	m_pRenderWindow = new WindowsRenderWindow(hInstance, nWidth, nHeight, false);
	m_hWnd = m_pRenderWindow->GetHandle();
	auto msgCallback = std::bind(&CD3DApplication::HandleWindowMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	m_pRenderWindow->SetMessageCallBack(msgCallback);
	CGlobals::GetApp()->SetMainWindow(m_hWnd, false);
	CGlobals::GetApp()->Create();

	while (!m_pRenderWindow->ShouldClose())
	{
		m_pRenderWindow->PollEvents();
		DoWork();
	}

	return 0;
}

HRESULT CD3DApplication::DoWork()
{
	// continue with next activation. 
	double fCurTime = DXUtil_Timer( TIMER_GETAPPTIME );
	if(g_doWorkFRC.FrameMove(fCurTime) > 0)
	{
		return Render3DEnvironment();
	}
	return S_OK;
}


LRESULT CD3DApplication::HandleWindowMessage(WindowsRenderWindow* sender, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hWnd = sender->GetHandle();
	if (uMsg == WM_DESTROY)
	{
		CGlobals::GetApp()->PostWinThreadMessage(PE_WM_QUIT, 0, 0);
	}
	return CGlobals::GetApp()->MsgProcWinThread(hWnd, uMsg, wParam, lParam,true);
}

#pragma endregion Miscs