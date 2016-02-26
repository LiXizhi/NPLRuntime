//----------------------------------------------------------------------
// Class:	CTextureRenderer
// Authors:	Li, Xizhi, Liu Weili
// Date:	2006.3.9
// Revised:	2006.10.25 by LiXizhi
// desc: 
// The Texture renderer for DirectShow. This class gets input from the DirectShow decoder and render them into a texture. 
// The texture is typically in D3DUSAGE_DYNAMIC form, if the device support this usage. 
// Some codes are from DirectShow sample. I modified the code so it is more concise and effective. 
// 
// @Note by lixizhi: Since some codec and the game runs in separate thread. We should not update the texture in the codec thread such as the DoRenderSample()
// function, unless D3DCREATE_MULTITHREADED flag is specified. However, I did not want to use D3DCREATE_MULTITHREADED flag, since there are some minor 
// performance lost. Instead I used pure heap memory and a mutex to share the texture. 
//----------------------------------------------------------------------

#include "DxStdAfx.h"
#include "ParaEngine.h"
#include "dshowtextures.h"
#include "GraphBuilder.h"
#include <amstream.h>

#include "memdebug.h"

using namespace ParaEngine;

CTextureRenderer::CTextureRenderer( LPUNKNOWN pUnk, HRESULT *phr)
: CBaseVideoRenderer(__uuidof(CLSID_TextureRenderer),	
					 NAME("Texture Renderer"), pUnk, phr),
					 m_bUseDynamicTextures(FALSE)
{
	// Store and AddRef the texture for our use.
	ASSERT(phr);
	if (phr)
		*phr = S_OK;
	m_bUseDynamicTextures=false;
	m_TextureFormat = D3DFMT_X8R8G8B8;
	m_VideoBpp = 24;
	m_bFrameChanged = true;
	m_hInitDone    = CreateEvent(NULL, TRUE, FALSE, _T("TextureRenderInit"));
}

CTextureRenderer::~CTextureRenderer()
{
	// Do nothing
	m_textureData.clear();
}

HRESULT CTextureRenderer::CreateTexture(LPDIRECT3DTEXTURE9 *ppTexture)
{
	// Create the texture that maps to this media type
	HRESULT hr = E_UNEXPECTED;
	UINT uintWidth = 2;
	UINT uintHeight = 2;
	
	if (WAIT_TIMEOUT==WaitForSingleObject(m_hInitDone,100)) 
	{
		return hr;
	}
	// here let's check if we can use dynamic textures
	D3DCAPS9 caps;
	ZeroMemory( &caps, sizeof(D3DCAPS9));
	LPDIRECT3DDEVICE9 pd3dDevice=CGlobals::GetRenderDevice();
	hr = pd3dDevice->GetDeviceCaps( &caps );
	if( caps.Caps2 & D3DCAPS2_DYNAMICTEXTURES )
	{
		m_bUseDynamicTextures = true;
	}

	if( caps.TextureCaps & D3DPTEXTURECAPS_POW2 )
	{
		while( (LONG)uintWidth < m_lVidWidth )
		{
			uintWidth = uintWidth << 1;
		}
		while( (LONG)uintHeight < m_lVidHeight )
		{
			uintHeight = uintHeight << 1;
		}
	}
	else
	{
		uintWidth = m_lVidWidth;
		uintHeight = m_lVidHeight;
	}	
	
	if( m_bUseDynamicTextures )
	{
		hr = pd3dDevice->CreateTexture(uintWidth, uintHeight, 1, D3DUSAGE_DYNAMIC,
			D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT,
			ppTexture, NULL);
		if( FAILED(hr))
		{
			m_bUseDynamicTextures = FALSE;
		}
	}
	else
	{
		hr = pd3dDevice->CreateTexture(uintWidth, uintHeight, 1, 0,
			D3DFMT_X8R8G8B8,D3DPOOL_MANAGED,
			ppTexture, NULL);
	}

	if( FAILED(hr))
	{
		OUTPUT_LOG(TEXT("Could not create the D3DX texture!  hr=0x%x"), hr);
		return hr;
	}

	// CreateTexture can silently change the parameters on us
	D3DSURFACE_DESC ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));

	if ( FAILED( hr = (*ppTexture)->GetLevelDesc( 0, &ddsd ) ) ) {
		OUTPUT_LOG(TEXT("Could not get level Description of D3DX texture! hr = 0x%x"), hr);
		return hr;
	}


	CComPtr<IDirect3DSurface9> pSurf;

	if (SUCCEEDED(hr = (*ppTexture)->GetSurfaceLevel(0, &pSurf)))
		pSurf->GetDesc(&ddsd);

	// Save format info
	m_TextureFormat = ddsd.Format;

	if (m_TextureFormat != D3DFMT_X8R8G8B8) {
		OUTPUT_LOG(TEXT("Texture is format we can't handle! Format = 0x%x"), m_TextureFormat);
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
	pSurf.Release();
	return S_OK;
}
HRESULT CTextureRenderer::CheckMediaType(const CMediaType *pmt)
{
	HRESULT   hr = E_FAIL;

	CheckPointer(pmt,E_POINTER);

	// Reject the connection if this is not a video type
	if( *pmt->FormatType() != FORMAT_VideoInfo ) {
		return E_INVALIDARG;
	}

	// Only accept RGB24 video

	if(IsEqualGUID( *pmt->Type(),    MEDIATYPE_Video)  &&
		IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_RGB24))
	{
		m_TextureFormat = D3DFMT_X8R8G8B8;
		m_VideoBpp = 24;
		hr = S_OK;
	}

	return hr;
}

HRESULT CTextureRenderer::GetDDMediaSample( IMediaSample * pSample )
{
	IDirectDrawMediaSample * pDDMediaSample;
	HRESULT hr = S_OK;

	LONG output_pitch;

	hr = pSample->QueryInterface(IID_IDirectDrawMediaSample, (void **)&pDDMediaSample);
	if(SUCCEEDED(hr))
	{
		LPDIRECTDRAWSURFACE pDDSurface;
		RECT rc;

		hr = pDDMediaSample->GetSurfaceAndReleaseLock(&pDDSurface, &rc);
		if(SUCCEEDED(hr))
		{
			DDSURFACEDESC DDSurfDesc;
			DDSurfDesc.dwSize = sizeof(DDSURFACEDESC);
			hr = pDDSurface->GetSurfaceDesc(&DDSurfDesc);
			output_pitch = DDSurfDesc.lPitch / 2;
			pDDSurface->Release();
		}
		pDDMediaSample->Release();
	}
	return hr;
}


HRESULT CTextureRenderer::GetConnectedMediaType(D3DFORMAT *uiTexFmt, LONG *uiVidBpp)
{
	HRESULT hr = S_OK;

	*uiTexFmt = m_TextureFormat;
	*uiVidBpp = m_VideoBpp;

	return hr;
}


HRESULT CTextureRenderer::SetMediaType(const CMediaType *pmt)
{
	// Retrive the size of this media type
	VIDEOINFO *pviBmp;                      // Bitmap info header
	pviBmp = (VIDEOINFO *)pmt->Format();
	memcpy((void*)&m_viBmp,(void*)pviBmp,sizeof(VIDEOINFO));

	m_lVidWidth  = pviBmp->bmiHeader.biWidth;
	m_lVidHeight = abs(pviBmp->bmiHeader.biHeight);
	m_lVidPitch  = (m_lVidWidth * 3 + 3) & ~(3); // We are forcing RGB24
	m_textureData.resize(m_lVidPitch*m_lVidHeight, 0);
	m_bUpSideDown=pviBmp->bmiHeader.biHeight>0?true:false;
	SetEvent(m_hInitDone);
	return S_OK;
}

HRESULT CTextureRenderer::lock(byte** data, int* nSize)
{
	mMutex.m_lock();
	*data = &(m_textureData[0]);
	*nSize = (int)m_textureData.size();
	return S_OK;
}

HRESULT CTextureRenderer::unlock()
{
	mMutex.m_unlock();
	return S_OK;
}

HRESULT CTextureRenderer::UpdateTexture(LPDIRECT3DTEXTURE9 pTexture)
{
	if(pTexture==0)
		return E_FAIL;

	::Lock l(&mMutex);
	if(m_bFrameChanged == false)
		return S_OK;
	m_bFrameChanged = false;

	if(m_TextureFormat != D3DFMT_X8R8G8B8)
		return E_FAIL;

	BYTE  *pBmpBuffer, *pTxtBuffer; // Bitmap buffer, texture buffer
	LONG  lTxtPitch;                // Pitch of bitmap, texture

	// Get the video bitmap buffer
	if((int)m_textureData.size() < m_lVidPitch*m_lVidHeight)
		m_textureData.resize(m_lVidPitch*m_lVidHeight, 0);
	pBmpBuffer = &(m_textureData[0]);

	BYTE  * pbS = NULL;
	DWORD * pdwS = NULL;
	DWORD * pdwD = NULL;
	int row, col, dwordWidth;

	// Lock the Texture
	D3DLOCKED_RECT d3dlr;
	if( m_bUseDynamicTextures )
	{
		if( FAILED(pTexture->LockRect(0, &d3dlr, 0, D3DLOCK_DISCARD)))
			return E_FAIL;
	}
	else
	{
		if (FAILED(pTexture->LockRect(0, &d3dlr, 0, 0)))
			return E_FAIL;
	}
	// Get the texture buffer & pitch
	pTxtBuffer = static_cast<byte *>(d3dlr.pBits);
	lTxtPitch = d3dlr.Pitch;


	// Copy the bits

	//if (m_TextureFormat == D3DFMT_X8R8G8B8)
	{
		// Instead of copying data bytewise, we use DWORD alignment here.
		// We also unroll loop by copying 4 pixels at once.
		//
		// original BYTE array is [b0][g0][r0][b1][g1][r1][b2][g2][r2][b3][g3][r3]
		//
		// aligned DWORD array is     [b1 r0 g0 b0][g2 b2 r1 g1][r3 g3 b3 r2]
		//
		// We want to transform it to [ff r0 g0 b0][ff r1 g1 b1][ff r2 g2 b2][ff r3 b3 g3]
		// below, bitwise operations do exactly this.

		dwordWidth = m_lVidWidth / 4; // aligned width of the row, in DWORDS
		// (pixel by 3 bytes over sizeof(DWORD))
		if (m_bUpSideDown) {
			for( row = m_lVidHeight-1; row>=0; row--)
			{
				pdwS = ( DWORD*)(pBmpBuffer+row*m_lVidPitch);
				pdwD = ( DWORD*)pTxtBuffer;

				for( col = 0; col < dwordWidth; col ++ )
				{
					pdwD[0] =  pdwS[0] | 0xFF000000;
					pdwD[1] = ((pdwS[1]<<8)  | 0xFF000000) | (pdwS[0]>>24);
					pdwD[2] = ((pdwS[2]<<16) | 0xFF000000) | (pdwS[1]>>16);
					pdwD[3] = 0xFF000000 | (pdwS[2]>>8);
					pdwD +=4;
					pdwS +=3;
				}

				// we might have remaining (misaligned) bytes here
				pbS = (BYTE*) pdwS;
				for( col = 0; col < m_lVidWidth % 4; col++)
				{
					*pdwD = 0xFF000000     |
						(pbS[2] << 16) |
						(pbS[1] <<  8) |
						(pbS[0]);
					pdwD++;
					pbS += 3;
				}

				pTxtBuffer += lTxtPitch;
			}// for rows
		}else{
			for( row = 0; row<m_lVidHeight; row++)
			{
				pdwS = ( DWORD*)(pBmpBuffer);
				pdwD = ( DWORD*)pTxtBuffer;

				for( col = 0; col < dwordWidth; col ++ )
				{
					pdwD[0] =  pdwS[0] | 0xFF000000;
					pdwD[1] = ((pdwS[1]<<8)  | 0xFF000000) | (pdwS[0]>>24);
					pdwD[2] = ((pdwS[2]<<16) | 0xFF000000) | (pdwS[1]>>16);
					pdwD[3] = 0xFF000000 | (pdwS[2]>>8);
					pdwD +=4;
					pdwS +=3;
				}

				// we might have remaining (misaligned) bytes here
				pbS = (BYTE*) pdwS;
				for( col = 0; col < m_lVidWidth % 4; col++)
				{
					*pdwD = 0xFF000000     |
						(pbS[2] << 16) |
						(pbS[1] <<  8) |
						(pbS[0]);
					pdwD++;
					pbS += 3;
				}
				pBmpBuffer+=m_lVidPitch;
				pTxtBuffer += lTxtPitch;
			}// for rows

		}
	}

	// Unlock the Texture
	if (FAILED(pTexture->UnlockRect(0)))
		return E_FAIL;
	return S_OK;
}

/// this function is called in the codec thread. 
HRESULT CTextureRenderer::DoRenderSample( IMediaSample * pSample )
{
	if(m_TextureFormat != D3DFMT_X8R8G8B8)
		return E_FAIL;
	
	::Lock l(&mMutex);

	CheckPointer(pSample,E_POINTER);
	// Get the video bitmap buffer
	BYTE * pBmpBuffer = NULL;
	if( FAILED(pSample->GetPointer( &pBmpBuffer )) )
		return E_FAIL;

	// Get the video bitmap buffer
	if((int)m_textureData.size() < m_lVidPitch*m_lVidHeight)
		m_textureData.resize(m_lVidPitch*m_lVidHeight, 0);
	
	memcpy(&(m_textureData[0]), pBmpBuffer, m_lVidPitch*m_lVidHeight);

	m_bFrameChanged = true;
	return S_OK;
}

int CTextureRenderer::GetHeight()
{
	if (WAIT_OBJECT_0==WaitForSingleObject(m_hInitDone,100)) {
		return m_viBmp.bmiHeader.biHeight;
	}
	return 0;
}

int CTextureRenderer::GetWidth()
{
	if (WAIT_OBJECT_0==WaitForSingleObject(m_hInitDone,100)) {
		return m_viBmp.bmiHeader.biWidth;
	}
	return 0;
}
#ifdef REGISTER_FILTERGRAPH

//-----------------------------------------------------------------------------
// Running Object Table functions: Used to debug. By registering the graph
// in the running object table, GraphEdit is able to connect to the running
// graph. This code should be removed before the application is shipped in
// order to avoid third parties from spying on your graph.
//-----------------------------------------------------------------------------
DWORD dwROTReg = 0xfedcba98;
static bool bROTInitialized=false;

HRESULT CTextureRenderer::AddToROT(IUnknown *pUnkGraph)
{
	if (bROTInitialized) {
		return S_OK;
	}
	bROTInitialized=true;
	IMoniker * pmk;
	IRunningObjectTable *pROT;
	if (FAILED(GetRunningObjectTable(0, &pROT))) {
		return E_FAIL;
	}
	WCHAR wsz[256];
	(void)StringCchPrintfW(wsz, NUMELMS(wsz),L"FilterGraph %08x  pid %08x\0", (DWORD_PTR)0, GetCurrentProcessId());

	HRESULT hr = CreateItemMoniker(L"!", wsz, &pmk);
	if (SUCCEEDED(hr))
	{
		// Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference
		// to the object.  Using this flag will cause the object to remain
		// registered until it is explicitly revoked with the Revoke() method.
		//
		// Not using this flag means that if GraphEdit remotely connects
		// to this graph and then GraphEdit exits, this object registration
		// will be deleted, causing future attempts by GraphEdit to fail until
		// this application is restarted or until the graph is registered again.
		hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph,
			pmk, &dwROTReg);
		pmk->Release();
	}

	pROT->Release();
	return hr;
}


void CTextureRenderer::RemoveFromROT(void)
{
	bROTInitialized=false;
	IRunningObjectTable *pirot=0;

	if (SUCCEEDED(GetRunningObjectTable(0, &pirot)))
	{
		pirot->Revoke(dwROTReg);
		pirot->Release();
	}
}

#endif

