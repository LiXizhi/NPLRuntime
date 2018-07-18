#include "Framework/Interface/Render/ITexture.h"

namespace ParaEngine
{
	class TextureOpenGL : public IParaEngine::ITexture
	{
	public:
		TextureOpenGL();
		virtual ~TextureOpenGL() override;


		virtual void* Lock(unsigned int level, unsigned int& pitch, const ParaEngine::Rect* rect) override;


		virtual void Unlock(unsigned int level) override;


		virtual bool CopyTo(ITexture* target) override;


		virtual bool StretchRect(ITexture* target, const ParaEngine::Rect* srcRect, const ParaEngine::Rect* targetRect, const ParaEngine::ETextureFilterType filter) override;


		virtual uint32_t GetWidth() const override;


		virtual uint32_t GetHeight() const override;

	protected:
		virtual void OnRelease() override;

	};
}