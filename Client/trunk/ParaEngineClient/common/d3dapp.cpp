
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

static CD3DApplication* g_pD3DApp = NULL;
CFrameRateController g_doWorkFRC(CFrameRateController::FRC_CONSTANT_OR_BELOW);

CD3DApplication::CD3DApplication()
{
    g_pD3DApp           = this;

	SetAppState(PEAppState_None);

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

	m_pRenderDevice = NULL;
	m_pRenderContext = nullptr;

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
	
	if(!m_bDisableD3D)
	{

		m_pRenderContext = D3D9RenderContext::Create();

		// Create the Direct3D object
		m_pD3D = static_cast<D3D9RenderContext*>(m_pRenderContext)->GetD3D();

		if( m_pD3D == NULL )
			return DisplayErrorMsg( D3DAPPERR_NODIRECT3D, MSGERR_APPMUSTEXIT );

		// Build a list of Direct3D adapters, modes and devices. The
		// ConfirmDevice() callback is used to confirm that only devices that
		// meet the app's requirements are considered.
		m_d3dEnumeration.SetD3D( m_pD3D );
		m_d3dEnumeration.ConfirmDeviceCallback = NULL;
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

							if(!m_bWindowed)
							{
								// if user toggles devices during full screen mode, we will switch to windowed mode. 
								ToggleFullscreen();
							}
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
	hr = m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
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
    bool bFoundFullscreen = FindBestFullscreenMode( false, false );
    bool bFoundWindowed = FindBestWindowedMode( false, false );
    m_d3dSettings.SetDeviceClip( false );

	// For external window, always start in windowed mode, this fixed a bug for web browser plugin. 

		if( m_bStartFullscreen && bFoundFullscreen)
			m_d3dSettings.IsWindowed = false;
		if( !bFoundWindowed && bFoundFullscreen )
			m_d3dSettings.IsWindowed = false;
	

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
 //   RECT rcClientOld;
 //   rcClientOld = m_rcWindowClient;

 //   // Update window properties
	GetWindowRect(m_hWnd, &m_rcWindowBounds);
	GetClientRect(m_hWnd, &m_rcWindowClient);
	m_nClientWidth = m_rcWindowClient.right - m_rcWindowClient.left;
	m_nClientHeight = m_rcWindowClient.bottom - m_rcWindowClient.top;

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

HRESULT CD3DApplication::Initialize3DEnvironment()
{
    HRESULT hr = S_OK;


    m_bWindowed = m_d3dSettings.IsWindowed;

    // Prepare window for possible windowed/fullscreen change
    AdjustWindowForChange();

    // Set up the presentation parameters
    BuildPresentParamsFromSettings();


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




//-----------------------------------------------------------------------------
// Name: BuildPresentParamsFromSettings()
// Desc:
//-----------------------------------------------------------------------------
void CD3DApplication::BuildPresentParamsFromSettings()
{
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
//       - Makes some determinations as to the abilites of the driver (HAL, etc)
//       - Sets up some cursor stuff
//       - Calls InitDeviceObjects()
//       - Calls RestoreDeviceObjects()
//       - If all goes well, m_bActive is set to TRUE, and the function returns
//       - Otherwise, initialization is reattempted using the reference device
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::Reset3DEnvironment()
{
    HRESULT hr;

    // Release all vidmem objects
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


//-----------------------------------------------------------------------------
// Name: ToggleFullScreen()
// Desc: Called when user toggles between fullscreen mode and windowed mode
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::ToggleFullscreen()
{
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
		if( !m_bWindowed )
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
	if( m_bWindowed)
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