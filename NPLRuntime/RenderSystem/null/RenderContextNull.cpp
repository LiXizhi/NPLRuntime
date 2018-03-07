#include "RenderContextNull.h"
#include "RenderDeviceNull.h"
using namespace ParaEngine;


IRenderContext* ParaEngine::IRenderContext::Create()
{
	IRenderContext* context = new RenderContextNull();
	return context;
}

ParaEngine::IRenderDevice* ParaEngine::RenderContextNull::CreateDevice(const RenderConfiguration& cfg)
{
	IRenderDevice* device = new RenderDeviceNull();
	return device;
}

bool ParaEngine::RenderContextNull::ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg)
{
	return true;
}

