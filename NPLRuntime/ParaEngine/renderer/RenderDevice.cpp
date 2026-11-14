//-----------------------------------------------------------------------------
// Class: RenderDevice
// Authors:	LiXizhi
// Emails:	lixizhi@yeah.net
// Date: 2014.9.11
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "RenderDevice.h"
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
	auto* pDev = CGlobals::GetRenderDevice();
	return pDev ? pDev->GetMaxSimultaneousTextures() : 8;
}

bool ParaEngine::IRenderDevice::CheckRenderError(const char* filename, const char* func, int nLine)
{
	auto* pDev = CGlobals::GetRenderDevice();
	return pDev ? pDev->CheckDeviceError(filename, func, nLine) : true;
}

RenderDevice* ParaEngine::RenderDevice::GetInstance()
{
	static RenderDevice g_instance;
	return &g_instance;
}

HRESULT ParaEngine::RenderDevice::DrawPrimitive(IRenderDevice* pRenderDevice, int nStatisticsType, EPrimitiveType PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount)
{
	if (!pRenderDevice) return E_FAIL;
	IncrementDrawBatchAndVertices(1, PrimitiveCount, nStatisticsType);
	return pRenderDevice->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount) ? S_OK : E_FAIL;
}

HRESULT ParaEngine::RenderDevice::DrawPrimitiveUP(IRenderDevice* pRenderDevice, int nStatisticsType, EPrimitiveType PrimitiveType, uint32_t PrimitiveCount, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
{
	if (!pRenderDevice) return E_FAIL;
	IncrementDrawBatchAndVertices(1, PrimitiveCount, nStatisticsType);
	return pRenderDevice->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride) ? S_OK : E_FAIL;
}

HRESULT ParaEngine::RenderDevice::DrawIndexedPrimitive(IRenderDevice* pRenderDevice, int nStatisticsType, EPrimitiveType Type, int32_t BaseVertexIndex, uint32_t MinIndex, uint32_t NumVertices, uint32_t indexStart, uint32_t PrimitiveCount)
{
	if (!pRenderDevice) return E_FAIL;
	IncrementDrawBatchAndVertices(1, PrimitiveCount, nStatisticsType);
	return pRenderDevice->DrawIndexedPrimitive(Type, BaseVertexIndex, MinIndex, NumVertices, indexStart, PrimitiveCount) ? S_OK : E_FAIL;
}

HRESULT ParaEngine::RenderDevice::DrawIndexedPrimitiveUP(IRenderDevice* pRenderDevice, int nStatisticsType, EPrimitiveType PrimitiveType, uint32_t MinVertexIndex, uint32_t NumVertices, uint32_t PrimitiveCount, const void* pIndexData, PixelFormat IndexDataFormat, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
{
	if (!pRenderDevice) return E_FAIL;
	IncrementDrawBatchAndVertices(1, PrimitiveCount, nStatisticsType);
	return pRenderDevice->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride) ? S_OK : E_FAIL;
}

HRESULT ParaEngine::RenderDevice::DrawIndexedInstanced(IRenderDevice* pRenderDevice, int nStatisticsType, EPrimitiveType PrimitiveType, int32_t BaseVertexIndex, uint32_t indexStart, uint32_t PrimitiveCount, uint32_t InstanceCount)
{
	if (!pRenderDevice) return E_FAIL;
	IncrementDrawBatchAndVertices(1, PrimitiveCount * InstanceCount, nStatisticsType);
	return pRenderDevice->DrawIndexedInstanced(PrimitiveType, BaseVertexIndex, indexStart, PrimitiveCount, InstanceCount) ? S_OK : E_FAIL;
}




