//-----------------------------------------------------------------------------
// Class: RenderDevice
// Authors:	LiXizhi
// Emails:	lixizhi@yeah.net
// Date: 2014.9.11
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "RenderDevice.h"
#ifdef USE_DIRECTX_RENDERER

#endif
using namespace ParaEngine;

int ParaEngine::RenderDeviceBase::g_perfCounters[RenderDeviceBase::DRAW_PERF_END];

int RenderDeviceBase::GetPerfCount(StatisticsType nStatisticsType)
{
	return g_perfCounters[nStatisticsType];
}

void RenderDeviceBase::ClearAllPerfCount()
{
	memset(g_perfCounters, 0, sizeof(g_perfCounters));
}

void ParaEngine::RenderDeviceBase::IncrementDrawBatchAndVertices(int nDrawCount, int nVertices, int nPrimitiveType)
{
	g_perfCounters[DRAW_PERF_DRAW_CALL_COUNT] += nDrawCount;
	g_perfCounters[DRAW_PERF_TOTAL_TRIANGLES] += nVertices;
	if (nPrimitiveType < DRAW_PERF_END) 
		g_perfCounters[nPrimitiveType] += nVertices;
}

int ParaEngine::RenderDeviceBase::GetMaxSimultaneousTextures()
{
	return CGlobals::GetRenderDevice()->GetCaps().MaxSimultaneousTextures;
}



