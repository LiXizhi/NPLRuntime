#pragma once
#include "RenderDeviceOpenGL.h"
@class NSOpenGLContext;
namespace ParaEngine
{
    class RenderDeviceAGL : public RenderDeviceOpenGL
    {
    public:
        RenderDeviceAGL(NSOpenGLContext* context);
        ~RenderDeviceAGL();
        
        virtual bool Present() override;

        virtual bool SetRenderTarget(uint32_t index, IParaEngine::ITexture* target) override;
		virtual bool SetDepthStencil(IParaEngine::ITexture* target) override;
		virtual bool StretchRect(IParaEngine::ITexture* source, IParaEngine::ITexture* dest, RECT* srcRect, RECT* destRect) override;
	private:
		void InitCpas();
		void InitFrameBuffer();
		void DrawQuad();
	private:
		GLuint m_FBO;
		std::shared_ptr<IParaEngine::IEffect> m_DownSampleEffect;
        NSOpenGLContext* m_GLContext;
    };
}
