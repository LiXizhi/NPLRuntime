//-----------------------------------------------------------------------------
// Class: RenderDevice
// Authors:	LiXizhi
// Emails:	lixizhi@yeah.net
// Date: 2014.9.11
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "RenderDevice.h"
#include <atomic>
#include <climits>
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

namespace
{
	std::atomic<int> lastFrameCounts[RenderDeviceBase::DRAW_PERF_END]{};

	int AddPerfCount(int current, uint64_t count)
	{
		return static_cast<int>((std::min)(uint64_t(INT_MAX), current + count));
	}
}

void RenderDeviceBase::RecordDraw(EPrimitiveType topology, uint32_t primitiveCount, uint32_t instanceCount)
{
	auto& draws = g_perfCounters[DRAW_PERF_DRAW_CALL_COUNT];
	draws = AddPerfCount(draws, 1);
	if (topology == EPrimitiveType::TRIANGLELIST || topology == EPrimitiveType::TRIANGLESTRIP || topology == EPrimitiveType::TRIANGLEFAN)
	{
		auto& triangles = g_perfCounters[DRAW_PERF_TOTAL_TRIANGLES];
		triangles = AddPerfCount(triangles, uint64_t(primitiveCount) * instanceCount);
	}
}

void RenderDeviceBase::PublishFramePerfCounts()
{
	for (int i = 0; i < DRAW_PERF_END; ++i)
		lastFrameCounts[i].store(g_perfCounters[i], std::memory_order_relaxed);
}

int RenderDeviceBase::GetLastFramePerfCount(StatisticsType type)
{
	return lastFrameCounts[type].load(std::memory_order_relaxed);
}

// The scene's character budget reads category counts while rendering is still in progress.
static void CountCategoryTriangles(int category, uint64_t triangles)
{
	if (category >= 0 && category < RenderDeviceBase::DRAW_PERF_DRAW_CALL_COUNT)
	{
		auto& count = RenderDeviceBase::g_perfCounters[category];
		count = AddPerfCount(count, triangles);
	}
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
	CountCategoryTriangles(nStatisticsType, PrimitiveCount);
	return pRenderDevice->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount) ? S_OK : E_FAIL;
}

HRESULT ParaEngine::RenderDevice::DrawPrimitiveUP(IRenderDevice* pRenderDevice, int nStatisticsType, EPrimitiveType PrimitiveType, uint32_t PrimitiveCount, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
{
	if (!pRenderDevice) return E_FAIL;
	CountCategoryTriangles(nStatisticsType, PrimitiveCount);
	return pRenderDevice->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride) ? S_OK : E_FAIL;
}

HRESULT ParaEngine::RenderDevice::DrawIndexedPrimitive(IRenderDevice* pRenderDevice, int nStatisticsType, EPrimitiveType Type, int32_t BaseVertexIndex, uint32_t MinIndex, uint32_t NumVertices, uint32_t indexStart, uint32_t PrimitiveCount)
{
	if (!pRenderDevice) return E_FAIL;
	CountCategoryTriangles(nStatisticsType, PrimitiveCount);
	return pRenderDevice->DrawIndexedPrimitive(Type, BaseVertexIndex, MinIndex, NumVertices, indexStart, PrimitiveCount) ? S_OK : E_FAIL;
}

HRESULT ParaEngine::RenderDevice::DrawIndexedPrimitiveUP(IRenderDevice* pRenderDevice, int nStatisticsType, EPrimitiveType PrimitiveType, uint32_t MinVertexIndex, uint32_t NumVertices, uint32_t PrimitiveCount, const void* pIndexData, PixelFormat IndexDataFormat, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
{
	if (!pRenderDevice) return E_FAIL;
	CountCategoryTriangles(nStatisticsType, PrimitiveCount);
	return pRenderDevice->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride) ? S_OK : E_FAIL;
}

HRESULT ParaEngine::RenderDevice::DrawIndexedInstanced(IRenderDevice* pRenderDevice, int nStatisticsType, EPrimitiveType PrimitiveType, int32_t BaseVertexIndex, uint32_t indexStart, uint32_t PrimitiveCount, uint32_t InstanceCount)
{
	if (!pRenderDevice) return E_FAIL;
	CountCategoryTriangles(nStatisticsType, uint64_t(PrimitiveCount) * InstanceCount);
	return pRenderDevice->DrawIndexedInstanced(PrimitiveType, BaseVertexIndex, indexStart, PrimitiveCount, InstanceCount) ? S_OK : E_FAIL;
}




