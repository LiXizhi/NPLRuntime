#pragma once
#include "RenderDeviceOpenGL.h"
@class EAGLContext;
@class CAEAGLLayer;
namespace ParaEngine
{
	class RenderDeviceAEGL : public RenderDeviceOpenGL
	{
	public:
		RenderDeviceAEGL();
		~RenderDeviceAEGL();
		virtual bool Reset(const RenderConfiguration& cfg) override;
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
		friend class IRenderDevice;
		void InitCpas();
		bool InitFrameBuffer();
		void DrawQuad();
	private:
		GLuint m_FBO;
		std::shared_ptr<IParaEngine::IEffect> m_DownSampleEffect;
	protected:
		virtual bool Initialize() override;
        
	};
}
