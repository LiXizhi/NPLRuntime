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
		static RenderDevice* GetInstance();

		static HRESULT DrawPrimitive(RenderDevicePtr pd3dDevice, int nStatisticsType, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);

		static HRESULT DrawPrimitiveUP(RenderDevicePtr pd3dDevice, int nStatisticsType,
			D3DPRIMITIVETYPE PrimitiveType,
			UINT PrimitiveCount,
			CONST void* pVertexStreamZeroData,
			UINT VertexStreamZeroStride);


		static HRESULT DrawIndexedPrimitive(RenderDevicePtr pd3dDevice, int nStatisticsType, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinIndex, UINT NumVertices, UINT indexStart, UINT PrimitiveCount);

		static HRESULT DrawIndexedPrimitiveUP(RenderDevicePtr pd3dDevice, int nStatisticsType,
			D3DPRIMITIVETYPE PrimitiveType,
			UINT MinVertexIndex,
			UINT NumVertices,
			UINT PrimitiveCount,
			CONST void * pIndexData,
			D3DFORMAT IndexDataFormat,
			CONST void* pVertexStreamZeroData,
			UINT VertexStreamZeroStride);
	
		/** check render error and print to log. only call this in debug mode, since it breaks parallelism between gpu and cpu. */
		static bool CheckRenderError(const char* filename = NULL, const char* func = NULL, int nLine = 0);

		/** read a block of pixels from the frame buffer
		this emulate: glReadPixels(). */
		static bool ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, DWORD nDataFormat = 0, DWORD nDataType = 0);
	public:

	};
}
