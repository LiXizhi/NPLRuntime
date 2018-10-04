#pragma once
#include "RenderDeviceOpenGL.h"
#include <EGL/egl.h>
namespace ParaEngine
{
	class RenderDeviceEGL : public RenderDeviceOpenGL
	{
	public:
		RenderDeviceEGL(EGLDisplay display, EGLSurface surface);
		~RenderDeviceEGL();

		void Reset(EGLDisplay display, EGLSurface surface);
		virtual bool Present() override;
		virtual bool SetRenderTarget(uint32_t index, IParaEngine::ITexture* target) override;
		virtual bool SetDepthStencil(IParaEngine::ITexture* target) override;
		virtual bool StretchRect(IParaEngine::ITexture* source, IParaEngine::ITexture* dest, RECT* srcRect, RECT* destRect) override;
	private:
		void InitCpas();
		void InitFrameBuffer();
		void DrawQuad();
	private:
		EGLDisplay m_Display;
		EGLSurface m_Surface;
		GLuint m_FBO;
		std::shared_ptr<IParaEngine::IEffect> m_DownSampleEffect;
	};
}