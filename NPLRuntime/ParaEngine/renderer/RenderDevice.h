#pragma once
#include "Framework/Interface/Render/IRenderDevice.h"

namespace ParaEngine
{
	class RenderDeviceBase
	{
	public:
		enum StatisticsType
		{
			DRAW_PERF_TRIANGLES_UNKNOWN = 0,
			DRAW_PERF_TRIANGLES_UI = 1,
			DRAW_PERF_TRIANGLES_TERRAIN = 2,
			DRAW_PERF_TRIANGLES_CHARACTER = 3,
			DRAW_PERF_TRIANGLES_MESH = 4,
			// total number of draw calls
			DRAW_PERF_DRAW_CALL_COUNT = 5,
			// total triangles
			DRAW_PERF_TOTAL_TRIANGLES = 6,
			DRAW_PERF_END,
		};
	public:
		/** get the performance counter for a given type */
		static int GetPerfCount(StatisticsType nStatisticsType);
		/** reset all the performance counters to 0. this is usually called at the beginning of each frame */
		static void ClearAllPerfCount();

		/** increment both draw count and vertices */
		static void IncrementDrawBatchAndVertices(int nDrawCount, int nVertices, int nPrimitiveType = DRAW_PERF_TRIANGLES_UNKNOWN);

		/* max number of simultaneous textures. in most cards, it is 8, some is 4. */
		static int GetMaxSimultaneousTextures();
	public:

		static int g_perfCounters[DRAW_PERF_END];
	};

	/**
	 * Unified draw-dispatch & perf-counting helper shared by all ParaRHI backends.
	 */
	class RenderDevice : public RenderDeviceBase
	{
	public:
		static RenderDevice* GetInstance();

		static HRESULT DrawPrimitive(IRenderDevice* pRenderDevice, int nStatisticsType, EPrimitiveType PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount);
		static HRESULT DrawPrimitiveUP(IRenderDevice* pRenderDevice, int nStatisticsType, EPrimitiveType PrimitiveType, uint32_t PrimitiveCount, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride);
		static HRESULT DrawIndexedPrimitive(IRenderDevice* pRenderDevice, int nStatisticsType, EPrimitiveType Type, int32_t BaseVertexIndex, uint32_t MinIndex, uint32_t NumVertices, uint32_t indexStart, uint32_t PrimitiveCount);
		static HRESULT DrawIndexedPrimitiveUP(IRenderDevice* pRenderDevice, int nStatisticsType, EPrimitiveType PrimitiveType, uint32_t MinVertexIndex, uint32_t NumVertices, uint32_t PrimitiveCount, const void* pIndexData, PixelFormat IndexDataFormat, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride);
		static HRESULT DrawIndexedInstanced(IRenderDevice* pRenderDevice, int nStatisticsType, EPrimitiveType PrimitiveType, int32_t BaseVertexIndex, uint32_t indexStart, uint32_t PrimitiveCount, uint32_t InstanceCount);
	};
}
