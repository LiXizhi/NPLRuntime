//-----------------------------------------------------------------------------
// Class: RenderDevice
// Authors:	LiXizhi
// Emails:	lixizhi@yeah.net
// Date: 2014.9.11
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_DIRECTX_RENDERER
#include "DirectXEngine.h"
#include "RenderDeviceDirectX.h"

using namespace ParaEngine;

RenderDevice* RenderDevice::GetInstance()
{
	static RenderDevice g_instance;
	return &g_instance;
}

HRESULT RenderDevice::DrawPrimitive(RenderDevicePtr pd3dDevice, int nStatisticsType, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	IncrementDrawBatchAndVertices(1, PrimitiveCount, nStatisticsType);
	return pd3dDevice->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT RenderDevice::DrawPrimitiveUP(RenderDevicePtr pd3dDevice, int nStatisticsType, D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	IncrementDrawBatchAndVertices(1, PrimitiveCount, nStatisticsType);
	return pd3dDevice->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT RenderDevice::DrawIndexedPrimitive(RenderDevicePtr pd3dDevice, int nStatisticsType, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinIndex, UINT NumVertices, UINT indexStart, UINT PrimitiveCount)
{
	IncrementDrawBatchAndVertices(1, PrimitiveCount, nStatisticsType);
	return pd3dDevice->DrawIndexedPrimitive(Type, BaseVertexIndex, MinIndex, NumVertices, indexStart, PrimitiveCount);
}

HRESULT RenderDevice::DrawIndexedPrimitiveUP(RenderDevicePtr pd3dDevice, int nStatisticsType, D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void * pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	IncrementDrawBatchAndVertices(1, PrimitiveCount, nStatisticsType);
	return pd3dDevice->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

bool RenderDevice::CheckRenderError(const char* filename, const char* func, int nLine)
{
	return true;
}

bool ParaEngine::RenderDevice::ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, DWORD nDataFormat /*= 0*/, DWORD nDataType /*= 0*/)
{
	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	// gets the surface of the back buffer.
	IDirect3DSurface9* pFrameBufferSurface = CGlobals::GetDirectXEngine().GetRenderTarget();
	/*HRESULT hResult = pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pFrameBufferSurface);
	if (FAILED(hResult))
	return false;
	bool bIsBackbuffer = CGlobals::GetDirectXEngine().GetRenderTarget() == pFrameBufferSurface;
	SAFE_RELEASE(pFrameBufferSurface);*/

	// we do not support reading from back buffer directly now. Since it requires D3DPRESENTFLAG_LOCKABLE_BACKBUFFER which may kill some performance. 
	bool bIsBackbuffer = false;
	if (!bIsBackbuffer)
	{
		// get pixels from the render target.
		// 1. copy current render target to another render target of picking size
		// 2. create a off screen memory surface of picking size and GetRenderTargetData to it. 
		// 3. read pixels from the off screen memory surface.
		bool bRes = false;
		D3DFORMAT colorFormat = D3DFMT_A8R8G8B8;
		LPDIRECT3DTEXTURE9 pTextureDest = NULL;
		LPDIRECT3DSURFACE9 pSurDest = NULL;
		RECT srcRect = {nLeft, nTop, nLeft+nWidth, nTop+nHeight};
		RECT destRect = { 0, 0, nWidth, nHeight };
		if (FAILED(pd3dDevice->CreateTexture(nWidth, nHeight, 1, D3DUSAGE_RENDERTARGET, colorFormat, D3DPOOL_DEFAULT, &pTextureDest, NULL)))
		{
			return false;
		}
		if (FAILED(pTextureDest->GetSurfaceLevel(0, &pSurDest)))
		{
			SAFE_RELEASE(pTextureDest);
			return false;
		}

		// Copy scene to render target texture
		if (SUCCEEDED(pd3dDevice->StretchRect(pFrameBufferSurface, &srcRect, pSurDest, &destRect, D3DTEXF_NONE)))
		{
			LPDIRECT3DSURFACE9 pSurDestInMem = NULL;
			if (SUCCEEDED(pd3dDevice->CreateOffscreenPlainSurface(nWidth, nHeight, colorFormat, D3DPOOL_SYSTEMMEM, &pSurDestInMem, NULL)))
			{
				if (SUCCEEDED(pd3dDevice->GetRenderTargetData(pSurDest, pSurDestInMem)))
				{
					// Read the pixels 
					D3DLOCKED_RECT bits;
					RECT rect = { 0, 0, nWidth, nHeight };
					int nIndex = 0;
					if (SUCCEEDED(pSurDestInMem->LockRect(&bits, &rect, D3DLOCK_READONLY)))
					{
						DWORD* pickingResult = (DWORD*)pDataOut;
						for (int y = 0; y < nHeight; ++y)
						{
							DWORD* pBits = (DWORD*)(((BYTE*)bits.pBits) + (y*bits.Pitch));
							// pBits points to an array of ints, one for each pixel
							for (int x = 0; x < nWidth; ++x)
							{
								DWORD pickingId = (*pBits++);
								pickingResult[nIndex++] = pickingId;
								// OUTPUT_LOG("picking result: %d %d: %d\n", x + region.x(), y + region.y(), pickingId);
							}
						}
						bRes = true;
						pSurDestInMem->UnlockRect();
					}
				}
				SAFE_RELEASE(pSurDestInMem);
			}
		}
		SAFE_RELEASE(pSurDest);
		SAFE_RELEASE(pTextureDest);
		return bRes;
	}
	else
	{
		// Read the pixels directly from backbuffer
		D3DLOCKED_RECT bits;
		RECT rect = { nLeft, nTop, nLeft + nWidth, nTop + nHeight };
		int nIndex = 0;
		if (SUCCEEDED(pFrameBufferSurface->LockRect(&bits, &rect, D3DLOCK_READONLY)))
		{
			DWORD* pickingResult = (DWORD*)pDataOut;
			for (int y = 0; y < nHeight; ++y)
			{
				DWORD* pBits = (DWORD*)(((BYTE*)bits.pBits) + (y*bits.Pitch));
				// pBits points to an array of ints, one for each pixel
				for (int x = 0; x < nWidth; ++x)
				{
					DWORD pickingId = (*pBits++);
					pickingResult[nIndex++] = pickingId;
					// OUTPUT_LOG("picking result: %d %d: %d\n", x + region.x(), y + region.y(), pickingId);
				}
			}
			pFrameBufferSurface->UnlockRect();
		}
		return nIndex != 0;
	}
}

#endif