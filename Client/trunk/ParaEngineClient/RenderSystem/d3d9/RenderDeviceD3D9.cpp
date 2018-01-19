
#include "RenderDeviceD3D9.h"
#include "D3D9Tools.h"

using namespace ParaEngine;


ParaEngine::RenderDeviceD3D9::RenderDeviceD3D9(IDirect3DDevice9* device)
{
	m_pD3DDevice = device;
}


bool ParaEngine::RenderDeviceD3D9::SetClipPlane(uint32_t Index, const float* pPlane)
{
	return m_pD3DDevice->SetClipPlane(Index, pPlane) == S_OK;
}

//HRESULT ParaEngine::RenderDeviceD3D9::SetTexture(DWORD Stage, IDirect3DTexture9* pTexture)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->SetTexture(Stage, pTexture);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
//{
//	return m_pD3DDevice->SetSamplerState(Sampler, Type, Value);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
//{
//	return m_pD3DDevice->GetSamplerState(Sampler, Type, pValue);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::SetIndices(IDirect3DIndexBuffer9* pIndexData)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->SetIndices(pIndexData);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::SetTransform(D3DTRANSFORMSTATETYPE State, DeviceMatrix_ptr pMatrix)
//{
//	return m_pD3DDevice->SetTransform(State, pMatrix);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::SetFVF(DWORD FVF)
//{
//	return m_pD3DDevice->SetFVF(FVF);
//}
//
//void ParaEngine::RenderDeviceD3D9::SetCursorPosition(int X, int Y, DWORD Flags)
//{
//	return m_pD3DDevice->SetCursorPosition(X, Y, Flags);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->SetVertexDeclaration(pDecl);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
//{
//	return m_pD3DDevice->CreateVertexDeclaration(pVertexElements, ppDecl);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::BeginScene()
//{
//	return m_pD3DDevice->BeginScene();
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::EndScene()
//{
//	return m_pD3DDevice->EndScene();
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::Clear(DWORD Count, const D3DRECT* pRects, DWORD Flags, DWORD Color, float Z, DWORD Stencil)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->Clear(Count, pRects, Flags, Color, Z, Stencil);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::SetViewport(const D3DVIEWPORT9* pViewport)
//{
//	return m_pD3DDevice->SetViewport(pViewport);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::GetViewport(D3DVIEWPORT9* pViewport)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->GetViewport(pViewport);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::SetScissorRect(RECT* pRect)
//{
//	return m_pD3DDevice->SetScissorRect(pRect);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::GetScissorRect(RECT* pRect)
//{
//	return m_pD3DDevice->GetScissorRect(pRect);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::SetVertexShader(IDirect3DVertexShader9* pShader)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->SetVertexShader(pShader);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::SetPixelShader(IDirect3DPixelShader9* pShader)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->SetPixelShader(pShader);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::DrawPrimitive(int nStatisticsType, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->DrawPrimitive(PrimitiveType, StartVertex,PrimitiveCount);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::DrawPrimitiveUP(int nStatisticsType, D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData,VertexStreamZeroStride);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::DrawIndexedPrimitive(int nStatisticsType, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinIndex, UINT NumVertices, UINT indexStart, UINT PrimitiveCount)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->DrawIndexedPrimitive(Type, BaseVertexIndex, MinIndex, NumVertices, indexStart, PrimitiveCount);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::DrawIndexedPrimitiveUP(int nStatisticsType, D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void * pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
//}
//
//int ParaEngine::RenderDeviceD3D9::GetMaxSimultaneousTextures()
//{
//	throw std::logic_error("The method or operation is not implemented.");
//}
//
//
//HRESULT ParaEngine::RenderDeviceD3D9::SetMaterial(D3DMATERIAL9* pMaterial)
//{
//	return m_pD3DDevice->SetMaterial(pMaterial);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
//{
//	return m_pD3DDevice->SetRenderTarget(RenderTargetIndex, pRenderTarget);
//}
//
//bool ParaEngine::RenderDeviceD3D9::ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, DWORD nDataFormat, DWORD nDataType)
//{
//	throw std::logic_error("The method or operation is not implemented.");
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB)
//{
//	return m_pD3DDevice->CreateStateBlock(Type, ppSB);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->SetDepthStencilSurface(pNewZStencil);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
//{
//	return m_pD3DDevice->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->GetDepthStencilSurface(ppZStencilSurface);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::CreateTextureFromFileInMemoryEx(LPCVOID pSrcData, UINT SrcDataSize, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO* pSrcInfo, PALETTEENTRY*pPalette, LPDIRECT3DTEXTURE9*ppTexture)
//{
//	return D3DXCreateTextureFromFileInMemoryEx(m_pD3DDevice, pSrcData, SrcDataSize, Width, Height, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::CreateTextureFromFileEx(LPCSTR pSrcFile, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO* pSrcInfo, PALETTEENTRY* pPalette, LPDIRECT3DTEXTURE9* ppTexture)
//{
//	return D3DXCreateTextureFromFileEx(m_pD3DDevice, pSrcFile, Width, Height, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::CheckTextureRequirements(UINT* pWidth, UINT* pHeight, UINT* pNumMipLevels, DWORD Usage, D3DFORMAT* pFormat, D3DPOOL Pool)
//{
//	return D3DXCheckTextureRequirements(m_pD3DDevice, pWidth, pHeight, pNumMipLevels, Usage, pFormat, Pool);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::CreateCubeTextureFromFileInMemoryEx(LPCVOID pSrcData, UINT SrcDataSize, UINT Size, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO* pSrcInfo, PALETTEENTRY* pPalette, LPDIRECT3DCUBETEXTURE9* ppCubeTexture)
//{
//	return D3DXCreateCubeTextureFromFileInMemoryEx(m_pD3DDevice, pSrcData, SrcDataSize, Size, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppCubeTexture);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::LightEnable(DWORD Index, BOOL Enable)
//{
//	return m_pD3DDevice->LightEnable(Index, Enable);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::CreateEffect(LPCVOID pSrcData, UINT SrcDataLen, CONST D3DXMACRO* pDefines, LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXEFFECTPOOL pPool, LPD3DXEFFECT* ppEffect, LPD3DXBUFFER* ppCompilationErrors)
//{
//	return D3DXCreateEffect(m_pD3DDevice, pSrcData, SrcDataLen, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::CreateFont(INT Height, UINT Width, UINT Weight, UINT MipLevels, BOOL Italic, DWORD CharSet, DWORD OutputPrecision, DWORD Quality, DWORD PitchAndFamily, LPCSTR pFaceName, LPD3DXFONT* ppFont)
//{
//	return D3DXCreateFont(m_pD3DDevice,Height,Width,Weight,MipLevels,Italic,CharSet,OutputPrecision,Quality,PitchAndFamily,pFaceName,ppFont);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap)
//{
//	return m_pD3DDevice->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
//{
//	return m_pD3DDevice->CreateQuery(Type, ppQuery);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::CreateMeshFVF(DWORD NumFaces, DWORD NumVertices, DWORD Options, DWORD FVF, LPD3DXMESH* ppMesh)
//{
//	return D3DXCreateMeshFVF(NumFaces, NumVertices, Options, FVF,m_pD3DDevice, ppMesh);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::CreateSprite(LPD3DXSPRITE* ppSprite)
//{
//	return D3DXCreateSprite(m_pD3DDevice, ppSprite);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::LoadMeshFromXInMemory(LPCVOID Memory, DWORD SizeOfMemory, DWORD Options, LPD3DXBUFFER *ppAdjacency, LPD3DXBUFFER *ppMaterials, LPD3DXBUFFER *ppEffectInstances, DWORD *pNumMaterials, LPD3DXMESH *ppMesh)
//{
//	return D3DXLoadMeshFromXInMemory(Memory, SizeOfMemory, Options, m_pD3DDevice, ppAdjacency, ppMaterials, ppEffectInstances, pNumMaterials, ppMesh);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::LoadMeshFromXof(LPD3DXFILEDATA pxofMesh, DWORD Options, LPD3DXBUFFER *ppAdjacency, LPD3DXBUFFER *ppMaterials, LPD3DXBUFFER *ppEffectInstances, DWORD *pNumMaterials, LPD3DXMESH *ppMesh)
//{
//	return D3DXLoadMeshFromXof(pxofMesh, Options, m_pD3DDevice, ppAdjacency, ppMaterials, ppEffectInstances, pNumMaterials, ppMesh);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
//{
//	return m_pD3DDevice->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix)
//{
//	return m_pD3DDevice->GetTransform(State, pMatrix);
//}
//
//
//
//HRESULT ParaEngine::RenderDeviceD3D9::SetStreamSourceFreq(UINT StreamNumber, UINT Setting)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->SetStreamSourceFreq(StreamNumber, Setting);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::SetLight(DWORD Index, CONST D3DLIGHT9* pLight)
//{
//	return m_pD3DDevice->SetLight(Index, pLight);
//}
//
//
//HRESULT ParaEngine::RenderDeviceD3D9::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
//{
//	return m_pD3DDevice->GetRenderTargetData(pRenderTarget, pDestSurface);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::ShowCursor(BOOL bShow)
//{
//	return m_pD3DDevice->ShowCursor(bShow);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::Present(RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
//{
//	return m_pD3DDevice->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::TestCooperativeLevel()
//{
//	return m_pD3DDevice->TestCooperativeLevel();
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::CreateTexture(UINT Width, UINT Height, UINT MipLeves, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, LPDIRECT3DTEXTURE9* ppTexture)
//{
//	return D3DXCreateTexture(m_pD3DDevice, Width, Height, MipLeves, Usage, Format, Pool, ppTexture);
//}
//
//HRESULT ParaEngine::RenderDeviceD3D9::DrawIndexedPrimitive(D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
//{
//	return m_GETD3D(CGlobals::GetRenderDevice())->DrawIndexedPrimitive(Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
//}

LPDIRECT3DDEVICE9 RenderDeviceD3D9::GetDirect3DDevice9() const
{
	return m_pD3DDevice;
}

uint32_t ParaEngine::RenderDeviceD3D9::GetRenderState(const ERenderState& State)
{
	auto d3dRenderState = toD3DRenderState(State);
	DWORD state = 0;
	if (m_pD3DDevice->GetRenderState(d3dRenderState, &state) == S_OK)
	{
		return (uint32_t)state;
	}
	return 0;
}

bool ParaEngine::RenderDeviceD3D9::SetRenderState(const ERenderState State, const uint32_t Value)
{
	auto rs = toD3DRenderState(State);
	HRESULT hr = m_pD3DDevice->SetRenderState(rs, toD3DRenderStateValue(State,Value));
	return hr == D3D_OK;
}
