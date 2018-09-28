#pragma once
#include "RenderDeviceOpenGL.h"
@class EAGLContext;
@class CAEAGLLayer;
namespace ParaEngine
{
	class RenderDeviceAEGL : public RenderDeviceOpenGL
	{
	public:
		RenderDeviceAEGL(EAGLContext* context,CAEAGLLayer* layer);
		~RenderDeviceAEGL();
		void Reset();
		virtual bool Present() override;
		virtual bool SetRenderTarget(uint32_t index, IParaEngine::ITexture* target) override;
		virtual bool SetDepthStencil(IParaEngine::ITexture* target) override;
		virtual bool StretchRect(IParaEngine::ITexture* source, IParaEngine::ITexture* dest, RECT* srcRect, RECT* destRect) override;
    private:
        EAGLContext* m_GLContext;
        CAEAGLLayer* m_GLLayer;
        GLuint              m_FrameBuffer;
        GLuint              m_ColorBuffer;
        GLuint              m_DepthBuffer;

	private:
		void InitCpas();
		void InitFrameBuffer();
		void DrawQuad();
	private:
		GLuint m_FBO;
		std::shared_ptr<IParaEngine::IEffect> m_DownSampleEffect;
        
	};
}
