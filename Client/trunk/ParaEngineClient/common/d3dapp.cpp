//-----------------------------------------------------------------------------
// Class:	CD3DApplication
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Desc: Create the d3d device and manage its relationship with a window HWND. 
/** 
Fix0.1: Currently, framemove and render are in the same thread. 
The default mode is that the framemove and render function will be called 30 times a seconds
It is thus assumed that the player's CPU and GPU is able to handle that rate. However, if the task has been
completed less than 33ms, the remaining time is wasted. It is the programmer's task to gaurantee that 
the application always terminate after 33ms, which include the physics, IO and rendering routines. 
Alternatively, one can turn on NO_FRAME_RATE_CONTROL flag, to remove this inforced 30FPS constraint. 
The game is allowed to run as fast as possible(over several hundreds FPS). However, some tearing and jittering
effect will appear with some screen resolution and refreshrate. This is due to the Present() method and
VSYNC parameters. It is up to the user to properly configure their hardware device.

The default behavior of Present method prior to Direct 9 was to wait for GPU to become available 
for performing presentation operation, thus causing a stall in runtime or driver. This behavior 
changed in DirectX 9 by including a special presentation flag in swap chain Present method. 
Instead of using device��s Present method (which is really a shortcut to swap chain��s presentation 
function) retrieve a swap chain and use it for presentation with D3DPRESENT_DONOTWAIT flag. 
If CPU is currently unavailable for performing presentation, Present will return D3DERR_WASSTILLDRAWING error code. 
Using this presentation method can give scarce CPU cycles back to the application and improve CPU and graphics hardware parallelism.
*/
#include "ParaEngine.h"

#pragma region Headers

#include "resource.h"
#include "2dengine/GUIRoot.h"
#include "2dengine/GUIDirectInput.h"
#include "ParaEngineApp.h"
#include "D3DWindowDefault.h"
#include "D3DWindowUtil.h"
#include "FrameRateController.h"
#include "MiscEntity.h"
#include "d3dapp.h"

using namespace ParaEngine;

static CD3DApplication* g_pD3DApp = NULL;
CFrameRateController g_doWorkFRC(CFrameRateController::FRC_CONSTANT_OR_BELOW);

CD3DApplication::CD3DApplication()
{
    g_pD3DApp           = this;

	SetAppState(PEAppState_None);

	m_bIsExternalD3DDevice = false;
    m_pD3D              = NULL;
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

	m_bDisableD3D = false;
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
	m_nClientWidth = 0;
	m_nClientHeight = 0;
    m_bShowCursorWhenFullscreen = true;
    m_bStartFullscreen  = false;
    m_bCreateMultithreadDevice = true;
    m_bAllowDialogBoxMode = false;
	m_bIsExternalWindow = false;

	memset(&m_d3dSettings, 0 , sizeof(m_d3dSettings));
	memset( &m_rcWindowBounds, 0, sizeof(RECT));
	memset( &m_rcWindowClient, 0, sizeof(RECT));
	
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
// Name: ConfirmDeviceHelper()
// Desc: Static function used by D3DEnumeration
//-----------------------------------------------------------------------------
bool CD3DApplication::ConfirmDeviceHelper( D3DCAPS9* pCaps, VertexProcessingType vertexProcessingType, 
                         D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat )
{
	DWORD dwBehavior;

    if (vertexProcessingType == SOFTWARE_VP)
        dwBehavior = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    else if (vertexProcessingType == MIXED_VP)
        dwBehavior = D3DCREATE_MIXED_VERTEXPROCESSING;
    else if (vertexProcessingType == HARDWARE_VP)
        dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING;
    else if (vertexProcessingType == PURE_HARDWARE_VP)
        dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
    else
        dwBehavior = 0; // TODO: throw exception
    
    return SUCCEEDED( g_pD3DApp->ConfirmDevice( pCaps, dwBehavior, adapterFormat, backBufferFormat ) );
}

HRESULT CD3DApplication::CreateFromD3D9Device(IDirect3DDevice9* pD3dDevice, IDirect3DSwapChain9* apSwapChain)
{
	HRESULT hr; 
	m_bIsExternalD3DDevice = true;
	m_pD3D = NULL;
	m_pd3dDevice = pD3dDevice;
	m_pd3dDevice->AddRef();
	m_pd3dSwapChain = apSwapChain;
	m_pd3dSwapChain->AddRef();
	SetAppState(PEAppState_Device_Created);

	OUTPUT_LOG("Note: d3d device is created by external application\n");

	// The focus window can be a specified to be a different window than the
	// device window.  If not, use the device window as the focus window.
	if( m_hWndFocus == NULL )
		m_hWndFocus = m_hWnd;

	OUTPUT_LOG("DEBUG: main thread wnd handle : %d\n", m_hWndFocus);

	// Save window properties
	m_dwWindowStyle = GetWindowLong( m_hWnd, GWL_STYLE );
	HandlePossibleSizeChange(true);

	// Initialize the application timer
	DXUtil_Timer( TIMER_START );

	// Initialize the app's custom scene stuff
	if( FAILED( hr = OneTimeSceneInit() ) )
	{
		return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
	}

	if(!m_bDisableD3D)
	{
		// Initialize the 3D environment for the app
		if( FAILED( hr = Initialize3DEnvironment() ) )
		{
			return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
		}
	}

	// The app is ready to go
	Pause( false );
	SetAppState(PEAppState_Ready);

	return S_OK;
}
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

	m_bIsExternalD3DDevice = false;
	SetAppState(PEAppState_Device_Created);
	
	if(!m_bDisableD3D)
	{
		// Create the Direct3D object
		m_pD3D = Direct3DCreate9( D3D_SDK_VERSION );
		if( m_pD3D == NULL )
			return DisplayErrorMsg( D3DAPPERR_NODIRECT3D, MSGERR_APPMUSTEXIT );

		// Build a list of Direct3D adapters, modes and devices. The
		// ConfirmDevice() callback is used to confirm that only devices that
		// meet the app's requirements are considered.
		m_d3dEnumeration.SetD3D( m_pD3D );
		m_d3dEnumeration.ConfirmDeviceCallback = ConfirmDeviceHelper;
		if( FAILED( hr = m_d3dEnumeration.Enumerate() ) )
		{
			SAFE_RELEASE( m_pD3D );
			return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
		}
	}
	
    // The focus window can be a specified to be a different window than the
    // device window.  If not, use the device window as the focus window.
    if( m_hWndFocus == NULL )
        m_hWndFocus = m_hWnd;

	OUTPUT_LOG("DEBUG: main thread wnd handle : %d\n", m_hWndFocus);


    // Save window properties
    m_dwWindowStyle = GetWindowLong( m_hWnd, GWL_STYLE );
    HandlePossibleSizeChange(true);

	if(!m_bDisableD3D)
	{
		if( FAILED( hr = ChooseInitialD3DSettings() ) )
		{
			SAFE_RELEASE( m_pD3D );
			return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
		}
	}

    // Initialize the application timer
    DXUtil_Timer( TIMER_START );

    // Initialize the app's custom scene stuff
    if( FAILED( hr = OneTimeSceneInit() ) )
    {
        SAFE_RELEASE( m_pD3D );
        return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
    }

	if(!m_bDisableD3D)
	{
		// Initialize the 3D environment for the app
		if( FAILED( hr = Initialize3DEnvironment() ) )
		{
			SAFE_RELEASE( m_pD3D );
			return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
		}
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

	if(!m_bDisableD3D &&  m_bDeviceLost )
	{
		// Test the cooperative level to see if it's okay to render
		if(!m_pd3dDevice || FAILED( hr = m_pd3dDevice->TestCooperativeLevel() ) )
		{
			static int s_deviceLostCount = 0;
			// If the device was lost, do not render until we get it back
			if (!m_pd3dDevice || D3DERR_DEVICELOST == hr)
			{
				OUTPUT_LOG("TestCooperativeLevel D3DERR_DEVICELOST\n");
				// TRICKY: when connecting with win10 remote desktop, TestCooperativeLevel always returns D3DERR_DEVICELOST
				// we will recreate the entire directX device in this case. 
				s_deviceLostCount = s_deviceLostCount + 1;
				if (s_deviceLostCount >= 5)
				{
					s_deviceLostCount = 0;
					OUTPUT_LOG("keep getting D3DERR_DEVICELOST. Try recreate entire d3d interface instead of resetting it. This could be a bug of direct3d. \n");
					Cleanup3DEnvironment();
					SAFE_RELEASE(m_pD3D);
					m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
					m_d3dEnumeration.SetD3D(m_pD3D);
					if (SUCCEEDED(hr = m_d3dEnumeration.Enumerate()))
					{
						if (SUCCEEDED(hr = ChooseInitialD3DSettings()))
						{
							hr = Initialize3DEnvironment();
							if (SUCCEEDED(hr))
								return S_OK;
						}
						else
						{
							SAFE_RELEASE(m_pD3D);
						}
					}
					else
					{
						SAFE_RELEASE(m_pD3D);
					}
				}

				::Sleep(1000);
				return S_OK;
			}

			// Check if the device needs to be reset.
			if( D3DERR_DEVICENOTRESET == hr )
			{
				// If we are windowed, read the desktop mode and use the same format for
				// the back buffer
				if( IsExternalD3DDevice() && m_bWindowed )
				{
					D3DAdapterInfo* pAdapterInfo = m_d3dSettings.PAdapterInfo();
					m_pD3D->GetAdapterDisplayMode( pAdapterInfo->AdapterOrdinal, &m_d3dSettings.Windowed_DisplayMode );
					m_d3dpp.BackBufferFormat = m_d3dSettings.Windowed_DisplayMode.Format;
				}
				
				OUTPUT_LOG("TestCooperativeLevel needs to reset device with D3DERR_DEVICENOTRESET\n");
				
				if( FAILED( hr = Reset3DEnvironment() ) )
				{
					// This fixed a strange issue where d3d->reset() returns D3DERR_DEVICELOST. 
					// I will keep reset for 5 seconds, until reset() succeed
					for (int i=0; i < 5 && hr == D3DERR_DEVICELOST; ++i)
					{
						::Sleep(1000);
						if( SUCCEEDED( hr = Reset3DEnvironment() ) )
						{
							OUTPUT_LOG("TestCooperativeLevel successfully reset devices.\n");

							if(!m_bWindowed)
							{
								// if user toggles devices during full screen mode, we will switch to windowed mode. 
								ToggleFullscreen();
							}
							return hr;
						}
					}
				}
				s_deviceLostCount = 0;
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

			if(m_bActive && !IsPassiveRenderingEnabled() && (!m_bDisableD3D))
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
	if (IsExternalD3DDevice()) {
		hr = m_pd3dSwapChain->Present( NULL, NULL, m_hWnd, NULL, 0 );
	}
	else {
		hr = m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	}
	if( D3DERR_DEVICELOST == hr )
		m_bDeviceLost = true;
	return hr;
}

//-----------------------------------------------------------------------------
// Name: FindBestWindowedMode()
// Desc: Sets up m_d3dSettings with best available windowed mode, subject to 
//       the bRequireHAL and bRequireREF constraints.  Returns false if no such
//       mode can be found.
//-----------------------------------------------------------------------------
bool CD3DApplication::FindBestWindowedMode( bool bRequireHAL, bool bRequireREF )
{
	if(IsExternalD3DDevice())
		return true;
    // Get display mode of primary adapter (which is assumed to be where the window 
    // will appear)
    D3DDISPLAYMODE primaryDesktopDisplayMode;
    m_pD3D->GetAdapterDisplayMode(0, &primaryDesktopDisplayMode);

    D3DAdapterInfo* pBestAdapterInfo = NULL;
    D3DDeviceInfo* pBestDeviceInfo = NULL;
    D3DDeviceCombo* pBestDeviceCombo = NULL;

    for( UINT iai = 0; iai < m_d3dEnumeration.m_pAdapterInfoList->Count(); iai++ )
    {
        D3DAdapterInfo* pAdapterInfo = (D3DAdapterInfo*)m_d3dEnumeration.m_pAdapterInfoList->GetPtr(iai);
        for( UINT idi = 0; idi < pAdapterInfo->pDeviceInfoList->Count(); idi++ )
        {
            D3DDeviceInfo* pDeviceInfo = (D3DDeviceInfo*)pAdapterInfo->pDeviceInfoList->GetPtr(idi);
            if (bRequireHAL && pDeviceInfo->DevType != D3DDEVTYPE_HAL)
                continue;
            if (bRequireREF && pDeviceInfo->DevType != D3DDEVTYPE_REF)
                continue;
            for( UINT idc = 0; idc < pDeviceInfo->pDeviceComboList->Count(); idc++ )
            {
                D3DDeviceCombo* pDeviceCombo = (D3DDeviceCombo*)pDeviceInfo->pDeviceComboList->GetPtr(idc);
                bool bAdapterMatchesBB = (pDeviceCombo->BackBufferFormat == pDeviceCombo->AdapterFormat);
                if (!pDeviceCombo->IsWindowed)
                    continue;
                if (pDeviceCombo->AdapterFormat != primaryDesktopDisplayMode.Format)
                    continue;
                // If we haven't found a compatible DeviceCombo yet, or if this set
                // is better (because it's a HAL, and/or because formats match better),
                // save it
                if( pBestDeviceCombo == NULL || 
                    pBestDeviceCombo->DevType != D3DDEVTYPE_HAL && pDeviceCombo->DevType == D3DDEVTYPE_HAL ||
                    pDeviceCombo->DevType == D3DDEVTYPE_HAL && (bAdapterMatchesBB|| pDeviceCombo->BackBufferFormat==D3DFMT_A8R8G8B8 ) )
                {
                    pBestAdapterInfo = pAdapterInfo;
                    pBestDeviceInfo = pDeviceInfo;
                    pBestDeviceCombo = pDeviceCombo;
                    if( pDeviceCombo->DevType == D3DDEVTYPE_HAL && (pDeviceCombo->BackBufferFormat==D3DFMT_A8R8G8B8) )
                    {
                        // This windowed device combo looks great -- take it
                        goto EndWindowedDeviceComboSearch;
                    }
                    // Otherwise keep looking for a better windowed device combo
                }
            }
        }
    }
EndWindowedDeviceComboSearch:
    if (pBestDeviceCombo == NULL )
        return false;

    m_d3dSettings.pWindowed_AdapterInfo = pBestAdapterInfo;
    m_d3dSettings.pWindowed_DeviceInfo = pBestDeviceInfo;
    m_d3dSettings.pWindowed_DeviceCombo = pBestDeviceCombo;
    m_d3dSettings.IsWindowed = true;
    m_d3dSettings.Windowed_DisplayMode = primaryDesktopDisplayMode;
    m_d3dSettings.Windowed_Width = m_rcWindowClient.right - m_rcWindowClient.left;
    m_d3dSettings.Windowed_Height = m_rcWindowClient.bottom - m_rcWindowClient.top;
    if (m_d3dEnumeration.AppUsesDepthBuffer)
        m_d3dSettings.Windowed_DepthStencilBufferFormat = *(D3DFORMAT*)pBestDeviceCombo->pDepthStencilFormatList->GetPtr(0);

	{
		// by LXZ: now we try to find m_d3dSettings.Fullscreen_MultisampleType, m_d3dSettings.Fullscreen_MultisampleQuality
		bool bFound = false;
		for( UINT ims = 0; ims < pBestDeviceCombo->pMultiSampleTypeList->Count(); ims++ )
		{
			D3DMULTISAMPLE_TYPE msType = *(D3DMULTISAMPLE_TYPE*)pBestDeviceCombo->pMultiSampleTypeList->GetPtr(ims);
			if(m_d3dSettings.Windowed_MultisampleType == msType)
			{
				bFound = true;
				DWORD maxQuality = *(DWORD*)pBestDeviceCombo->pMultiSampleQualityList->GetPtr(ims);
				if(maxQuality<m_d3dSettings.Windowed_MultisampleQuality)
				{
					OUTPUT_LOG("warning: MultiSampleQuality has maximum value for your device is %d. However you are setting it to %d. We will disable AA.\r\n", maxQuality, m_d3dSettings.Windowed_MultisampleQuality);
					m_d3dSettings.Windowed_MultisampleType = D3DMULTISAMPLE_NONE;
					m_d3dSettings.Windowed_MultisampleQuality = 0;
				}
				break;
			}
		}
		if(!bFound){
			OUTPUT_LOG("warning: MultiSampleType %d is not supported for your GPU\r\n", m_d3dSettings.Windowed_MultisampleType);
			m_d3dSettings.Windowed_MultisampleType = D3DMULTISAMPLE_NONE; // *(D3DMULTISAMPLE_TYPE*)pBestDeviceCombo->pMultiSampleTypeList->GetPtr(0)
			m_d3dSettings.Windowed_MultisampleQuality = 0;
		}
	}

    m_d3dSettings.Windowed_VertexProcessingType = *(VertexProcessingType*)pBestDeviceCombo->pVertexProcessingTypeList->GetPtr(0);
    m_d3dSettings.Windowed_PresentInterval = *(UINT*)pBestDeviceCombo->pPresentIntervalList->GetPtr(0);
    return true;
}




//-----------------------------------------------------------------------------
// Name: FindBestFullscreenMode()
// Desc: Sets up m_d3dSettings with best available fullscreen mode, subject to 
//       the bRequireHAL and bRequireREF constraints.  Returns false if no such
//       mode can be found.
//-----------------------------------------------------------------------------
bool CD3DApplication::FindBestFullscreenMode( bool bRequireHAL, bool bRequireREF )
{
	if(IsExternalD3DDevice())
		return true;
    // For fullscreen, default to first HAL DeviceCombo that supports the current desktop 
    // display mode, or any display mode if HAL is not compatible with the desktop mode, or 
    // non-HAL if no HAL is available
    D3DDISPLAYMODE adapterDesktopDisplayMode;
    D3DDISPLAYMODE bestAdapterDesktopDisplayMode;
    D3DDISPLAYMODE bestDisplayMode;
    bestAdapterDesktopDisplayMode.Width = 0;
    bestAdapterDesktopDisplayMode.Height = 0;
    bestAdapterDesktopDisplayMode.Format = D3DFMT_UNKNOWN;
    bestAdapterDesktopDisplayMode.RefreshRate = 0;

    D3DAdapterInfo* pBestAdapterInfo = NULL;
    D3DDeviceInfo* pBestDeviceInfo = NULL;
    D3DDeviceCombo* pBestDeviceCombo = NULL;

    for( UINT iai = 0; iai < m_d3dEnumeration.m_pAdapterInfoList->Count(); iai++ )
    {
        D3DAdapterInfo* pAdapterInfo = (D3DAdapterInfo*)m_d3dEnumeration.m_pAdapterInfoList->GetPtr(iai);
        m_pD3D->GetAdapterDisplayMode( pAdapterInfo->AdapterOrdinal, &adapterDesktopDisplayMode );
        for( UINT idi = 0; idi < pAdapterInfo->pDeviceInfoList->Count(); idi++ )
        {
            D3DDeviceInfo* pDeviceInfo = (D3DDeviceInfo*)pAdapterInfo->pDeviceInfoList->GetPtr(idi);
            if (bRequireHAL && pDeviceInfo->DevType != D3DDEVTYPE_HAL)
                continue;
            if (bRequireREF && pDeviceInfo->DevType != D3DDEVTYPE_REF)
                continue;
            for( UINT idc = 0; idc < pDeviceInfo->pDeviceComboList->Count(); idc++ )
            {
                D3DDeviceCombo* pDeviceCombo = (D3DDeviceCombo*)pDeviceInfo->pDeviceComboList->GetPtr(idc);
                bool bAdapterMatchesBB = (pDeviceCombo->BackBufferFormat == pDeviceCombo->AdapterFormat);
                bool bAdapterMatchesDesktop = (pDeviceCombo->AdapterFormat == adapterDesktopDisplayMode.Format);
                if (pDeviceCombo->IsWindowed)
                    continue;
                // If we haven't found a compatible set yet, or if this set
                // is better (because it's a HAL, and/or because formats match better),
                // save it
                if (pBestDeviceCombo == NULL ||
                    pBestDeviceCombo->DevType != D3DDEVTYPE_HAL && pDeviceInfo->DevType == D3DDEVTYPE_HAL ||
                    pDeviceCombo->DevType == D3DDEVTYPE_HAL && pBestDeviceCombo->AdapterFormat != adapterDesktopDisplayMode.Format && bAdapterMatchesDesktop ||
                    pDeviceCombo->DevType == D3DDEVTYPE_HAL && bAdapterMatchesDesktop && (bAdapterMatchesBB|| pDeviceCombo->BackBufferFormat==D3DFMT_A8R8G8B8 ) )
                {
                    bestAdapterDesktopDisplayMode = adapterDesktopDisplayMode;
                    pBestAdapterInfo = pAdapterInfo;
                    pBestDeviceInfo = pDeviceInfo;
                    pBestDeviceCombo = pDeviceCombo;
					if (pDeviceInfo->DevType == D3DDEVTYPE_HAL && bAdapterMatchesDesktop && (pDeviceCombo->BackBufferFormat==D3DFMT_A8R8G8B8 ))
                    {
                        // This fullscreen device combo looks great -- take it
                        goto EndFullscreenDeviceComboSearch;
                    }
                    // Otherwise keep looking for a better fullscreen device combo
                }
            }
        }
    }
EndFullscreenDeviceComboSearch:
    if (pBestDeviceCombo == NULL)
        return false;

    // Need to find a display mode on the best adapter that uses pBestDeviceCombo->AdapterFormat
    // and is as close to bestAdapterDesktopDisplayMode's res as possible
    bestDisplayMode.Width = 0;
    bestDisplayMode.Height = 0;
    bestDisplayMode.Format = D3DFMT_UNKNOWN;
    bestDisplayMode.RefreshRate = 0;
    for( UINT idm = 0; idm < pBestAdapterInfo->pDisplayModeList->Count(); idm++ )
    {
        D3DDISPLAYMODE* pdm = (D3DDISPLAYMODE*)pBestAdapterInfo->pDisplayModeList->GetPtr(idm);
        if( pdm->Format != pBestDeviceCombo->AdapterFormat )
            continue;
        if( pdm->Width == bestAdapterDesktopDisplayMode.Width &&
            pdm->Height == bestAdapterDesktopDisplayMode.Height && 
            pdm->RefreshRate == bestAdapterDesktopDisplayMode.RefreshRate )
        {
            // found a perfect match, so stop
            bestDisplayMode = *pdm;
            break;
        }
        else if( pdm->Width == bestAdapterDesktopDisplayMode.Width &&
                 pdm->Height == bestAdapterDesktopDisplayMode.Height && 
                 pdm->RefreshRate > bestDisplayMode.RefreshRate )
        {
            // refresh rate doesn't match, but width/height match, so keep this
            // and keep looking
            bestDisplayMode = *pdm;
        }
        else if( pdm->Width == bestAdapterDesktopDisplayMode.Width )
        {
            // width matches, so keep this and keep looking
            bestDisplayMode = *pdm;
        }
        else if( bestDisplayMode.Width == 0 )
        {
            // we don't have anything better yet, so keep this and keep looking
            bestDisplayMode = *pdm;
        }
    }


	{
		// By LiXizhi 2008.7.5: Find the closest match to the (m_dwCreationWidth, m_dwCreationHeight)
		// it is either a perfect match or a match that is slightly bigger than (m_dwCreationWidth, m_dwCreationHeight)
		// by LXZ: now we try to find (m_dwCreationWidth, m_dwCreationHeight) if any
		for( UINT idm = 0; idm < pBestAdapterInfo->pDisplayModeList->Count(); idm++ )
		{
			D3DDISPLAYMODE* pdm = (D3DDISPLAYMODE*)pBestAdapterInfo->pDisplayModeList->GetPtr(idm);
			if( pdm->Format != pBestDeviceCombo->AdapterFormat )
				continue;
			if( pdm->Width == m_dwCreationWidth &&
				pdm->Height == m_dwCreationHeight && 
				pdm->RefreshRate == bestAdapterDesktopDisplayMode.RefreshRate )
			{
				// found a perfect match, so stop
				bestDisplayMode = *pdm;
				break;
			}
			else if( pdm->Width >= m_dwCreationWidth && pdm->Height >= m_dwCreationHeight 
					&& (pdm->Width <= bestAdapterDesktopDisplayMode.Width || bestAdapterDesktopDisplayMode.Width <m_dwCreationWidth)
					&& (pdm->Height <= bestAdapterDesktopDisplayMode.Height || bestAdapterDesktopDisplayMode.Height <m_dwCreationHeight))
			{
				// OUTPUT_LOG("candidate window (%d, %d)\n", pdm->Width, pdm->Height);
				// width/height is bigger, so keep this and keep looking
				if(bestDisplayMode.Width < m_dwCreationWidth || bestDisplayMode.Height < m_dwCreationHeight )
				{
					// if the best display mode is not big enough, select a bigger one. 
					bestDisplayMode = *pdm;
				}
				else if(pdm->Width<bestDisplayMode.Width || pdm->Height<bestDisplayMode.Height)
				{
					// if the new one size is more close to the creation size, keep it.
					if(((bestDisplayMode.Width-pdm->Width)*m_dwCreationHeight + (bestDisplayMode.Height-pdm->Height)*m_dwCreationWidth)>0)
					{
						bestDisplayMode = *pdm;
					}
				}
				else if((pdm->Width==bestDisplayMode.Width && pdm->Height==bestDisplayMode.Height) && pdm->RefreshRate == bestAdapterDesktopDisplayMode.RefreshRate)
				{
					// if the new one size is same as the current best, but with a best refresh rate, keep it. 
					bestDisplayMode = *pdm;
				}
			}
		}
	}
	
    m_d3dSettings.pFullscreen_AdapterInfo = pBestAdapterInfo;
    m_d3dSettings.pFullscreen_DeviceInfo = pBestDeviceInfo;
    m_d3dSettings.pFullscreen_DeviceCombo = pBestDeviceCombo;
    m_d3dSettings.IsWindowed = false;
    m_d3dSettings.Fullscreen_DisplayMode = bestDisplayMode;
    if (m_d3dEnumeration.AppUsesDepthBuffer)
        m_d3dSettings.Fullscreen_DepthStencilBufferFormat = *(D3DFORMAT*)pBestDeviceCombo->pDepthStencilFormatList->GetPtr(0);

	{
		// by LXZ: now we try to find m_d3dSettings.Fullscreen_MultisampleType, m_d3dSettings.Fullscreen_MultisampleQuality
		bool bFound = false;
		for( UINT ims = 0; ims < pBestDeviceCombo->pMultiSampleTypeList->Count(); ims++ )
		{
			D3DMULTISAMPLE_TYPE msType = *(D3DMULTISAMPLE_TYPE*)pBestDeviceCombo->pMultiSampleTypeList->GetPtr(ims);
			if(m_d3dSettings.Fullscreen_MultisampleType == msType)
			{
				bFound = true;
				DWORD maxQuality = *(DWORD*)pBestDeviceCombo->pMultiSampleQualityList->GetPtr(ims);
				if(maxQuality<m_d3dSettings.Fullscreen_MultisampleQuality)
				{
					OUTPUT_LOG("warning: MultiSampleQuality has maximum value for your device is %d. However you are setting it to %d. We will disable AA.\r\n", maxQuality, m_d3dSettings.Fullscreen_MultisampleQuality);
					m_d3dSettings.Fullscreen_MultisampleType = D3DMULTISAMPLE_NONE;
					m_d3dSettings.Fullscreen_MultisampleQuality = 0;
				}
				break;
			}
		}
		if(!bFound){
			OUTPUT_LOG("warning: MultiSampleType %d is not supported for your GPU\r\n", m_d3dSettings.Fullscreen_MultisampleType);
			m_d3dSettings.Fullscreen_MultisampleType = D3DMULTISAMPLE_NONE; // *(D3DMULTISAMPLE_TYPE*)pBestDeviceCombo->pMultiSampleTypeList->GetPtr(0);
			m_d3dSettings.Fullscreen_MultisampleQuality = 0;
		}
	}
	
    m_d3dSettings.Fullscreen_VertexProcessingType = *(VertexProcessingType*)pBestDeviceCombo->pVertexProcessingTypeList->GetPtr(0);
    m_d3dSettings.Fullscreen_PresentInterval = D3DPRESENT_INTERVAL_DEFAULT;
	
	OUTPUT_LOG("Best full screen mode is (%d, %d)\n", m_d3dSettings.Fullscreen_DisplayMode.Width, m_d3dSettings.Fullscreen_DisplayMode.Height);
    return true;
}




//-----------------------------------------------------------------------------
// Name: ChooseInitialD3DSettings()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::ChooseInitialD3DSettings()
{
	if(IsExternalD3DDevice())
		return S_OK;
    bool bFoundFullscreen = FindBestFullscreenMode( false, false );
    bool bFoundWindowed = FindBestWindowedMode( false, false );
    m_d3dSettings.SetDeviceClip( false );

	// For external window, always start in windowed mode, this fixed a bug for web browser plugin. 
	if(!m_bIsExternalWindow)
	{
		if( m_bStartFullscreen && bFoundFullscreen)
			m_d3dSettings.IsWindowed = false;
		if( !bFoundWindowed && bFoundFullscreen )
			m_d3dSettings.IsWindowed = false;
	}

    if( !bFoundFullscreen && !bFoundWindowed )
        return D3DAPPERR_NOCOMPATIBLEDEVICES;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: HandlePossibleSizeChange()
// Desc: Reset the device if the client area size has changed.
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::HandlePossibleSizeChange(bool bUpdateSizeOnly)
{
	HRESULT hr = S_OK;
    RECT rcClientOld;
    rcClientOld = m_rcWindowClient;

    // Update window properties
    GetWindowRect( m_hWnd, &m_rcWindowBounds );
    GetClientRect( m_hWnd, &m_rcWindowClient );
	m_nClientWidth = m_rcWindowClient.right - m_rcWindowClient.left;
	m_nClientHeight = m_rcWindowClient.bottom - m_rcWindowClient.top;

	//OUTPUT_LOG("window rect width:%d height:%d\n", m_rcWindowBounds.right - m_rcWindowBounds.left, m_rcWindowBounds.bottom - m_rcWindowBounds.top);
	//OUTPUT_LOG("client rect width:%d height:%d\n", m_rcWindowClient.right - m_rcWindowClient.left, m_rcWindowClient.bottom - m_rcWindowClient.top);

    if( (rcClientOld.right - rcClientOld.left) != (m_rcWindowClient.right - m_rcWindowClient.left) ||
        (rcClientOld.bottom - rcClientOld.top) != (m_rcWindowClient.bottom - m_rcWindowClient.top) )
    {
		OUTPUT_LOG("update d3d window size: width: %d, height:%d, left:%d, top:%d\n", m_nClientWidth, m_nClientHeight, m_rcWindowBounds.left, m_rcWindowBounds.top);
		if( !bUpdateSizeOnly && !m_bIgnoreSizeChange )
		{
			// A new window size will require a new backbuffer
			// size, so the 3D structures must be changed accordingly.
			Pause( true );

			m_d3dpp.BackBufferWidth  = std::max(1, (int)(m_rcWindowClient.right - m_rcWindowClient.left));
			m_d3dpp.BackBufferHeight = std::max(1, (int)(m_rcWindowClient.bottom - m_rcWindowClient.top));
            
			m_d3dSettings.Windowed_Width = m_d3dpp.BackBufferWidth;
			m_d3dSettings.Windowed_Height = m_d3dpp.BackBufferHeight;

			if(IsExternalD3DDevice())
			{
				// With swapchain, there is no need to do this. 
				//// Release all vidmem objects
				//if( m_bDeviceObjectsRestored )
				//{
				//	m_bDeviceObjectsRestored = false;
				//	InvalidateDeviceObjects();
				//}
				//// Initialize the app's device-dependent objects
				//hr = RestoreDeviceObjects();
				//m_bDeviceObjectsRestored = true;
			}
			else if( m_pd3dDevice != NULL )
			{
				// Reset the 3D environment
				if( FAILED( hr = Reset3DEnvironment() ) )
				{
					if( hr == D3DERR_DEVICELOST )
					{
						m_bDeviceLost = true;
						hr = S_OK;
					}
					else
					{
						if( hr != D3DERR_OUTOFVIDEOMEMORY )
							hr = D3DAPPERR_RESETFAILED;

						DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
					}
				}
			}
			Pause( false );

		}

		if(m_bWindowed && !m_bIgnoreSizeChange && CGlobals::GetRenderDevice())
		{
			UpdateViewPort();
		}
    }
    return hr;
}

bool CD3DApplication::UpdateViewPort()
{
	if(CGlobals::GetRenderDevice())
	{
		D3DVIEWPORT9 CurrentViewport;
		CGlobals::GetRenderDevice()->GetViewport(&CurrentViewport);
		if(m_d3dpp.BackBufferWidth != CurrentViewport.Width && m_d3dpp.BackBufferHeight != CurrentViewport.Height)
		{
			CurrentViewport.Width = m_d3dpp.BackBufferWidth;
			CurrentViewport.Height = m_d3dpp.BackBufferHeight;
			CGlobals::GetRenderDevice()->SetViewport(&CurrentViewport);
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Name: Initialize3DEnvironment()
// Desc: Usually this function is not overridden.  Here's what this function does:
//       - Sets the windowed flag to be either windowed or fullscreen
//       - Sets parameters for z-buffer depth and back buffer
//       - Creates the D3D device
//       - Sets the window position (if windowed, that is)
//       - Makes some determinations as to the abilites of the driver (HAL, etc)
//       - Sets up some cursor stuff
//       - Calls InitDeviceObjects()
//       - Calls RestoreDeviceObjects()
//       - If all goes well, m_bActive is set to TRUE, and the function returns
//       - Otherwise, initialization is reattempted using the reference device
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::Initialize3DEnvironment()
{
    HRESULT hr = S_OK;

	if(IsExternalD3DDevice()) {
		/*
		This actually gets us only one 'main' swap chain per adapter...
		so it doesn't include the swap chains created with CreateAdditionalSwapChain()....

		DWORD numSwapChains = m_pd3dDevice->GetNumberOfSwapChains();
		if (numSwapChains == 0) {
			return E_FAIL;
		}

		IDirect3DSwapChain9* pLastSwapChain = NULL;
		if (FAILED(m_pd3dDevice->GetSwapChain(numSwapChains-1,&pLastSwapChain))) {
			return E_FAIL;
		}*/
		
		// Store render target surface desc
		LPDIRECT3DSURFACE9 pBackBuffer = NULL;
		m_pd3dSwapChain->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
		pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
		

		m_pd3dSwapChain->GetPresentParameters(&m_d3dpp);
		OUTPUT_LOG("External d3d device info: Backbuffer size:%dx%d, MultiSampleType:%d\n", m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight, m_d3dpp.MultiSampleType);
	
		InitDeviceObjects();
		m_bDeviceObjectsInited = true;
		RestoreDeviceObjects();
		m_bDeviceObjectsRestored = true;
		pBackBuffer->Release();
		return hr;
	}

    D3DAdapterInfo* pAdapterInfo = m_d3dSettings.PAdapterInfo();
    D3DDeviceInfo* pDeviceInfo = m_d3dSettings.PDeviceInfo();

    m_bWindowed = m_d3dSettings.IsWindowed;

    // Prepare window for possible windowed/fullscreen change
    AdjustWindowForChange();

    // Set up the presentation parameters
    BuildPresentParamsFromSettings();

    if( pDeviceInfo->Caps.PrimitiveMiscCaps & D3DPMISCCAPS_NULLREFERENCE )
    {
        // Warn user about null ref device that can't render anything
        DisplayErrorMsg( D3DAPPERR_NULLREFDEVICE, 0 );
    }

    DWORD behaviorFlags;
    if (m_d3dSettings.GetVertexProcessingType() == SOFTWARE_VP)
        behaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    else if (m_d3dSettings.GetVertexProcessingType() == MIXED_VP)
        behaviorFlags = D3DCREATE_MIXED_VERTEXPROCESSING;
    else if (m_d3dSettings.GetVertexProcessingType() == HARDWARE_VP)
        behaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
    else if (m_d3dSettings.GetVertexProcessingType() == PURE_HARDWARE_VP)
        behaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
    else
        behaviorFlags = 0; // TODO: throw exception

    // Add multithreaded flag if requested by app
    if( m_bCreateMultithreadDevice )
        behaviorFlags |= D3DCREATE_MULTITHREADED;

    // Create the device
	/* D3DCREATE_FPU_PRESERVE:
	There's also an issue about Direct X autonomously changing the internal FPU accuracy, 
	leading to inconsistent calculations. Direct3D by default changes the FPU to single precision 
	mode at program startup and does not change it back to improve speed 
	(This means that your double variables will behave like float). 
	To disable this behaviour, pass the D3DCREATE_FPU_PRESERVE flag to the CreateDevice call. 
	This might adversely affect D3D performance (not much though).
	
	D3DCREATE_NOWINDOWCHANGES :
	Let ParaEngine to manage window focus changes, such as Alt-tab and mouse activate. 
	*/

    hr = m_pD3D->CreateDevice( m_d3dSettings.AdapterOrdinal(), pDeviceInfo->DevType,
                               m_hWndFocus, behaviorFlags | D3DCREATE_FPU_PRESERVE /*| D3DCREATE_NOWINDOWCHANGES*/ , &m_d3dpp,
                               &m_pd3dDevice );
	
	//following code create nvidia perfhud device for performance testing --clayman
	/*
	UINT AdapterToUse=D3DADAPTER_DEFAULT;
	for (UINT Adapter=0;Adapter<m_pD3D->GetAdapterCount();Adapter++)
	{
		D3DADAPTER_IDENTIFIER9 Identifier;
		HRESULT Res;
		Res = m_pD3D->GetAdapterIdentifier(Adapter,0,&Identifier);
		if (strstr(Identifier.Description,"PerfHUD") != 0)
		{
			AdapterToUse=Adapter;
			break;
		}
		}

	hr = m_pD3D->CreateDevice( AdapterToUse, D3DDEVTYPE_REF,
		m_hWndFocus, D3DCREATE_HARDWARE_VERTEXPROCESSING , &m_d3dpp,
		&m_pd3dDevice );
	*/

	if(FAILED(hr))
	{
		OUTPUT_LOG("error: Can not create d3d device with the said settings: %d*%d\n", m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight);
		InterpretError(hr, __FILE__, __LINE__);
		// for web browser, sometimes it is not possible to create device due to unknown reason.  
		for (int i=0;i<3;i++)
		{
			::Sleep(1000);
			
			GetClientRect( m_hWnd, &m_rcWindowClient );
			m_nClientWidth = m_rcWindowClient.right - m_rcWindowClient.left;
			m_nClientHeight = m_rcWindowClient.bottom - m_rcWindowClient.top;
			m_d3dSettings.Windowed_Width = m_nClientWidth;
			m_d3dSettings.Windowed_Height = m_nClientHeight;

			m_d3dpp.BackBufferWidth  = m_d3dSettings.Windowed_Width;
			m_d3dpp.BackBufferHeight = m_d3dSettings.Windowed_Height;

			// some graphics card's multi-sample  quality will return 1 in CheckDeviceMultiSampleType, but actually fail when creating device. 
			// this will somehow fix the problem. 
			/*m_d3dSettings.Fullscreen_MultisampleType = D3DMULTISAMPLE_NONE;
			m_d3dSettings.Fullscreen_MultisampleQuality = 0;
			m_d3dSettings.Windowed_MultisampleQuality = D3DMULTISAMPLE_NONE;
			m_d3dSettings.Windowed_MultisampleQuality = 0;*/
			m_d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
			m_d3dpp.MultiSampleQuality = 0;
			

			OUTPUT_LOG("trying recreating 3d device with %d*%d\n", m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight);
			if(m_d3dpp.BackBufferWidth < 1)
				m_d3dpp.BackBufferWidth = 960;
			if(m_d3dpp.BackBufferHeight < 1)
				m_d3dpp.BackBufferHeight = 560;

			hr = m_pD3D->CreateDevice( m_d3dSettings.AdapterOrdinal(), pDeviceInfo->DevType,
				m_hWndFocus, behaviorFlags | D3DCREATE_FPU_PRESERVE, &m_d3dpp,
				&m_pd3dDevice );
			if( SUCCEEDED(hr) )
			{
				OUTPUT_LOG("Successfully created 3d device\n");
				break;
			}
		}
	}

    if( SUCCEEDED(hr) )
    {
        // When moving from fullscreen to windowed mode, it is important to
        // adjust the window size after recreating the device rather than
        // beforehand to ensure that you get the window size you want.  For
        // example, when switching from 640x480 fullscreen to windowed with
        // a 1000x600 window on a 1024x768 desktop, it is impossible to set
        // the window size to 1000x600 until after the display mode has
        // changed to 1024x768, because windows cannot be larger than the
        // desktop.
        if( m_bWindowed && !m_bIsExternalWindow)
        {
			// NOTE: this could be wrong if the window is a child window. For child window, always specify m_bIsExternalWindow to true.
            SetWindowPos( m_hWnd, HWND_NOTOPMOST,
                          m_rcWindowBounds.left, m_rcWindowBounds.top,
                          ( m_rcWindowBounds.right - m_rcWindowBounds.left ),
                          ( m_rcWindowBounds.bottom - m_rcWindowBounds.top ),
						  SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER);
        }

        // Store device Caps
        m_pd3dDevice->GetDeviceCaps( &m_d3dCaps );
        m_dwCreateFlags = behaviorFlags;

        // Store device description
        if( pDeviceInfo->DevType == D3DDEVTYPE_REF )
            lstrcpy( m_strDeviceStats, TEXT("REF") );
        else if( pDeviceInfo->DevType == D3DDEVTYPE_HAL )
            lstrcpy( m_strDeviceStats, TEXT("HAL") );
        else if( pDeviceInfo->DevType == D3DDEVTYPE_SW )
            lstrcpy( m_strDeviceStats, TEXT("SW") );

        if( behaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING &&
            behaviorFlags & D3DCREATE_PUREDEVICE )
        {
            if( pDeviceInfo->DevType == D3DDEVTYPE_HAL )
                lstrcat( m_strDeviceStats, TEXT(" (pure hw vp)") );
            else
                lstrcat( m_strDeviceStats, TEXT(" (simulated pure hw vp)") );
        }
        else if( behaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING )
        {
            if( pDeviceInfo->DevType == D3DDEVTYPE_HAL )
                lstrcat( m_strDeviceStats, TEXT(" (hw vp)") );
            else
                lstrcat( m_strDeviceStats, TEXT(" (simulated hw vp)") );
        }
        else if( behaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING )
        {
            if( pDeviceInfo->DevType == D3DDEVTYPE_HAL )
                lstrcat( m_strDeviceStats, TEXT(" (mixed vp)") );
            else
                lstrcat( m_strDeviceStats, TEXT(" (simulated mixed vp)") );
        }
        else if( behaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING )
        {
            lstrcat( m_strDeviceStats, TEXT(" (sw vp)") );
        }

        if( pDeviceInfo->DevType == D3DDEVTYPE_HAL )
        {
            // Be sure not to overflow m_strDeviceStats when appending the adapter 
            // description, since it can be long.  Note that the adapter description
            // is initially CHAR and must be converted to TCHAR.
            lstrcat( m_strDeviceStats, TEXT(": ") );
            const int cchDesc = sizeof(pAdapterInfo->AdapterIdentifier.Description);
            TCHAR szDescription[cchDesc];
            DXUtil_ConvertAnsiStringToGenericCch( szDescription, 
                pAdapterInfo->AdapterIdentifier.Description, cchDesc );
            int maxAppend = sizeof(m_strDeviceStats) / sizeof(TCHAR) -
                lstrlen( m_strDeviceStats ) - 1;
            _tcsncat( m_strDeviceStats, szDescription, maxAppend );
        }

        // Store render target surface desc
        LPDIRECT3DSURFACE9 pBackBuffer = NULL;
        m_pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
        pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
        pBackBuffer->Release();

        // Set up the fullscreen cursor
        if(  !m_bWindowed )
        {
//            HCURSOR hCursor;
//#ifdef _WIN64
//            hCursor = (HCURSOR)GetClassLongPtr( m_hWnd, GCLP_HCURSOR );
//#else
//            hCursor = (HCURSOR)ULongToHandle( GetClassLong( m_hWnd, GCL_HCURSOR ) );
//#endif
//            D3DUtil_SetDeviceCursor( m_pd3dDevice, hCursor, true );
//            m_pd3dDevice->ShowCursor( true );
			
        }

        // Confine cursor to full screen window
        if( m_bClipCursorWhenFullscreen )
        {
            if (!m_bWindowed )
            {
                RECT rcWindow;
                GetWindowRect( m_hWnd, &rcWindow );
                ClipCursor( &rcWindow );
            }
            else
            {
                ClipCursor( NULL );
            }
        }

        // Initialize the app's device-dependent objects
        hr = InitDeviceObjects();

        if( FAILED(hr) )
        {
            DeleteDeviceObjects();
        }
        else
        {
            m_bDeviceObjectsInited = true;
            hr = RestoreDeviceObjects();
            if( FAILED(hr) )
            {
                InvalidateDeviceObjects();
            }
            else
            {
                m_bDeviceObjectsRestored = true;
                return S_OK;
            }
        }

        // Cleanup before we try again
        Cleanup3DEnvironment();
    }

    // If that failed, fall back to the reference rasterizer
    if( hr != D3DAPPERR_MEDIANOTFOUND && 
        hr != HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) && 
        pDeviceInfo->DevType == D3DDEVTYPE_HAL )
    {
		OUTPUT_LOG("warning: Can not create d3d device with the said settings: %d*%d\n", m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight);
		InterpretError(hr, __FILE__, __LINE__);

        if (FindBestWindowedMode(false, true))
        {
            m_bWindowed = true;
			if(!m_bIsExternalWindow)
			{
				AdjustWindowForChange();
				// Make sure main window isn't topmost, so error message is visible
				SetWindowPos( m_hWnd, HWND_NOTOPMOST,
							  m_rcWindowBounds.left, m_rcWindowBounds.top,
							  ( m_rcWindowBounds.right - m_rcWindowBounds.left ),
							  ( m_rcWindowBounds.bottom - m_rcWindowBounds.top ),
							  SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER);
			}
            // Let the user know we are switching from HAL to the reference rasterizer
            DisplayErrorMsg( hr, MSGWARN_SWITCHEDTOREF );

			hr = Initialize3DEnvironment();
        }
    }
    return hr;
}




//-----------------------------------------------------------------------------
// Name: BuildPresentParamsFromSettings()
// Desc:
//-----------------------------------------------------------------------------
void CD3DApplication::BuildPresentParamsFromSettings()
{
	if(IsExternalD3DDevice())
		return;
    m_d3dpp.Windowed               = m_d3dSettings.IsWindowed;
    m_d3dpp.BackBufferCount        = 1;
    m_d3dpp.MultiSampleType        = m_d3dSettings.MultisampleType();
	m_d3dpp.MultiSampleQuality     = m_d3dSettings.MultisampleQuality();
    m_d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    m_d3dpp.EnableAutoDepthStencil = m_d3dEnumeration.AppUsesDepthBuffer;
    m_d3dpp.hDeviceWindow          = m_hWnd;
    if( m_d3dEnumeration.AppUsesDepthBuffer )
    {
        m_d3dpp.Flags              = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
        m_d3dpp.AutoDepthStencilFormat = m_d3dSettings.DepthStencilBufferFormat();
    }
    else
    {
        m_d3dpp.Flags              = 0;
    }

    if ( m_d3dSettings.DeviceClip() )
        m_d3dpp.Flags |= D3DPRESENTFLAG_DEVICECLIP;

	// make it lockable so that we can read back pixel data for CBufferPicking.  Not needed now.
	// m_d3dpp.Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

    if( m_bWindowed )
    {
        m_d3dpp.BackBufferWidth  = m_d3dSettings.Windowed_Width;
        m_d3dpp.BackBufferHeight = m_d3dSettings.Windowed_Height;
        m_d3dpp.BackBufferFormat = m_d3dSettings.PDeviceCombo()->BackBufferFormat;
        m_d3dpp.FullScreen_RefreshRateInHz = 0;
        m_d3dpp.PresentationInterval = m_d3dSettings.PresentInterval();
    }
    else
    {
        m_d3dpp.BackBufferWidth  = m_d3dSettings.DisplayMode().Width;
        m_d3dpp.BackBufferHeight = m_d3dSettings.DisplayMode().Height;
        m_d3dpp.BackBufferFormat = m_d3dSettings.PDeviceCombo()->BackBufferFormat;
        m_d3dpp.FullScreen_RefreshRateInHz = m_d3dSettings.Fullscreen_DisplayMode.RefreshRate;
        m_d3dpp.PresentationInterval = m_d3dSettings.PresentInterval();

        if( m_bAllowDialogBoxMode )
        {
            // Make the back buffers lockable in fullscreen mode
            // so we can show dialog boxes via SetDialogBoxMode() 
            // but since lockable back buffers incur a performance cost on 
            // some graphics hardware configurations we'll only 
            // enable lockable backbuffers where SetDialogBoxMode() would work.
            if ( (m_d3dpp.BackBufferFormat == D3DFMT_X1R5G5B5 || m_d3dpp.BackBufferFormat == D3DFMT_R5G6B5 || m_d3dpp.BackBufferFormat == D3DFMT_X8R8G8B8 ) &&
                ( m_d3dpp.MultiSampleType == D3DMULTISAMPLE_NONE ) &&
                ( m_d3dpp.SwapEffect == D3DSWAPEFFECT_DISCARD ) )
            {
                m_d3dpp.Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
            }
        }
    }
}




//-----------------------------------------------------------------------------
// Name: Reset3DEnvironment()
// Desc: Usually this function is not overridden.  Here's what this function does:
//       - Sets the windowed flag to be either windowed or fullscreen
//       - Sets parameters for z-buffer depth and back buffer
//       - Creates the D3D device
//       - Sets the window position (if windowed, that is)
//       - Makes some determinations as to the abilities of the driver (HAL, etc)
//       - Sets up some cursor stuff
//       - Calls InitDeviceObjects()
//       - Calls RestoreDeviceObjects()
//       - If all goes well, m_bActive is set to TRUE, and the function returns
//       - Otherwise, initialization is reattempted using the reference device
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::Reset3DEnvironment()
{
    HRESULT hr;

    // Release all video memory objects
    if( m_bDeviceObjectsRestored )
    {
        m_bDeviceObjectsRestored = false;
        InvalidateDeviceObjects();
    }
	
    // Reset the device
    if( FAILED( hr = m_pd3dDevice->Reset( &m_d3dpp ) ) )
	{
		OUTPUT_LOG("reset d3d device failed because Reset function failed: %d\n", hr);
		InterpretError(hr, __FILE__, __LINE__);
        return hr;
	}


    // Store render target surface desc
    LPDIRECT3DSURFACE9 pBackBuffer;
    m_pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
    pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
    pBackBuffer->Release();

    // Confine cursor to full screen window
    if( m_bClipCursorWhenFullscreen )
    {
        if (!m_bWindowed )
        {
            RECT rcWindow;
            GetWindowRect( m_hWnd, &rcWindow );
            ClipCursor( &rcWindow );
        }
        else
        {
            ClipCursor( NULL );
        }
    }

    // Initialize the app's device-dependent objects
    hr = RestoreDeviceObjects();
    if( FAILED(hr) )
    {
		OUTPUT_LOG("reset d3d device failed because Restore func failed: %d\n", hr);
        InvalidateDeviceObjects();
        return hr;
    }
    m_bDeviceObjectsRestored = true;

    // If the app is paused, trigger the rendering of the current frame
    if( false == m_bFrameMoving )
    {
		OUTPUT_LOG("render single frame");
        m_bSingleStep = true;
        DXUtil_Timer( TIMER_START );
        DXUtil_Timer( TIMER_STOP );
    }
	OUTPUT_LOG("reset d3d device succeed\n");
    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: ToggleFullScreen()
// Desc: Called when user toggles between fullscreen mode and windowed mode
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::ToggleFullscreen()
{
	if(IsExternalD3DDevice())
		return S_OK;
    HRESULT hr;
    int AdapterOrdinalOld = m_d3dSettings.AdapterOrdinal();
    D3DDEVTYPE DevTypeOld = m_d3dSettings.DevType();

    Pause( true );
	OUTPUT_LOG("ToggleFullscreen\n");
	bool bOldIgnoreSizeChange = m_bIgnoreSizeChange;
	m_bIgnoreSizeChange = true;

    // Toggle the windowed state
    m_bWindowed = !m_bWindowed;
    m_d3dSettings.IsWindowed = m_bWindowed;

    // Prepare window for windowed/fullscreen change
    AdjustWindowForChange();

    // If AdapterOrdinal and DevType are the same, we can just do a Reset().
    // If they've changed, we need to do a complete device teardown/rebuild.
    if (m_d3dSettings.AdapterOrdinal() == AdapterOrdinalOld &&
        m_d3dSettings.DevType() == DevTypeOld)
    {
        // Reset the 3D device
		OUTPUT_LOG("Reset the 3D device \n");
        BuildPresentParamsFromSettings();
        hr = Reset3DEnvironment();
    }
    else
    {
		OUTPUT_LOG("Cleanup 3D Environment\n");
        Cleanup3DEnvironment();
		//OUTPUT_LOG("Initialize 3D Environment\n");
        hr = Initialize3DEnvironment();
    }
    if( FAILED( hr ) )
    {
		OUTPUT_LOG("Failed to toggle screen mode\n");
        if( hr != D3DERR_OUTOFVIDEOMEMORY )
            hr = D3DAPPERR_RESETFAILED;
        m_bIgnoreSizeChange = false;
		if( !m_bWindowed && !m_bIsExternalWindow)
        {
			OUTPUT_LOG("Restore window type to windowed mode\n");
            // Restore window type to windowed mode
            m_bWindowed = !m_bWindowed;
            m_d3dSettings.IsWindowed = m_bWindowed;
            AdjustWindowForChange();
            SetWindowPos( m_hWnd, HWND_NOTOPMOST,
                        m_rcWindowBounds.left, m_rcWindowBounds.top,
                        ( m_rcWindowBounds.right - m_rcWindowBounds.left ),
                        ( m_rcWindowBounds.bottom - m_rcWindowBounds.top ),
                        SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER);
        }
        return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
    }

    m_bIgnoreSizeChange = bOldIgnoreSizeChange;
	

    // When moving from fullscreen to windowed mode, it is important to
    // adjust the window size after resetting the device rather than
    // beforehand to ensure that you get the window size you want.  For
    // example, when switching from 640x480 fullscreen to windowed with
    // a 1000x600 window on a 1024x768 desktop, it is impossible to set
    // the window size to 1000x600 until after the display mode has
    // changed to 1024x768, because windows cannot be larger than the
    // desktop.
	if( m_bWindowed && !m_bIsExternalWindow)
    {
        SetWindowPos( m_hWnd, HWND_NOTOPMOST,
                      m_rcWindowBounds.left, m_rcWindowBounds.top,
                      ( m_rcWindowBounds.right - m_rcWindowBounds.left ),
                      ( m_rcWindowBounds.bottom - m_rcWindowBounds.top ),
                      SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER);
    }

    GetClientRect( m_hWnd, &m_rcWindowClient );  // Update our copy

    Pause( false );
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ForceWindowed()
// Desc: Switch to a windowed mode, even if that means picking a new device
//       and/or adapter
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::ForceWindowed()
{
	if(IsExternalD3DDevice())
		return S_OK;
    HRESULT hr;

    if( m_bWindowed )
        return S_OK;

    if( !FindBestWindowedMode(false, false) )
    {
        return E_FAIL;
    }
    m_bWindowed = true;

    // Now destroy the current 3D device objects, then reinitialize

    Pause( true );

    // Release all scene objects that will be re-created for the new device
    Cleanup3DEnvironment();

    // Create the new device
    if( FAILED(hr = Initialize3DEnvironment() ) )
        return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );

    Pause( false );
    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: AdjustWindowForChange()
// Desc: Prepare the window for a possible change between windowed mode and
//       fullscreen mode.  This function is virtual and thus can be overridden
//       to provide different behavior, such as switching to an entirely
//       different window for full screen mode (as in the MFC sample apps).
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::AdjustWindowForChange()
{
	if(IsExternalD3DDevice() || m_bIsExternalWindow)
		return S_OK;
    if( m_bWindowed )
    {
        // Set windowed-mode style
        SetWindowLong( m_hWnd, GWL_STYLE, m_dwWindowStyle );
        if( m_hMenu != NULL )
        {
            SetMenu( m_hWnd, m_hMenu );
            m_hMenu = NULL;
        }
    }
    else
    {
        // Set full screen mode style
        SetWindowLong( m_hWnd, GWL_STYLE, WS_POPUP|WS_SYSMENU|WS_VISIBLE );
        if( m_hMenu == NULL )
        {
            m_hMenu = GetMenu( m_hWnd );
            SetMenu( m_hWnd, NULL );
        }
    }
    return S_OK;
}
#pragma endregion DevicesAndEvents

#pragma region Miscs

//-----------------------------------------------------------------------------
// Name: UserSelectNewDevice()
// Desc: Displays a dialog so the user can select a new adapter, device, or
//       display mode, and then recreates the 3D environment if needed
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::UserSelectNewDevice()
{
	if(IsExternalD3DDevice())
		return S_OK;
    HRESULT hr;
    bool bDialogBoxMode = false;
    bool bOldWindowed = m_bWindowed;  // Preserve original windowed flag

    if( m_bWindowed == false )
    {
        // See if the current settings comply with the rules
        // for allowing SetDialogBoxMode().  
        if( (m_d3dpp.BackBufferFormat == D3DFMT_X1R5G5B5 || m_d3dpp.BackBufferFormat == D3DFMT_R5G6B5 || m_d3dpp.BackBufferFormat == D3DFMT_X8R8G8B8 ) &&
            ( m_d3dpp.MultiSampleType == D3DMULTISAMPLE_NONE ) &&
            ( m_d3dpp.SwapEffect == D3DSWAPEFFECT_DISCARD ) &&
            ( (m_d3dpp.Flags & D3DPRESENTFLAG_LOCKABLE_BACKBUFFER) == D3DPRESENTFLAG_LOCKABLE_BACKBUFFER ) &&
            ( (m_dwCreateFlags & D3DCREATE_ADAPTERGROUP_DEVICE) != D3DCREATE_ADAPTERGROUP_DEVICE ) )
        {
            if( SUCCEEDED( m_pd3dDevice->SetDialogBoxMode( true ) ) )
                bDialogBoxMode = true;
        }

        // If SetDialogBoxMode(true) didn't work then we can't display dialogs  
        // in fullscreen mode so we'll go back to windowed mode
        if( FALSE == bDialogBoxMode )
        {
            if( FAILED( ToggleFullscreen() ) )
            {
                DisplayErrorMsg( D3DAPPERR_RESETFAILED, MSGERR_APPMUSTEXIT );
                return E_FAIL;
            }
        }
    }

    // The dialog should use the mode the sample runs in, not
    // the mode that the dialog runs in.
    CD3DSettings tempSettings = m_d3dSettings;
    tempSettings.IsWindowed = bOldWindowed;
    CD3DSettingsDialog settingsDialog( &m_d3dEnumeration, &tempSettings);
	INT_PTR nResult = settingsDialog.ShowDialog( m_hWnd, CGlobals::GetApp()->GetModuleHandle());

    // Before creating the device, switch back to SetDialogBoxMode(false) 
    // mode to allow the user to pick multisampling or backbuffer formats 
    // not supported by SetDialogBoxMode(true) but typical apps wouldn't 
    // need to switch back.
    if( bDialogBoxMode )
        m_pd3dDevice->SetDialogBoxMode( false );

    if( nResult != IDOK )
    {
        // If we had to switch mode to display the dialog, we
        // need to go back to the original mode the sample
        // was running in.
        if( bOldWindowed != m_bWindowed && FAILED( ToggleFullscreen() ) )
        {
            DisplayErrorMsg( D3DAPPERR_RESETFAILED, MSGERR_APPMUSTEXIT );
            return E_FAIL;
        }

        return S_OK;
    }

    settingsDialog.GetFinalSettings( &m_d3dSettings );

    m_bWindowed = m_d3dSettings.IsWindowed;

	// Release all scene objects that will be re-created for the new device
    Cleanup3DEnvironment();

    // Inform the display class of the change. It will internally
    // re-create valid surfaces, a d3ddevice, etc.
    if( FAILED( hr = Initialize3DEnvironment() ) )
    {
        if( hr != D3DERR_OUTOFVIDEOMEMORY )
            hr = D3DAPPERR_RESETFAILED;
		if( !m_bWindowed && !m_bIsExternalWindow)
        {
            // Restore window type to windowed mode
            m_bWindowed = !m_bWindowed;
            m_d3dSettings.IsWindowed = m_bWindowed;
            AdjustWindowForChange();
            SetWindowPos( m_hWnd, HWND_NOTOPMOST,
                        m_rcWindowBounds.left, m_rcWindowBounds.top,
                        ( m_rcWindowBounds.right - m_rcWindowBounds.left ),
                        ( m_rcWindowBounds.bottom - m_rcWindowBounds.top ),
						SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER);
        }
        return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
    }

    // If the app is paused, trigger the rendering of the current frame
    if( false == m_bFrameMoving )
    {
        m_bSingleStep = true;
        DXUtil_Timer( TIMER_START );
        DXUtil_Timer( TIMER_STOP );
    }

    return S_OK;
}


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

        TCHAR strFmt[100];
        D3DFORMAT fmtAdapter = m_d3dSettings.DisplayMode().Format;
        if( fmtAdapter == m_d3dsdBackBuffer.Format )
        {
            lstrcpyn( strFmt, D3DUtil_D3DFormatToString( fmtAdapter, false ), 100 );
        }
        else
        {
            _sntprintf( strFmt, 100, TEXT("backbuf %s, adapter %s"), 
                D3DUtil_D3DFormatToString( m_d3dsdBackBuffer.Format, false ), 
                D3DUtil_D3DFormatToString( fmtAdapter, false ) );
        }
        strFmt[99] = TEXT('\0');

        TCHAR strDepthFmt[100];
        if( m_d3dEnumeration.AppUsesDepthBuffer )
        {
            _sntprintf( strDepthFmt, 100, TEXT(" (%s)"), 
                D3DUtil_D3DFormatToString( m_d3dSettings.DepthStencilBufferFormat(), false ) );
            strDepthFmt[99] = TEXT('\0');
        }
        else
        {
            // No depth buffer
            strDepthFmt[0] = TEXT('\0');
        }

        TCHAR* pstrMultiSample;
        switch( m_d3dSettings.MultisampleType() )
        {
        case D3DMULTISAMPLE_NONMASKABLE:  pstrMultiSample = TEXT(" (Nonmaskable Multisample)"); break;
        case D3DMULTISAMPLE_2_SAMPLES:  pstrMultiSample = TEXT(" (2x Multisample)"); break;
        case D3DMULTISAMPLE_3_SAMPLES:  pstrMultiSample = TEXT(" (3x Multisample)"); break;
        case D3DMULTISAMPLE_4_SAMPLES:  pstrMultiSample = TEXT(" (4x Multisample)"); break;
        case D3DMULTISAMPLE_5_SAMPLES:  pstrMultiSample = TEXT(" (5x Multisample)"); break;
        case D3DMULTISAMPLE_6_SAMPLES:  pstrMultiSample = TEXT(" (6x Multisample)"); break;
        case D3DMULTISAMPLE_7_SAMPLES:  pstrMultiSample = TEXT(" (7x Multisample)"); break;
        case D3DMULTISAMPLE_8_SAMPLES:  pstrMultiSample = TEXT(" (8x Multisample)"); break;
        case D3DMULTISAMPLE_9_SAMPLES:  pstrMultiSample = TEXT(" (9x Multisample)"); break;
        case D3DMULTISAMPLE_10_SAMPLES: pstrMultiSample = TEXT(" (10x Multisample)"); break;
        case D3DMULTISAMPLE_11_SAMPLES: pstrMultiSample = TEXT(" (11x Multisample)"); break;
        case D3DMULTISAMPLE_12_SAMPLES: pstrMultiSample = TEXT(" (12x Multisample)"); break;
        case D3DMULTISAMPLE_13_SAMPLES: pstrMultiSample = TEXT(" (13x Multisample)"); break;
        case D3DMULTISAMPLE_14_SAMPLES: pstrMultiSample = TEXT(" (14x Multisample)"); break;
        case D3DMULTISAMPLE_15_SAMPLES: pstrMultiSample = TEXT(" (15x Multisample)"); break;
        case D3DMULTISAMPLE_16_SAMPLES: pstrMultiSample = TEXT(" (16x Multisample)"); break;
        default:                        pstrMultiSample = TEXT(""); break;
        }

        const int cchMaxFrameStats = sizeof(m_strFrameStats) / sizeof(TCHAR);
        _sntprintf( m_strFrameStats, cchMaxFrameStats, _T("%.02f fps (%dx%d), %s%s%s"), m_fFPS,
                    m_d3dsdBackBuffer.Width, m_d3dsdBackBuffer.Height,
                    strFmt, strDepthFmt, pstrMultiSample );
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
				if (!IsExternalD3DDevice())
					DisplayErrorMsg( D3DAPPERR_NONZEROREFCOUNT, MSGERR_APPMUSTEXIT );
			}
		}
		long nRefCount = 0;
		if ((nRefCount=m_pd3dDevice->Release()) > 0) {
			if (!IsExternalD3DDevice()){
				OUTPUT_LOG("\n\nerror: reference count of d3ddevice is non-zero %d when exit\n\n", nRefCount);
				// DisplayErrorMsg(D3DAPPERR_NONZEROREFCOUNT, MSGERR_APPMUSTEXIT); 
			}
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

//-----------------------------------------------------------------------------
// Name: LaunchReadme()
// Desc: Ensures the app is windowed, and launches the readme.txt 
//       in the default text editor
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::LaunchReadme()
{
    // Switch to windowed if launching the readme.txt
    if( m_bWindowed == false )
    {
        if( FAILED( ToggleFullscreen() ) )
        {
            DisplayErrorMsg( D3DAPPERR_RESETFAILED, MSGERR_APPMUSTEXIT );
            return E_FAIL;
        }
    }

    DXUtil_LaunchReadme( m_hWnd );

    return S_OK;
}

int CD3DApplication::Run(HINSTANCE hInstance)
{
	// CD3DWindowDefault can be used as a sample to create application using paraengine lib. 
	CD3DWindowDefault defaultWin;
	defaultWin.SetAppInterface(CParaEngineApp::GetInstance());
	
	return defaultWin.Run(hInstance);
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


#pragma endregion Miscs