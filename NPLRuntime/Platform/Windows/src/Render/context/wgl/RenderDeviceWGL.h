#pragma once
#include "RenderDeviceOpenGL.h"
namespace ParaEngine
{
	class RenderDeviceOpenWGL : public RenderDeviceOpenGL
	{
	public:
		RenderDeviceOpenWGL();
		~RenderDeviceOpenWGL();

		virtual bool Present() override;
		virtual bool SetRenderTarget(uint32_t index, IParaEngine::ITexture* target) override;
		virtual bool SetDepthStencil(IParaEngine::ITexture* target) override;
		virtual bool StretchRect(IParaEngine::ITexture* source, IParaEngine::ITexture* dest, RECT* srcRect, RECT* destRect) override;


		virtual bool Reset(const RenderConfiguration& cfg) override;

	private:
		void InitCpas();
		bool InitFrameBuffer();	
		void DrawQuad();
		
		HDC m_DeviceContext;
		HGLRC m_GLRenderingContext;
		GLuint m_FBO;
		std::shared_ptr<IParaEngine::IEffect> m_DownSampleEffect;

		friend class IRenderDevice;

	protected:
		virtual bool Initialize() override;

	};
}
