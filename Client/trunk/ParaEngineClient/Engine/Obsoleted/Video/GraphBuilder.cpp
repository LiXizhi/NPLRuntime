//----------------------------------------------------------------------
// Class:	CGraphBuilder
// Authors:	Liu Weili
// Date:	2006.3.9
//
// desc: 
// The GraphBuilder for DirectShow. This class builds an object to control loading and playing of a video clip.
// Some codes are from DirectShow sample. I modified theo code so it is more consice and effective. 
//
//----------------------------------------------------------------------

#include "DxStdAfx.h"
#include "ParaEngine.h"
#include "GraphBuilder.h"
#include "memdebug.h"

using namespace ParaEngine;
CGraphBuilder::CGraphBuilder()
{

	m_pGB=NULL;
	m_pME=NULL;
	m_pMC=NULL;
	m_pMP=NULL;
	m_pFileSource=NULL;
	m_pPinOut0=NULL;
	m_pPinOut1=NULL;
	m_pRenderer=NULL;
	m_bAudioOn=false;
	m_bLocked=false;
	m_pTexture=NULL;
}

CGraphBuilder::~CGraphBuilder()
{
	CleanupDShow();
}

HRESULT CGraphBuilder::RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{
	HRESULT hr=S_OK;
	if (FAILED(hr=m_pRenderer->CreateTexture(&m_pTexture))) {
		OUTPUT_LOG(TEXT("Failed creating the render texture for the video file"), hr);
		return hr;
	}
	return hr;
}

HRESULT CGraphBuilder::InvalidateDeviceObjects()
{
	StopGraph();
	SAFE_RELEASE(m_pTexture);
	return S_OK;
}
HRESULT CGraphBuilder::LoadVideoFile( LPCWSTR wFileName, bool bAudioOn)
{
	HRESULT hr = S_OK;
	try
	{
		if (m_bLocked) {
			CleanupDShow();
		}
		m_bAudioOn=bAudioOn;
		// Create the filter graph
		if (FAILED(m_pGB.CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC)))
			return E_FAIL;

		// Create the Texture Renderer object
		m_pRenderer = new CTextureRenderer(NULL, &hr);
		if (FAILED(hr) || !m_pRenderer)
		{
			SAFE_DELETE(m_pRenderer);
			OUTPUT_LOG(TEXT("Could not create texture renderer object!  hr=0x%x"), hr);
			return E_FAIL;
		}
#ifdef REGISTER_FILTERGRAPH
		// Register the graph in the Running Object Table (for debug purposes)
		m_pRenderer->AddToROT(m_pGB);
#endif

		m_pRenderer1=m_pRenderer;
		if (FAILED(hr = m_pGB->AddFilter(m_pRenderer1, L"TEXTURERENDERER")))
		{
			OUTPUT_LOG(TEXT("Could not add renderer filter to graph!  hr=0x%x"), hr);
			return hr;
		}
		WCHAR *wmv_ext[] = { L".wmv", L".wma", L".asf", NULL };
		WCHAR *mpg_ext[] = { L".mpg", L"mpeg", L".m2v", NULL };
		long length = lstrlenW(wFileName)-4;


		if (!lstrcmpiW(&wFileName[length], wmv_ext[0]) ||
			!lstrcmpiW(&wFileName[length], wmv_ext[1]) ||
			!lstrcmpiW(&wFileName[length], wmv_ext[2])) 
		{
			hr = ConnectWMVFile(wFileName);
			if (FAILED(hr)) {
				return hr;
			}
		} else{
			hr = ConnectOtherFile(wFileName);
			if (FAILED(hr)) {
				return hr;
			}		
		} 

		if (!m_bAudioOn) {
			// If no audio component is desired, directly connect the two video pins
			// instead of allowing the Filter Graph Manager to render all pins.

			CComPtr<IPin> pFTRPinIn;      // Texture Renderer Input Pin

			// Find the source's output pin and the renderer's input pin
			if (FAILED(hr = m_pRenderer->FindPin(L"In", &pFTRPinIn)))
			{
				OUTPUT_LOG(TEXT("Could not find input pin!  hr=0x%x"), hr);
				return hr;
			}

			// Connect these two filters
			if (FAILED(hr = m_pGB->Connect(m_pPinOut1, pFTRPinIn)))
			{
				OUTPUT_LOG(TEXT("Could not connect pins!  hr=0x%x"), hr);
				return hr;
			}
			pFTRPinIn.Release();
		}
		else{
			// Render the source filter's output pin.  The Filter Graph Manager
			// will connect the video stream to the loaded CTextureRenderer
			// and will load and connect an audio renderer (if needed).

			if (S_OK!=(hr = m_pGB->Render(m_pPinOut1)))
			{
				OUTPUT_LOG(TEXT("Could not render source output pin!  hr=0x%x"), hr);
				return hr;
			}
			if (m_pPinOut0&&m_bAudioOn) {
				if (FAILED(hr = m_pGB->Render(m_pPinOut0))) {
					OUTPUT_LOG(TEXT("Connecting RenderPin output failed! hr=0x%x\nCheck to see if Video Decoder filters are installed."), hr);
					return hr;
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// wait until the renderer has created a texture for us. 
		if (IsNeedCreateTexture()) 
		{
			if (FAILED(hr=m_pRenderer->CreateTexture(&m_pTexture))) 
			{
				OUTPUT_LOG(TEXT("Failed creating the render texture for the video file"), hr);
				return hr;
			}
		}
		m_bLocked=true;
	}
	catch (...)
	{
		OUTPUT_LOG("exception thrown when loading video file.\r\n");
		return E_FAIL;
	}
	return hr;
}

LPDIRECT3DTEXTURE9 CGraphBuilder::GetTexture()
{
	if(m_pTexture == 0 || m_pRenderer == 0)
		return 0;
	m_pRenderer->UpdateTexture(m_pTexture);
	return m_pTexture;
}

HRESULT CGraphBuilder::ConnectWMVFile( LPCWSTR wFileName )
{
	HRESULT hr = S_OK;
	CComPtr<IBaseFilter>    pFSrc;          // Source Filter
	//hr = m_pGB->AddSourceFilter (wFileName, L"SOURCE", &pFSrc);
	hr= pFSrc.CoCreateInstance(CLSID_WMAsfReader,NULL,CLSCTX_INPROC);
	//hr = AddFilterByCLSID(g_pGB, CLSID_WMAsfReader, wFileName, &pFSrc);
	hr=m_pGB->AddFilter(pFSrc,L"SOURCE");
	if (FAILED(hr)) {
		OUTPUT_LOG(TEXT("Unable to add [WM ASF Reader]!  hr=0x%x"), hr);
		return hr;
	}

	// Get the Inteface to IFileSource so we can load the file
	hr = pFSrc->QueryInterface(IID_IFileSourceFilter, (void **)&m_pFileSource);
	if (!FAILED(hr) && (m_pFileSource != NULL)) {
		hr = m_pFileSource->Load(wFileName, NULL);

		if (FAILED(hr)) {
			OUTPUT_LOG(TEXT("Unable to load file %s!  hr=0x%x"), wFileName, hr);
			return hr;
		}
	}

	// Find the source's Raw Video #1 output pin and the video renderer's input pin
	if (FAILED(hr = pFSrc->FindPin(L"Raw Video 0", &m_pPinOut1))) {
		if (FAILED(hr = pFSrc->FindPin(L"Raw Video 1", &m_pPinOut1))) {
			OUTPUT_LOG(TEXT("ConnectWMVFile() unable to find Raw Video Pin #1!  hr=0x%x"), hr);
			return hr;
		}
	}

	// Find the source's Raw Audio #0 output pin and the audio renderer's input pin
	if (FAILED(hr = pFSrc->FindPin(L"Raw Audio 0", &m_pPinOut0))) {
		if (FAILED(hr = pFSrc->FindPin(L"Raw Audio 1", &m_pPinOut0))) {
			m_pPinOut0 = NULL;							 
			OUTPUT_LOG(TEXT("No audio track for the wmv file"), hr);
			hr = S_OK;
		}
	}
	pFSrc.Release();
	return hr;
}

HRESULT CGraphBuilder::ConnectOtherFile( LPCWSTR wFileName )
{
	HRESULT hr=S_OK;
	CComPtr<IBaseFilter>    pFSrc;          // Source Filter
	// Add the source filter to the graph.
	hr = m_pGB->AddSourceFilter (wFileName, L"SOURCE", &pFSrc);

	// If the media file was not found, inform the user.
	if (hr == VFW_E_NOT_FOUND)
	{
		OUTPUT_LOG(TEXT("Could not add source filter to graph!  (hr==VFW_E_NOT_FOUND)\r\n\r\n")
			TEXT("This sample reads a media file from your windows directory.\r\n")
			TEXT("This file is missing from this machine."));
		return hr;
	}
	else if(FAILED(hr))
	{
		OUTPUT_LOG(TEXT("Could not add source filter to graph!  hr=0x%x"), hr);
		return hr;
	}

	if (FAILED(hr = pFSrc->FindPin(L"Output", &m_pPinOut1)))
	{
		OUTPUT_LOG(TEXT("Could not find output pin!  hr=0x%x"), hr);
		return hr;
	}
	pFSrc.Release();
	return hr;
}

HRESULT CGraphBuilder::PauseGraph()
{
	if (!m_pRenderer) {
		return E_FAIL;
	}
	HRESULT hr = S_OK;

	// Get the graph's media control, event & position interfaces
	if (!m_pMC) {
		m_pGB.QueryInterface(&m_pMC);
	}

	if (m_pMC) {
		// Pause the graph ;
		if (FAILED(hr = m_pMC->Pause()))
		{
			OUTPUT_LOG(TEXT("Unable to PAUSE the DirectShow graph!  hr=0x%x"), hr);
			return hr;
		}
	}
	return S_OK;
}

HRESULT CGraphBuilder::StopGraph()
{
	if (!m_pRenderer) {
		return E_FAIL;
	}
	HRESULT hr = S_OK;

	// Get the graph's media control, event & position interfaces
	if (!m_pMC) {
		m_pGB.QueryInterface(&m_pMC);
	}
	if (!m_pMP) {
		m_pGB.QueryInterface(&m_pMP);
	}

	if (m_pMC) {
		// Stop the graph running;
		if (FAILED(hr = m_pMC->Stop()))
		{
			OUTPUT_LOG(TEXT("Unable to STOP the DirectShow graph!  hr=0x%x"), hr);
			return hr;
		}
		if (FAILED(hr = m_pMP->put_CurrentPosition(0)))
		{
			OUTPUT_LOG(TEXT("Unable to STOP the DirectShow graph!  hr=0x%x"), hr);
			return hr;
		}
	}
	return S_OK;
}

HRESULT CGraphBuilder::RunGraph()
{
	HRESULT hr = S_OK;
	if (!m_pRenderer) {
		return E_FAIL;
	}
	// Get the graph's media control, event & position interfaces
	if (!m_pMC) m_pGB.QueryInterface(&m_pMC);
	if (!m_pMP) m_pGB.QueryInterface(&m_pMP);
	if (!m_pME) m_pGB.QueryInterface(&m_pME);

	// Start the graph running;
	if (FAILED(hr = m_pMC->Run()))
	{
		OUTPUT_LOG(TEXT("Unable to RUN the DirectShow graph!  hr=0x%x"), hr);
		return hr;
	}

	return S_OK;
}

HRESULT CGraphBuilder::Seek(double seekvalue)
{
	if (!m_pRenderer) {
		return E_FAIL;
	}
	if (seekvalue<0.0001) {
		seekvalue=0.0001;
	}
	if (seekvalue>99.9999) {
		seekvalue=99.9999;
	}
	REFTIME length, newpos;

	if (!m_pMP) {
		m_pGB.QueryInterface(&m_pMP);
	}
	m_pMP->get_Duration(&length);
	newpos = seekvalue * length/100.0f;
	m_pMP->put_CurrentPosition(newpos);

	return S_OK;
}

double CGraphBuilder::GetCurrPos()
{
	if (!m_pRenderer) {
		return 0;
	}
	REFTIME currpos;

	if (!m_pMP) {
		m_pGB.QueryInterface(&m_pMP);
	}
	m_pMP->get_CurrentPosition(&currpos);

	return currpos;
}

double CGraphBuilder::GetDuration()
{
	if (!m_pRenderer) {
		return 0;
	}
	REFTIME length;

	if (!m_pMP) {
		m_pGB.QueryInterface(&m_pMP);
	}
	m_pMP->get_Duration(&length);

	return length;
}

long CGraphBuilder::CheckMovieStatus(bool bRepeat)
{
	long lEventCode=0;
	LONG_PTR lParam1, lParam2;
	HRESULT hr;

	if (m_pME==NULL)
		return 0;

	// Check for completion events
	hr = m_pME->GetEvent(&lEventCode, &lParam1, &lParam2, 0);
	if (SUCCEEDED(hr))
	{
		// If we have reached the end of the media file, reset to beginning
		if (EC_COMPLETE == lEventCode&&bRepeat)
		{
			hr = m_pMP->put_CurrentPosition(0);
		}

		// Free any memory associated with this event
		hr = m_pME->FreeEventParams(lEventCode, lParam1, lParam2);
	}
	return lEventCode;
}

void CGraphBuilder::CleanupDShow()
{
#ifdef REGISTER_FILTERGRAPH
	// Pull graph from Running Object Table (Debug)
	if ( m_pRenderer) {
		m_pRenderer->RemoveFromROT();
	}
#endif

	// Shut down the graph
	if (m_pMC) m_pMC->Stop();
	SAFE_RELEASE(m_pFileSource);
	if (m_pPinOut0) m_pPinOut0.Release();
	if (m_pPinOut1) m_pPinOut1.Release();
	if (m_pRenderer1) {m_pRenderer1.Release();}
	if (m_pMC) m_pMC.Release();
	if (m_pME) m_pME.Release();
	if (m_pMP) m_pMP.Release();
	if (m_pGB) m_pGB.Release();
	SAFE_RELEASE(m_pTexture);
	m_bLocked=false;
}

int CGraphBuilder::GetHeight()
{
	if (m_pRenderer) {
		return m_pRenderer->GetHeight();
	}
	return 0;
}

int CGraphBuilder::GetWidth()
{
	if ( m_pRenderer) {
		return m_pRenderer->GetWidth();
	}
	return 0;
}