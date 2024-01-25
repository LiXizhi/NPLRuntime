#pragma once

namespace ParaEngine
{
	// Primitives supported by draw-primitive API
	class EPrimitiveType {
	public:
		static const D3DPRIMITIVETYPE POINTLIST = D3DPT_POINTLIST;
		static const D3DPRIMITIVETYPE LINELIST = D3DPT_LINELIST;
		static const D3DPRIMITIVETYPE LINESTRIP = D3DPT_LINESTRIP;
		static const D3DPRIMITIVETYPE TRIANGLELIST = D3DPT_TRIANGLELIST;
		static const D3DPRIMITIVETYPE TRIANGLESTRIP = D3DPT_TRIANGLESTRIP;
		static const D3DPRIMITIVETYPE TRIANGLEFAN = D3DPT_TRIANGLEFAN;
	};

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
}

#ifdef USE_DIRECTX_RENDERER
#include "RenderDeviceDirectX.h"
#elif defined(USE_OPENGL_RENDERER)
#include "RenderDeviceOpenGL.h"
#else
#include "RenderDeviceOpenGL.h"
#endif