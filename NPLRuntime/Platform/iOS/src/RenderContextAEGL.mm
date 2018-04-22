#include "ParaEngine.h"
#include "RenderContextAEGL.h"
#include "RenderWindowiOS.h"
#include "RenderDeviceOpenGL.h"
#include "RenderDeviceAEGL.h"

using namespace ParaEngine;

ParaEngine::IRenderContext* ParaEngine::IRenderContext::Create()
{
	return new RenderContextAEGL();
}

IRenderDevice* RenderContextAEGL::CreateDevice(const RenderConfiguration & cfg)
{
    return nullptr;
}

bool RenderContextAEGL::ResetDevice(IRenderDevice* device, const RenderConfiguration & cfg)
{



	return true;
}

ParaEngine::RenderContextAEGL::RenderContextAEGL()
{

}

ParaEngine::RenderContextAEGL::~RenderContextAEGL()
{
	ReleaseContext();
}

void ParaEngine::RenderContextAEGL::ReleaseContext()
{

}
