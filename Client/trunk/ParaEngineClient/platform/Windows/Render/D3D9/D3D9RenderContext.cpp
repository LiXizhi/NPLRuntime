#include "ParaEngine.h"
#include "D3D9RenderContext.h"
#include "PEtypes.h"
#include "D3DWindowUtil.h"

using namespace ParaEngine;
D3D9RenderContext::D3D9RenderContext(WindowsRenderWindow * window)
	:m_pRenderWindow(window)
	,m_pD3D9(nullptr)
	,m_pD3Device(nullptr)
	,m_invalid(false)
{

}

bool ParaEngine::D3D9RenderContext::Initialize()
{

	HRESULT hr = S_FALSE;

	m_pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
	if (m_pD3D9 == nullptr) return false;

	m_d3dEnumeration.SetD3D(m_pD3D9);
	m_d3dEnumeration.ConfirmDeviceCallback = ConfirmDeviceHelper;
	if (FAILED(hr = m_d3dEnumeration.Enumerate()))
	{
		SAFE_RELEASE(m_pD3D9);
		return false;
	}

	if (FAILED(hr = ChooseInitialD3DSettings()))
	{
		SAFE_RELEASE(m_pD3D9);
		return false;
	}

	if (FAILED(hr = Initialize3DEnvironment()))
	{
		SAFE_RELEASE(m_pD3D9);
		return false;
	}

	m_invalid = false;
	return true;
}

void ParaEngine::D3D9RenderContext::Uninitialize()
{
	if (m_invalid)return;
	if (m_pD3Device)
	{
		m_pD3Device->Release();
		m_pD3Device = nullptr;
	}
	if (m_pD3D9)
	{
		m_pD3D9->Release();
		m_pD3D9 = nullptr;
	}
	m_invalid = true;
}



bool ParaEngine::D3D9RenderContext::IsInvalid() const
{
	return m_invalid;
}

HRESULT D3D9RenderContext::Initialize3DEnvironment()
{
	HRESULT hr = S_OK;
	D3DAdapterInfo* pAdapterInfo = m_d3dSettings.PAdapterInfo();
	D3DDeviceInfo* pDeviceInfo = m_d3dSettings.PDeviceInfo();



	if (pDeviceInfo->Caps.PrimitiveMiscCaps & D3DPMISCCAPS_NULLREFERENCE)
	{
		// Warn user about null ref device that can't render anything
		return E_FAIL;
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

	hr = m_pD3D9->CreateDevice(m_d3dSettings.AdapterOrdinal(), pDeviceInfo->DevType,
		m_pRenderWindow->GetHandle(), behaviorFlags | D3DCREATE_FPU_PRESERVE /*| D3DCREATE_NOWINDOWCHANGES*/, &m_d3dpp,
		&m_pD3Device);

	if (FAILED(hr))
	{
		OUTPUT_LOG("error: Can not create d3d device with the said settings: %d*%d\n", m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight);
		InterpretError(hr, __FILE__, __LINE__);
		// for web browser, sometimes it is not possible to create device due to unknown reason.  
		for (int i = 0; i<3; i++)
		{
			::Sleep(1000);
			m_d3dSettings.Windowed_Width = m_pRenderWindow->GetWidth();
			m_d3dSettings.Windowed_Height = m_pRenderWindow->GetHeight();

			m_d3dpp.BackBufferWidth = m_d3dSettings.Windowed_Width;
			m_d3dpp.BackBufferHeight = m_d3dSettings.Windowed_Height;

			m_d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
			m_d3dpp.MultiSampleQuality = 0;


			OUTPUT_LOG("trying recreating 3d device with %d*%d\n", m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight);
			if (m_d3dpp.BackBufferWidth < 1)
				m_d3dpp.BackBufferWidth = 960;
			if (m_d3dpp.BackBufferHeight < 1)
				m_d3dpp.BackBufferHeight = 560;

			hr = m_pD3D9->CreateDevice(m_d3dSettings.AdapterOrdinal(), pDeviceInfo->DevType,
				m_pRenderWindow->GetHandle(), behaviorFlags | D3DCREATE_FPU_PRESERVE, &m_d3dpp,
				&m_pD3Device);
			if (SUCCEEDED(hr))
			{
				OUTPUT_LOG("Successfully created 3d device\n");
				break;
			}
		}
	}
	return hr;
}



//-----------------------------------------------------------------------------
// Name: FindBestFullscreenMode()
// Desc: Sets up m_d3dSettings with best available fullscreen mode, subject to 
//       the bRequireHAL and bRequireREF constraints.  Returns false if no such
//       mode can be found.
//-----------------------------------------------------------------------------
bool D3D9RenderContext::FindBestFullscreenMode(bool bRequireHAL, bool bRequireREF)
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

	for (UINT iai = 0; iai < m_d3dEnumeration.m_pAdapterInfoList->Count(); iai++)
	{
		D3DAdapterInfo* pAdapterInfo = (D3DAdapterInfo*)m_d3dEnumeration.m_pAdapterInfoList->GetPtr(iai);
		m_pD3D9->GetAdapterDisplayMode(pAdapterInfo->AdapterOrdinal, &adapterDesktopDisplayMode);
		for (UINT idi = 0; idi < pAdapterInfo->pDeviceInfoList->Count(); idi++)
		{
			D3DDeviceInfo* pDeviceInfo = (D3DDeviceInfo*)pAdapterInfo->pDeviceInfoList->GetPtr(idi);
			if (bRequireHAL && pDeviceInfo->DevType != D3DDEVTYPE_HAL)
				continue;
			if (bRequireREF && pDeviceInfo->DevType != D3DDEVTYPE_REF)
				continue;
			for (UINT idc = 0; idc < pDeviceInfo->pDeviceComboList->Count(); idc++)
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
					pDeviceCombo->DevType == D3DDEVTYPE_HAL && bAdapterMatchesDesktop && (bAdapterMatchesBB || pDeviceCombo->BackBufferFormat == D3DFMT_A8R8G8B8))
				{
					bestAdapterDesktopDisplayMode = adapterDesktopDisplayMode;
					pBestAdapterInfo = pAdapterInfo;
					pBestDeviceInfo = pDeviceInfo;
					pBestDeviceCombo = pDeviceCombo;
					if (pDeviceInfo->DevType == D3DDEVTYPE_HAL && bAdapterMatchesDesktop && (pDeviceCombo->BackBufferFormat == D3DFMT_A8R8G8B8))
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
	for (UINT idm = 0; idm < pBestAdapterInfo->pDisplayModeList->Count(); idm++)
	{
		D3DDISPLAYMODE* pdm = (D3DDISPLAYMODE*)pBestAdapterInfo->pDisplayModeList->GetPtr(idm);
		if (pdm->Format != pBestDeviceCombo->AdapterFormat)
			continue;
		if (pdm->Width == bestAdapterDesktopDisplayMode.Width &&
			pdm->Height == bestAdapterDesktopDisplayMode.Height &&
			pdm->RefreshRate == bestAdapterDesktopDisplayMode.RefreshRate)
		{
			// found a perfect match, so stop
			bestDisplayMode = *pdm;
			break;
		}
		else if (pdm->Width == bestAdapterDesktopDisplayMode.Width &&
			pdm->Height == bestAdapterDesktopDisplayMode.Height &&
			pdm->RefreshRate > bestDisplayMode.RefreshRate)
		{
			// refresh rate doesn't match, but width/height match, so keep this
			// and keep looking
			bestDisplayMode = *pdm;
		}
		else if (pdm->Width == bestAdapterDesktopDisplayMode.Width)
		{
			// width matches, so keep this and keep looking
			bestDisplayMode = *pdm;
		}
		else if (bestDisplayMode.Width == 0)
		{
			// we don't have anything better yet, so keep this and keep looking
			bestDisplayMode = *pdm;
		}
	}


	{
		// By LiXizhi 2008.7.5: Find the closest match to the (m_dwCreationWidth, m_dwCreationHeight)
		// it is either a perfect match or a match that is slightly bigger than (m_dwCreationWidth, m_dwCreationHeight)
		// by LXZ: now we try to find (m_dwCreationWidth, m_dwCreationHeight) if any
		for (UINT idm = 0; idm < pBestAdapterInfo->pDisplayModeList->Count(); idm++)
		{
			D3DDISPLAYMODE* pdm = (D3DDISPLAYMODE*)pBestAdapterInfo->pDisplayModeList->GetPtr(idm);
			if (pdm->Format != pBestDeviceCombo->AdapterFormat)
				continue;
			if (pdm->Width == m_pRenderWindow->GetWidth() &&
				pdm->Height == m_pRenderWindow->GetHeight() &&
				pdm->RefreshRate == bestAdapterDesktopDisplayMode.RefreshRate)
			{
				// found a perfect match, so stop
				bestDisplayMode = *pdm;
				break;
			}
			else if (pdm->Width >= m_pRenderWindow->GetWidth() && pdm->Height >= m_pRenderWindow->GetHeight()
				&& (pdm->Width <= bestAdapterDesktopDisplayMode.Width || bestAdapterDesktopDisplayMode.Width < m_pRenderWindow->GetWidth())
				&& (pdm->Height <= bestAdapterDesktopDisplayMode.Height || bestAdapterDesktopDisplayMode.Height < m_pRenderWindow->GetHeight()))
			{
				// OUTPUT_LOG("candidate window (%d, %d)\n", pdm->Width, pdm->Height);
				// width/height is bigger, so keep this and keep looking
				if (bestDisplayMode.Width < m_pRenderWindow->GetWidth() || bestDisplayMode.Height < m_pRenderWindow->GetHeight())
				{
					// if the best display mode is not big enough, select a bigger one. 
					bestDisplayMode = *pdm;
				}
				else if (pdm->Width < bestDisplayMode.Width || pdm->Height < bestDisplayMode.Height)
				{
					// if the new one size is more close to the creation size, keep it.
					if (((bestDisplayMode.Width - pdm->Width)*m_pRenderWindow->GetHeight() + (bestDisplayMode.Height - pdm->Height)*m_pRenderWindow->GetWidth()) > 0)
					{
						bestDisplayMode = *pdm;
					}
				}
				else if ((pdm->Width == bestDisplayMode.Width && pdm->Height == bestDisplayMode.Height) && pdm->RefreshRate == bestAdapterDesktopDisplayMode.RefreshRate)
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
		for (UINT ims = 0; ims < pBestDeviceCombo->pMultiSampleTypeList->Count(); ims++)
		{
			D3DMULTISAMPLE_TYPE msType = *(D3DMULTISAMPLE_TYPE*)pBestDeviceCombo->pMultiSampleTypeList->GetPtr(ims);
			if (m_d3dSettings.Fullscreen_MultisampleType == msType)
			{
				bFound = true;
				DWORD maxQuality = *(DWORD*)pBestDeviceCombo->pMultiSampleQualityList->GetPtr(ims);
				if (maxQuality < m_d3dSettings.Fullscreen_MultisampleQuality)
				{
					OUTPUT_LOG("warning: MultiSampleQuality has maximum value for your device is %d. However you are setting it to %d. We will disable AA.\r\n", maxQuality, m_d3dSettings.Fullscreen_MultisampleQuality);
					m_d3dSettings.Fullscreen_MultisampleType = D3DMULTISAMPLE_NONE;
					m_d3dSettings.Fullscreen_MultisampleQuality = 0;
				}
				break;
			}
		}
		if (!bFound) {
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
// Name: FindBestWindowedMode()
// Desc: Sets up m_d3dSettings with best available windowed mode, subject to 
//       the bRequireHAL and bRequireREF constraints.  Returns false if no such
//       mode can be found.
//-----------------------------------------------------------------------------
bool D3D9RenderContext::FindBestWindowedMode(bool bRequireHAL, bool bRequireREF)
{
	// Get display mode of primary adapter (which is assumed to be where the window 
	// will appear)
	D3DDISPLAYMODE primaryDesktopDisplayMode;
	m_pD3D9->GetAdapterDisplayMode(0, &primaryDesktopDisplayMode);

	D3DAdapterInfo* pBestAdapterInfo = NULL;
	D3DDeviceInfo* pBestDeviceInfo = NULL;
	D3DDeviceCombo* pBestDeviceCombo = NULL;

	for (UINT iai = 0; iai < m_d3dEnumeration.m_pAdapterInfoList->Count(); iai++)
	{
		D3DAdapterInfo* pAdapterInfo = (D3DAdapterInfo*)m_d3dEnumeration.m_pAdapterInfoList->GetPtr(iai);
		for (UINT idi = 0; idi < pAdapterInfo->pDeviceInfoList->Count(); idi++)
		{
			D3DDeviceInfo* pDeviceInfo = (D3DDeviceInfo*)pAdapterInfo->pDeviceInfoList->GetPtr(idi);
			if (bRequireHAL && pDeviceInfo->DevType != D3DDEVTYPE_HAL)
				continue;
			if (bRequireREF && pDeviceInfo->DevType != D3DDEVTYPE_REF)
				continue;
			for (UINT idc = 0; idc < pDeviceInfo->pDeviceComboList->Count(); idc++)
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
				if (pBestDeviceCombo == NULL ||
					pBestDeviceCombo->DevType != D3DDEVTYPE_HAL && pDeviceCombo->DevType == D3DDEVTYPE_HAL ||
					pDeviceCombo->DevType == D3DDEVTYPE_HAL && (bAdapterMatchesBB || pDeviceCombo->BackBufferFormat == D3DFMT_A8R8G8B8))
				{
					pBestAdapterInfo = pAdapterInfo;
					pBestDeviceInfo = pDeviceInfo;
					pBestDeviceCombo = pDeviceCombo;
					if (pDeviceCombo->DevType == D3DDEVTYPE_HAL && (pDeviceCombo->BackBufferFormat == D3DFMT_A8R8G8B8))
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
	if (pBestDeviceCombo == NULL)
		return false;

	m_d3dSettings.pWindowed_AdapterInfo = pBestAdapterInfo;
	m_d3dSettings.pWindowed_DeviceInfo = pBestDeviceInfo;
	m_d3dSettings.pWindowed_DeviceCombo = pBestDeviceCombo;
	m_d3dSettings.IsWindowed = true;
	m_d3dSettings.Windowed_DisplayMode = primaryDesktopDisplayMode;
	m_d3dSettings.Windowed_Width = m_pRenderWindow->GetWidth();
	m_d3dSettings.Windowed_Height = m_pRenderWindow->GetHeight();
	if (m_d3dEnumeration.AppUsesDepthBuffer)
		m_d3dSettings.Windowed_DepthStencilBufferFormat = *(D3DFORMAT*)pBestDeviceCombo->pDepthStencilFormatList->GetPtr(0);

	{
		// by LXZ: now we try to find m_d3dSettings.Fullscreen_MultisampleType, m_d3dSettings.Fullscreen_MultisampleQuality
		bool bFound = false;
		for (UINT ims = 0; ims < pBestDeviceCombo->pMultiSampleTypeList->Count(); ims++)
		{
			D3DMULTISAMPLE_TYPE msType = *(D3DMULTISAMPLE_TYPE*)pBestDeviceCombo->pMultiSampleTypeList->GetPtr(ims);
			if (m_d3dSettings.Windowed_MultisampleType == msType)
			{
				bFound = true;
				DWORD maxQuality = *(DWORD*)pBestDeviceCombo->pMultiSampleQualityList->GetPtr(ims);
				if (maxQuality < m_d3dSettings.Windowed_MultisampleQuality)
				{
					OUTPUT_LOG("warning: MultiSampleQuality has maximum value for your device is %d. However you are setting it to %d. We will disable AA.\r\n", maxQuality, m_d3dSettings.Windowed_MultisampleQuality);
					m_d3dSettings.Windowed_MultisampleType = D3DMULTISAMPLE_NONE;
					m_d3dSettings.Windowed_MultisampleQuality = 0;
				}
				break;
			}
		}
		if (!bFound) {
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
// Name: ChooseInitialD3DSettings()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT D3D9RenderContext::ChooseInitialD3DSettings()
{
	bool bFoundFullscreen = FindBestFullscreenMode(false, false);
	bool bFoundWindowed = FindBestWindowedMode(false, false);
	m_d3dSettings.SetDeviceClip(false);
	if (!m_pRenderWindow->IsWindowed() && bFoundFullscreen)
			m_d3dSettings.IsWindowed = false;
		if (!bFoundWindowed && bFoundFullscreen)
			m_d3dSettings.IsWindowed = false;


	if (!bFoundFullscreen && !bFoundWindowed)
		return D3DAPPERR_NOCOMPATIBLEDEVICES;

	return S_OK;
}





//-----------------------------------------------------------------------------
// Name: ConfirmDeviceHelper()
// Desc: Static function used by D3DEnumeration
//-----------------------------------------------------------------------------
bool D3D9RenderContext::ConfirmDeviceHelper(D3DCAPS9* pCaps, VertexProcessingType vertexProcessingType,
	D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat)
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


}