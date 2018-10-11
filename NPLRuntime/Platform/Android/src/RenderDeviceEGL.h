#pragma once
#include "RenderDeviceOpenGL.h"
#include <EGL/egl.h>
namespace ParaEngine
{
	class RenderDeviceEGL : public RenderDeviceOpenGL
	{
	public:
		RenderDeviceEGL();
		~RenderDeviceEGL();

		virtual bool Present() override;
		virtual bool SetRenderTarget(uint32_t index, IParaEngine::ITexture* target) override;
		virtual bool SetDepthStencil(IParaEngine::ITexture* target) override;
		virtual bool StretchRect(IParaEngine::ITexture* source, IParaEngine::ITexture* dest, RECT* srcRect, RECT* destRect) override;
		virtual bool Reset(const RenderConfiguration& cfg) override;
	private:
		friend class IRenderDevice;
		void InitCpas();
		bool InitFrameBuffer();
		void DrawQuad();
	protected:
		virtual bool Initialize() override;
	private:
		EGLDisplay m_Display;
		EGLSurface m_Surface;
		EGLContext m_Context;
		GLuint m_FBO;
		std::shared_ptr<IParaEngine::IEffect> m_DownSampleEffect;
	};
}