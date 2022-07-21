#pragma once
#include "Framework/Interface/Render/IRenderContext.h"
#include "RenderDeviceOpenGL.h"
#include "SDL2/SDL.h"

namespace ParaEngine
{
	class RenderContextSDL2 : public IRenderContext
	{
	public:
		RenderContextSDL2() = default;
		virtual ~RenderContextSDL2() override = default;
		virtual IRenderDevice* CreateDevice(const RenderConfiguration& cfg) override;
		virtual bool ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg) override;
	private:
	};

	class RenderDeviceSDL2 : public RenderDeviceOpenGL
	{
	public:
		RenderDeviceSDL2(SDL_Window* window);
		~RenderDeviceSDL2();

		virtual bool Present() override;
	private:
		SDL_Window* m_window;

	};
}