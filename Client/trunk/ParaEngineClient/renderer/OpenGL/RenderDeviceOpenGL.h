#pragma once
#include "RenderDevice.h"

namespace ParaEngine
{
	/** a very thin wrapper to DirectX device with a portable version of openGL implementation. 
	* functions include drawing primitives, binding textures, setting blending operations, etc. 
	* This wrapper is made as thin as possible, so that one can use native DirectX or openGL api with this class. 
	*/
	class RenderDevice : public RenderDeviceBase
	{
	public:
		RenderDevice();
		static RenderDevice* GetInstance();

		static HRESULT DrawPrimitive(RenderDevicePtr pd3dDevice, int nStatisticsType, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);

		static HRESULT DrawPrimitiveUP(RenderDevicePtr pd3dDevice, int nStatisticsType,
			D3DPRIMITIVETYPE PrimitiveType,
			UINT PrimitiveCount,
			const void* pVertexStreamZeroData,
			UINT VertexStreamZeroStride);


		static HRESULT DrawIndexedPrimitive(RenderDevicePtr pd3dDevice, int nStatisticsType, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinIndex, UINT NumVertices, UINT indexStart, UINT PrimitiveCount);

		static HRESULT DrawIndexedPrimitiveUP(RenderDevicePtr pd3dDevice, int nStatisticsType,
			D3DPRIMITIVETYPE PrimitiveType,
			UINT MinVertexIndex,
			UINT NumVertices,
			UINT PrimitiveCount,
			const void * pIndexData,
			D3DFORMAT IndexDataFormat,
			const void* pVertexStreamZeroData,
			UINT VertexStreamZeroStride);
	
		/** check render error and print to log. only call this in debug mode, since it breaks parallelism between gpu and cpu. */
		static bool CheckRenderError(const char* filename = NULL, const char* func = NULL, int nLine = 0);

		/** read a block of pixels from the frame buffer
		this emulate: glReadPixels(). */
		static bool ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, DWORD nDataFormat = 0, DWORD nDataType = 0);

	public:
		HRESULT RendererRecreated();

		HRESULT SetRenderState(D3DRENDERSTATETYPE State, DWORD Value);
		HRESULT SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
		HRESULT SetClipPlane(DWORD Index, const float* pPlane);
		HRESULT SetTexture(DWORD Stage, DeviceTexturePtr_type pTexture);
		HRESULT SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
		HRESULT GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue);
		HRESULT SetIndices(IndexBufferDevicePtr_type pIndexData);
		HRESULT SetStreamSource(UINT StreamNumber, VertexBufferDevicePtr_type pStreamData, UINT OffsetInBytes, UINT Stride);
		HRESULT SetTransform(D3DTRANSFORMSTATETYPE State, DeviceMatrix_ptr pMatrix);
		HRESULT SetFVF(DWORD FVF);
		void SetCursorPosition(int X, int Y, DWORD Flags);
		HRESULT SetVertexDeclaration(VertexDeclarationPtr pDecl);
		HRESULT CreateVertexDeclaration(VertexElement* pVertexElements, VertexDeclarationPtr* ppDecl);
		HRESULT BeginScene();
		HRESULT EndScene();
		HRESULT Clear(DWORD Count, const void* pRects, DWORD Flags, DWORD Color, float Z, DWORD Stencil);

		HRESULT SetViewport(const D3DVIEWPORT9* pViewport);
		HRESULT GetViewport(D3DVIEWPORT9* pViewport);

		HRESULT SetScissorRect(RECT* pRect);
		HRESULT GetScissorRect(RECT* pRect);

		/** this is only used by Opengl render target. */
		void BeginRenderTarget(int nWidth, int nHeight);
		void EndRenderTarget();
		bool IsUsingRenderTarget();
	public:
		// following function is unique to opengl renderer
		static uint32 GetStencilBits();

		
	protected:
		void GetScreenSize(int &nScreenWidth, int &nScreenHeight);
		static void ApplyBlendingModeChange();
	};
}
