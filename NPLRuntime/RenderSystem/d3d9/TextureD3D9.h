#pragma once
#include "Framework/RenderSystem/RenderTypes.h"
#include "Framework/Interface/Render/ITexture.h"
#include <d3d9.h>
namespace ParaEngine
{
	class RenderDeviceD3D9;
	class TextureD3D9 : public IParaEngine::ITexture
	{
	public:
		static ITexture* Create(RenderDeviceD3D9* device,uint32_t width, uint32_t height, EPixelFormat format, ETextureUsage usage);
		static ITexture* Create(RenderDeviceD3D9* device,LPDIRECT3DTEXTURE9 texture);
		static ITexture* Create(RenderDeviceD3D9* device,LPDIRECT3DSURFACE9 surface, ETextureUsage usage);
		TextureD3D9();
		virtual ~TextureD3D9() override = default;
		virtual void OnRelease();
		virtual void* Lock(unsigned int level, unsigned int& pitch, const ParaEngine::Rect* rect) override;
		virtual void Unlock(unsigned int level) override;

		LPDIRECT3DSURFACE9 GetSurface() const;
		LPDIRECT3DTEXTURE9 GetTexture() const;


		virtual bool CopyTo(IParaEngine::ITexture* target) override;


		virtual uint32_t GetWidth() const override;


		virtual uint32_t GetHeight() const override;


		virtual bool StretchRect(IParaEngine::ITexture* target, const ParaEngine::Rect* srcRect, const ParaEngine::Rect* targetRect, const ETextureFilterType filter) override;

	protected:
		LPDIRECT3DTEXTURE9 m_Texture;
		LPDIRECT3DSURFACE9 m_Surface;
		RenderDeviceD3D9* m_Device;
		EPixelFormat m_Format;
		uint32_t m_Width;
		uint32_t m_Height;
		ETextureUsage m_Usage;
		

	private:
		bool InnerCreate(uint32_t width, uint32_t height, EPixelFormat format, ETextureUsage usage);
	};

	inline LPDIRECT3DTEXTURE9 GetD3DTex(IParaEngine::ITexture* tex)
	{
		if (tex == nullptr) return nullptr;

		return static_cast<TextureD3D9*>(tex)->GetTexture();
	}

	inline LPDIRECT3DSURFACE9 GetD3DSurface(IParaEngine::ITexture* tex)
	{
		if (tex == nullptr) return nullptr;

		return static_cast<TextureD3D9*>(tex)->GetSurface();
	}
};