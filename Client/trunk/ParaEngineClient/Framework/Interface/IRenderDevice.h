#pragma once
#include "VertexDeclaration.h"
#include "RenderCore.h"
#include "d3d9.h"
#include "PEtypes.h"
#include "RenderDevice.h"

namespace ParaEngine
{
	class IRenderDevice
	{
	public:
		IRenderDevice() = default;
		virtual ~IRenderDevice() = default;

		virtual HRESULT RendererRecreated() = 0;

		virtual HRESULT GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) = 0;
		virtual HRESULT SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) = 0;
		virtual HRESULT SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) = 0;
		virtual HRESULT SetClipPlane(DWORD Index, const float* pPlane) = 0;
		virtual HRESULT SetTexture(DWORD Stage, IDirect3DTexture9* pTexture) = 0;
		virtual HRESULT SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) = 0;
		virtual HRESULT GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) = 0;
		virtual HRESULT SetIndices(IDirect3DIndexBuffer9* pIndexData) = 0;
		virtual HRESULT SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) = 0;
		virtual HRESULT SetTransform(D3DTRANSFORMSTATETYPE State, DeviceMatrix_ptr pMatrix) = 0;
		virtual HRESULT SetFVF(DWORD FVF) = 0;
		virtual void SetCursorPosition(int X, int Y, DWORD Flags) = 0;
		virtual HRESULT SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) = 0;
		virtual HRESULT CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) = 0;
		virtual HRESULT BeginScene() = 0;
		virtual HRESULT EndScene() = 0;
		virtual HRESULT Clear(DWORD Count, const D3DRECT* pRects, DWORD Flags, DWORD Color, float Z, DWORD Stencil) = 0;

		virtual HRESULT SetViewport(const D3DVIEWPORT9* pViewport) = 0;
		virtual HRESULT GetViewport(D3DVIEWPORT9* pViewport) = 0;

		virtual HRESULT SetScissorRect(RECT* pRect) = 0;
		virtual HRESULT GetScissorRect(RECT* pRect) = 0;

		virtual HRESULT SetVertexShader(IDirect3DVertexShader9* pShader) = 0;
		virtual HRESULT SetPixelShader(IDirect3DPixelShader9* pShader) = 0;

		virtual HRESULT CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) = 0;
		virtual HRESULT CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) = 0;

		virtual HRESULT DrawPrimitive(int nStatisticsType, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) = 0;

		virtual HRESULT DrawPrimitiveUP(int nStatisticsType,
			D3DPRIMITIVETYPE PrimitiveType,
			UINT PrimitiveCount,
			CONST void* pVertexStreamZeroData,
			UINT VertexStreamZeroStride) = 0;


		virtual HRESULT DrawIndexedPrimitive(int nStatisticsType, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinIndex, UINT NumVertices, UINT indexStart, UINT PrimitiveCount) = 0;
		virtual HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) = 0;
		virtual HRESULT DrawIndexedPrimitiveUP(int nStatisticsType,
			D3DPRIMITIVETYPE PrimitiveType,
			UINT MinVertexIndex,
			UINT NumVertices,
			UINT PrimitiveCount,
			CONST void * pIndexData,
			D3DFORMAT IndexDataFormat,
			CONST void* pVertexStreamZeroData,
			UINT VertexStreamZeroStride) = 0;

		virtual int GetMaxSimultaneousTextures() = 0;
		virtual HRESULT SetMaterial(D3DMATERIAL9* pMaterial) = 0;
		virtual HRESULT SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) = 0;
		virtual bool ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, DWORD nDataFormat = 0, DWORD nDataType = 0) = 0;

		virtual HRESULT CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) = 0;

		virtual HRESULT SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) = 0;

		virtual HRESULT CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) = 0;

		virtual HRESULT GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) = 0;

		virtual HRESULT CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) = 0;

		virtual HRESULT CreateDepthStencilSurface(UINT Width,UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) = 0;

		virtual HRESULT StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) = 0;

		virtual HRESULT GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) = 0;

		virtual int GetPerfCount(RenderDeviceBase::StatisticsType nStatisticsType) = 0;

		virtual HRESULT SetStreamSourceFreq(UINT StreamNumber, UINT Setting) = 0;
		virtual HRESULT SetLight(DWORD Index, CONST D3DLIGHT9* pLight) = 0;

		virtual void ClearAllPerfCount() = 0;

		virtual HRESULT GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) = 0;

		virtual HRESULT SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) = 0;

		virtual HRESULT ShowCursor(BOOL bShow) = 0;

		virtual HRESULT Present(RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) = 0;

	};
}

