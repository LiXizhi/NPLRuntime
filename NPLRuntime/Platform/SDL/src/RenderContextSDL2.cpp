#include "ParaEngine.h"
#ifndef EMSCRIPTEN
#include <glad/glad.h>
#endif
#include "RenderContextSDL2.h"
#include "RenderWindowSDL2.h"
#include "RenderSystem/rhi_core/RHIBackendRegistry.h"
#ifdef PARA_RHI_HAS_WEBGPU
#include "RenderSystem/webgpu/RenderContextWebGPU.h"
#endif

namespace ParaEngine
{
	IRenderContext* CreateRenderContextSDL2()
	{
		return new RenderContextSDL2();
	}

	namespace
	{
		struct SDLBackendAutoRegistrar
		{
			SDLBackendAutoRegistrar()
			{
#ifdef PARA_RHI_HAS_WEBGPU
#ifdef EMSCRIPTEN
				RHIBackendRegistry::GetInstance().RegisterBackend(ERHIBackend::WebGPU, "WebGPU", &CreateRenderContextWebGPU, 100);
#else
				RHIBackendRegistry::GetInstance().RegisterBackend(ERHIBackend::WebGPU, "WebGPU", &CreateRenderContextWebGPU, 60);
#endif
#endif
				RHIBackendRegistry::GetInstance().RegisterBackend(ERHIBackend::OpenGL, "OpenGL", &CreateRenderContextSDL2, 50);
			}
		} s_sdlBackendRegistrar;
	}

#ifndef PARA_RHI_MULTI_BACKEND
	IRenderContext* IRenderContext::Create()
	{
		return CreateRenderContextSDL2();
	}
#endif

	IRenderDevice* RenderContextSDL2::CreateDevice(const RenderConfiguration& cfg)
	{
		if (!cfg.renderWindow)
			return nullptr;
		auto* renderWin = static_cast<RenderWindowSDL2*>(cfg.renderWindow);
		if (!renderWin->EnsureGLContext())
			return nullptr;
		return new RenderDeviceSDL2(reinterpret_cast<SDL_Window*>(renderWin->GetNativeHandle()));
	}

	bool RenderContextSDL2::ResetDevice(IRenderDevice* /*device*/, const RenderConfiguration& /*cfg*/)
	{
		return true;
	}

	RenderDeviceSDL2::RenderDeviceSDL2(SDL_Window* window)
		: m_window(window)
	{
	}

	RenderDeviceSDL2::~RenderDeviceSDL2()
	{
	}

	bool RenderDeviceSDL2::Present()
	{
		SDL_GL_SwapWindow(m_window);
		return true;
	}
}
