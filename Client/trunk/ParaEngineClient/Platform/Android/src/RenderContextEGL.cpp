#include "RenderContextEGL.h"

using namespace ParaEngine;

ParaEngine::IRenderContext* ParaEngine::IRenderContext::Create()
{
	return new RenderContextEGL();
}

IRenderDevice* RenderContextEGL::CreateDevice(const RenderConfiguration & cfg)
{



	return nullptr;
}

bool RenderContextEGL::ResetDevice(IRenderDevice* device, const RenderConfiguration & cfg)
{
	return false;
}
