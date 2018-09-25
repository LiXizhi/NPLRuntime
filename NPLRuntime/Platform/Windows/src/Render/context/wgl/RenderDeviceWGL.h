#pragma once
#include "RenderDeviceOpenGL.h"
namespace ParaEngine
{
	class RenderDeviceOpenWGL : public RenderDeviceOpenGL
	{
	public:
		RenderDeviceOpenWGL(HDC context);
		~RenderDeviceOpenWGL();

		virtual bool Present() override;
		virtual bool SetRenderTarget(uint32_t index, IParaEngine::ITexture* target) override;
		virtual bool SetDepthStencil(IParaEngine::ITexture* target) override;
		virtual bool StretchRect(IParaEngine::ITexture* source, IParaEngine::ITexture* dest, RECT* srcRect, RECT* destRect) override;

	private:
		void InitCpas();
		void InitFrameBuffer();
		bool IsSupportExt(const char* extName);
		void DrawQuad();

	private:
		HDC m_WGLContext;

		GLuint m_FBO;
		std::shared_ptr<IParaEngine::IEffect> m_DownSampleEffect;
	};
}
