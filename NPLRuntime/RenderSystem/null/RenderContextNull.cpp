#include "RenderContextNull.h"
#include "RenderDeviceNull.h"

using namespace ParaEngine;

namespace ParaEngine
{
	IRenderContext* CreateRenderContextNull()
	{
		return new RenderContextNull();
	}

#ifndef PARA_RHI_MULTI_BACKEND
	IRenderContext* IRenderContext::Create()
	{
		return CreateRenderContextNull();
	}

	IRenderContext* IRenderContext::Create(ERHIBackend)
	{
		return CreateRenderContextNull();
	}
#endif
}

ParaEngine::IRenderDevice* ParaEngine::RenderContextNull::CreateDevice(const RenderConfiguration& cfg)
{
	return new RenderDeviceNull();
}

bool ParaEngine::RenderContextNull::ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg)
{
	return true;
}
