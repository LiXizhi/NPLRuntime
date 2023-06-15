//-----------------------------------------------------------------------------
// Class: PaintEngine
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date: 2015.2.23
// Desc: I have referred to QT framework's qpaintengine.h, but greatly simplified. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "Painter.h"
#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/thread/tss.hpp>
#endif
#include "PaintEngineRaster.h"

using namespace ParaEngine;

ParaEngine::CPaintEngineRaster::CPaintEngineRaster()
{

}

ParaEngine::CPaintEngineRaster::~CPaintEngineRaster()
{

}

CPaintEngineRaster* ParaEngine::CPaintEngineRaster::GetInstance()
{
	thread_local static CPaintEngineRaster  s_singleton;
	return &s_singleton;
}

bool ParaEngine::CPaintEngineRaster::begin(CPaintDevice *pdev)
{
	return true;
}

bool ParaEngine::CPaintEngineRaster::end()
{
	return true;
}

void ParaEngine::CPaintEngineRaster::updateState(const CPaintEngineState &state)
{

}

void ParaEngine::CPaintEngineRaster::drawPixmap(const QRectF &r, const TextureEntity &pm, const QRectF &sr)
{

}
