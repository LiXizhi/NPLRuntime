#include "ParaEngine.h"
#ifndef EMSCRIPTEN
#include <glad/glad.h>
#endif
#include "RenderContextSDL2.h"
#include "RenderWindowSDL2.h"

namespace ParaEngine {
	IRenderContext* IRenderContext::Create()
	{
		return new RenderContextSDL2();
	}

	IRenderDevice* RenderContextSDL2::CreateDevice(const RenderConfiguration& cfg)
	{
		return new RenderDeviceSDL2((SDL_Window*)((RenderWindowSDL2*)(cfg.renderWindow)->GetNativeHandle()));
	}


	bool RenderContextSDL2::ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg)
	{
		//throw std::logic_error("The method or operation is not implemented.");

		return true;
	}

	RenderDeviceSDL2::RenderDeviceSDL2(SDL_Window* window) :m_window(window)
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

